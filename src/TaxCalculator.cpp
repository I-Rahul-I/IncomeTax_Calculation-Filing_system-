#include "TaxCalculator.h"


#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>


using namespace std;


namespace {


string trimCalculatorText(const string& value) {
    const string whitespace = " \t\n\r\f\v";

    const size_t first =
        value.find_first_not_of(whitespace);

    if (first == string::npos) {
        return "";
    }

    const size_t last =
        value.find_last_not_of(whitespace);

    return value.substr(first, last - first + 1);
}


void checkCalculatorAmount(
    double amount,
    const string& description
) {
    if (!isfinite(amount)) {
        throw overflow_error(
            description + " exceeded numeric limits."
        );
    }
}


double roundToNearestRupee(double amount) {
    if (!isfinite(amount)) {
        throw overflow_error(
            "Cannot round a non-finite amount."
        );
    }

    return round(amount);
}


void addCalculatorBreakdown(
    TaxComputationResult& result,
    const string& description,
    double amount
) {
    TaxBreakdownItem item;

    item.description = description;
    item.amount = amount;

    result.breakdown.push_back(item);
}


void addCalculatorAdjustment(
    TaxComputationResult& result,
    const string& code,
    const string& description,
    double claimedAmount,
    double allowedAmount,
    const string& explanation
) {
    TaxAdjustment adjustment;

    adjustment.code = code;
    adjustment.description = description;
    adjustment.claimedAmount = claimedAmount;
    adjustment.allowedAmount = allowedAmount;
    adjustment.explanation = explanation;

    result.adjustments.push_back(adjustment);
}


bool hasIncomeHead(
    const vector<Income*>& incomeHeads,
    IncomeHead head
) {
    for (const Income* income : incomeHeads) {
        if (income != nullptr && income->getHead() == head) {
            return true;
        }
    }

    return false;
}


// Selects the configured slab-profile name.
//
// The New Regime slab rates are identical for resident
// Individuals of all ages, but the profile names are retained
// to match the configuration file.
string getSlabProfileName(
    const Taxpayer& taxpayer
) {
    const TaxpayerType type = taxpayer.getType();
    const ResidentialStatus status =
        taxpayer.getResidentialStatus();

    if (type == TaxpayerType::HUF) {
        return "HUF";
    }

    if (type != TaxpayerType::Individual) {
        throw invalid_argument(
            "The current calculator supports Individual and "
            "HUF taxpayers only."
        );
    }

    if (status == ResidentialStatus::NonResident) {
        return "INDIVIDUAL_NON_RESIDENT";
    }

    const IndividualTaxpayer* individual =
        dynamic_cast<const IndividualTaxpayer*>(
            &taxpayer
        );

    if (individual == nullptr) {
        throw invalid_argument(
            "An Individual taxpayer must be an "
            "IndividualTaxpayer object."
        );
    }

    switch (individual->getAgeCategory()) {
        case AgeCategory::Below60:
            return "INDIVIDUAL_RESIDENT_BELOW_60";

        case AgeCategory::SeniorCitizen:
            return "INDIVIDUAL_RESIDENT_60_TO_79";

        case AgeCategory::SuperSeniorCitizen:
            return "INDIVIDUAL_RESIDENT_80_PLUS";

        default:
            throw logic_error(
                "Unexpected Individual age category."
            );
    }
}


} // namespace


// ============================================================
// Validation
// ============================================================


void TaxCalculator::validateIncomeHeadPointers(
    const vector<Income*>& incomeHeads
) {
    for (const Income* income : incomeHeads) {
        if (income == nullptr) {
            throw invalid_argument(
                "Income-head pointer cannot be null."
            );
        }
    }
}


void TaxCalculator::validateInput(
    const TaxCalculationInput& input
) {
    if (!input.rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before calculating tax."
        );
    }

    if (input.taxpayer == nullptr) {
        throw invalid_argument(
            "Taxpayer object cannot be null."
        );
    }

    validateIncomeHeadPointers(input.incomeHeads);

    string supportKey;

    switch (input.taxpayer->getType()) {
        case TaxpayerType::Individual:
            supportKey = "SUPPORT_INDIVIDUAL";
            break;

        case TaxpayerType::HUF:
            supportKey = "SUPPORT_HUF";
            break;

        default:
            throw invalid_argument(
                "The current calculator supports Individual "
                "and HUF taxpayers only."
            );
    }

    if (!input.rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "Taxpayer category is disabled in this configuration."
        );
    }

    input.payments.validateAgainstRules(input.rules);

    // Multiple transactions under capital gains are valid and need to be
    // aggregated for holding-period-specific loss set-off. Other heads
    // remain one annual entry each to avoid accidental duplicate inputs.
    vector<bool> seenHead(
        static_cast<size_t>(IncomeHead::ExemptIncome) + 1,
        false
    );

    for (const Income* income : input.incomeHeads) {
        const int headIndex =
            static_cast<int>(income->getHead());

        if (income->getHead() == IncomeHead::CapitalGains) {
            continue;
        }

        if (seenHead[headIndex]) {
            throw invalid_argument(
                "Duplicate income-head object: " +
                income->getHeadName()
            );
        }

        seenHead[headIndex] = true;
    }
}


// ============================================================
// Income evaluation and aggregation
// ============================================================


vector<IncomeComputationResult>
TaxCalculator::evaluateAllIncomeHeads(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    vector<IncomeComputationResult> headResults;

    for (Income* income : input.incomeHeads) {
        headResults.push_back(
            income->evaluate(
                *input.taxpayer,
                input.rules
            )
        );
    }

    for (const IncomeComputationResult& headResult :
         headResults) {
        for (const string& note : headResult.notes) {
            result.notes.push_back(
                "Income head: " + note
            );
        }
    }

    return headResults;
}


void TaxCalculator::aggregateIncomeHeads(
    vector<IncomeComputationResult>& headResults,
    RegimeType regime,
    TaxComputationResult& result
) {
    // Capital-loss set-off: STCL can offset STCG then LTCG; LTCL can
    // offset LTCG only. This is performed across all transaction entries
    // before the tax base is assembled. Remaining losses are surfaced in
    // the report as carry-forward candidates (subject to timely filing and
    // statutory eligibility, which are collected in a later step).
    double shortTermLoss = 0.0;
    double longTermLoss = 0.0;

    struct CapitalGainBucket {
        size_t resultIndex;
        bool shortTerm;
        bool normalRate;
        size_t componentIndex;
        double amount;
    };
    vector<CapitalGainBucket> capitalGains;

    for (size_t i = 0; i < headResults.size(); ++i) {
        auto& headResult = headResults[i];
        if (headResult.head != IncomeHead::CapitalGains) continue;

        for (const auto& adjustment : headResult.adjustments) {
            if (adjustment.code == "CAPITAL_LOSS_PENDING_ST")
                shortTermLoss += adjustment.claimedAmount;
            else if (adjustment.code == "CAPITAL_LOSS_PENDING_LT")
                longTermLoss += adjustment.claimedAmount;
        }

        bool shortTerm = false;
        bool hasClassification = false;
        for (const auto& adjustment : headResult.adjustments) {
            if (adjustment.code == "CAPITAL_GAIN_ST") {
                shortTerm = true;
                hasClassification = true;
                break;
            }
            if (adjustment.code == "CAPITAL_GAIN_LT") {
                shortTerm = false;
                hasClassification = true;
                break;
            }
        }
        if (!hasClassification) continue;

        if (headResult.normalRateIncome > 0.0) {
            capitalGains.push_back({i, shortTerm, true, 0,
                                    headResult.normalRateIncome});
        }
        for (size_t j = 0; j < headResult.specialRateComponents.size(); ++j) {
            const auto& component = headResult.specialRateComponents[j];
            capitalGains.push_back({i, shortTerm, false, j,
                                    component.amount});
        }
    }

    auto reduceBucket = [&](CapitalGainBucket& bucket, double& loss) {
        if (loss <= 0.0 || bucket.amount <= 0.0) return;
        const double applied = min(loss, bucket.amount);
        bucket.amount -= applied;
        loss -= applied;
        auto& headResult = headResults[bucket.resultIndex];
        if (bucket.normalRate) {
            headResult.normalRateIncome = bucket.amount;
        } else {
            headResult.specialRateComponents[bucket.componentIndex].amount =
                bucket.amount;
        }
    };

    // ST loss gets first set-off against ST gains, then LT gains.
    for (auto& bucket : capitalGains)
        if (bucket.shortTerm) reduceBucket(bucket, shortTermLoss);
    for (auto& bucket : capitalGains)
        if (!bucket.shortTerm) reduceBucket(bucket, shortTermLoss);

    // LT loss may only reduce remaining LTCG.
    for (auto& bucket : capitalGains)
        if (!bucket.shortTerm) reduceBucket(bucket, longTermLoss);

    if (shortTermLoss > 0.0 || longTermLoss > 0.0) {
        addCalculatorAdjustment(
            result,
            "CAPITAL_LOSS_CARRY_FORWARD_CANDIDATE",
            "Unabsorbed capital loss candidate",
            shortTermLoss + longTermLoss,
            0.0,
            "Unabsorbed STCL/LTCL remains subject to return-filing "
            "deadline, prior-year validity, and the applicable carry-forward period."
        );
        result.notes.push_back(
            "Unabsorbed capital losses are shown as carry-forward candidates; "
            "this session does not store them into a future assessment year."
        );
    }

    // Current-year inter-head loss set-off for business and house property.
    // Salary cannot absorb a business loss. House-property loss is capped
    // at ₹2 lakh for old-regime inter-head set-off and is not set off against
    // other heads under the new regime. Remaining amounts are carry-forward
    // candidates (filing/deadline requirements still apply).
    double businessLoss = 0.0;
    double housePropertyLoss = 0.0;
    for (auto& headResult : headResults) {
        if (headResult.head == IncomeHead::BusinessProfession &&
            headResult.normalRateIncome < 0.0) {
            businessLoss += -headResult.normalRateIncome;
            headResult.normalRateIncome = 0.0;
        } else if (headResult.head == IncomeHead::HouseProperty &&
                   headResult.normalRateIncome < 0.0) {
            housePropertyLoss += -headResult.normalRateIncome;
            headResult.normalRateIncome = 0.0;
        }
    }

    auto offsetAgainstNormalHeads = [&](double& loss, bool excludeSalary) {
        const IncomeHead order[] = {
            IncomeHead::OtherSources,
            IncomeHead::HouseProperty,
            IncomeHead::CapitalGains,
            IncomeHead::BusinessProfession,
            IncomeHead::Salary
        };
        for (IncomeHead allowedHead : order) {
            if (loss <= 0.0) break;
            if (excludeSalary && allowedHead == IncomeHead::Salary) continue;
            for (auto& headResult : headResults) {
                if (headResult.head != allowedHead ||
                    headResult.normalRateIncome <= 0.0) continue;
                const double offset = min(loss, headResult.normalRateIncome);
                headResult.normalRateIncome -= offset;
                loss -= offset;
                if (loss <= 0.0) break;
            }
        }
    };

    offsetAgainstNormalHeads(businessLoss, true);

    const double housePropertyInterHeadLimit =
        regime == RegimeType::Old ? 200000.0 : 0.0;
    const double housePropertySetoff =
        min(housePropertyLoss, housePropertyInterHeadLimit);
    housePropertyLoss -= housePropertySetoff;
    double allowedHouseSetoff = housePropertySetoff;
    offsetAgainstNormalHeads(allowedHouseSetoff, false);
    // The amount above is consumed by recipients; any residual due to no
    // positive recipient remains a candidate rather than disappearing.
    housePropertyLoss += allowedHouseSetoff;

    if (businessLoss > 0.0) {
        addCalculatorAdjustment(result, "BUSINESS_LOSS_CARRY_FORWARD_CANDIDATE",
            "Unabsorbed business loss candidate", businessLoss, 0.0,
            "Carry-forward depends on timely filing and the applicable business-loss rules.");
    }
    if (housePropertyLoss > 0.0) {
        addCalculatorAdjustment(result, "HOUSE_PROPERTY_LOSS_CARRY_FORWARD_CANDIDATE",
            "Unabsorbed house-property loss candidate", housePropertyLoss, 0.0,
            "Unabsorbed house-property loss remains subject to the statutory carry-forward rules.");
    }

    double normalIncome = 0.0;

    for (const IncomeComputationResult& headResult : headResults) {
        normalIncome += headResult.normalRateIncome;
        checkCalculatorAmount(normalIncome,
                             "Aggregated normal-rate income");
    }

    double specialIncome = 0.0;
    for (const IncomeComputationResult& headResult : headResults) {
        specialIncome += headResult.getSpecialRateIncomeTotal();
        checkCalculatorAmount(specialIncome, "Aggregated special-rate income");
    }

    result.grossTotalIncome = normalIncome + specialIncome;

    checkCalculatorAmount(
        result.grossTotalIncome,
        "Gross total income"
    );

    addCalculatorBreakdown(
        result,
        "Gross Total Income",
        result.grossTotalIncome
    );

    result.notes.push_back(
        "Exempt income is reported separately and is not "
        "included in Gross Total Income."
    );
}


// ============================================================
// Deductions
// ============================================================


void TaxCalculator::calculateDeductions(
    const TaxCalculationInput& input,
    double grossTotalIncome,
    TaxComputationResult& result
) {
    const vector<DeductionResult> claims =
        input.deductions.evaluateClaims(
            *input.taxpayer,
            input.rules
        );

    double totalAllowed = 0.0;

    double section80C = 0.0;
    double section80CCC = 0.0;
    double section80CCD1 = 0.0;

    for (const DeductionResult& claim : claims) {
        const double allowed = claim.allowedAmount;

        if (claim.ruleCode == "80C") {
            section80C = allowed;
        } else if (claim.ruleCode == "80CCC") {
            section80CCC = allowed;
        } else if (claim.ruleCode == "80CCD_1") {
            section80CCD1 = allowed;
        }

        totalAllowed += allowed;

        checkCalculatorAmount(
            totalAllowed,
            "Total Chapter VI-A deductions"
        );

        addCalculatorAdjustment(
            result,
            claim.ruleCode,
            claim.description,
            claim.claimedAmount,
            allowed,
            claim.explanation
        );
    }

    // --------------------------------------------------------
    // Combined 80C / 80CCC / 80CCD(1) ceiling
    // --------------------------------------------------------

    const double combinedCeiling =
        input.rules.getNumber(
            "SECTION_80CCE_COMBINED_LIMIT"
        );

    const double combinedClaimed =
        section80C + section80CCC + section80CCD1;

    if (combinedClaimed > combinedCeiling) {
        const double reduction =
            combinedClaimed - combinedCeiling;

        totalAllowed -= reduction;

        checkCalculatorAmount(
            totalAllowed,
            "Total deductions after combined ceiling"
        );

        addCalculatorAdjustment(
            result,
            "80C_80CCC_80CCD1_COMBINED",
            "Combined 80C, 80CCC and 80CCD(1) ceiling",
            combinedClaimed,
            combinedCeiling,
            "The combined ceiling has been applied before "
            "the GTI cap."
        );
    }

    // --------------------------------------------------------
    // GTI cap
    // --------------------------------------------------------

    double specialRateIncome = 0.0;
    for (const IncomeComputationResult& headResult : result.incomeHeadResults) {
        specialRateIncome += headResult.getSpecialRateIncomeTotal();
    }
    const double normalRateIncome =
        max(0.0, grossTotalIncome - specialRateIncome);

    const double cappedDeductions =
        min(totalAllowed, normalRateIncome);

    if (cappedDeductions < totalAllowed) {
        addCalculatorAdjustment(
            result,
            "CHAPTER_VIA_GTI_CAP",
            "Chapter VI-A deductions restricted to GTI",
            totalAllowed,
            cappedDeductions,
            "Deductions cannot exceed Gross Total Income."
        );
    }

    result.totalDeductions = cappedDeductions;

    addCalculatorBreakdown(
        result,
        "Less: Chapter VI-A deductions",
        -result.totalDeductions
    );

    result.totalIncomeBeforeRounding =
        result.grossTotalIncome - result.totalDeductions;

    checkCalculatorAmount(
        result.totalIncomeBeforeRounding,
        "Total income before rounding"
    );

    result.totalIncome =
        roundToNearestRupee(
            result.totalIncomeBeforeRounding
        );

    addCalculatorBreakdown(
        result,
        "Total Income after rounding",
        result.totalIncome
    );
}


// ============================================================
// Slab tax
// ============================================================


void TaxCalculator::calculateSlabTax(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    const string profileName =
        getSlabProfileName(*input.taxpayer);

    const TaxSlabProfile& profile =
        input.rules.getSlabProfile(profileName);

    if (profile.slabs.empty()) {
        throw invalid_argument(
            "No slab configuration is available for profile: " +
            profileName
        );
    }

    double specialRateIncome = 0.0;
    for (const IncomeComputationResult& headResult : result.incomeHeadResults) {
        specialRateIncome += headResult.getSpecialRateIncomeTotal();
    }
    const double slabTaxableIncome =
        max(0.0, result.totalIncome - specialRateIncome);

    double slabTax = 0.0;

    for (const TaxSlab& slab : profile.slabs) {
        if (slabTaxableIncome <= slab.lowerLimit) {
            continue;
        }

        double taxableInSlab;

        if (!slab.hasUpperLimit) {
            taxableInSlab =
                slabTaxableIncome - slab.lowerLimit;
        } else {
            taxableInSlab =
                min(
                    slabTaxableIncome,
                    slab.upperLimit
                ) - slab.lowerLimit;
        }

        if (taxableInSlab <= 0.0) {
            continue;
        }

        const double taxInSlab =
            taxableInSlab * slab.ratePercent / 100.0;

        slabTax += taxInSlab;

        checkCalculatorAmount(
            slabTax,
            "Slab tax"
        );

        addCalculatorBreakdown(
            result,
            "Slab tax: " + profileName,
            taxInSlab
        );
    }

    result.slabTax = slabTax;

    addCalculatorBreakdown(
        result,
        "Tax on slab income before rebate",
        result.slabTax
    );
}


// ============================================================
// Special-income tax
// ============================================================


void TaxCalculator::calculateSpecialIncomeTax(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    // Thresholds such as Section 112A's annual exemption are applied
    // once to the aggregate amount for that rule, not transaction by transaction.
    struct SpecialBucket {
        double amount = 0.0;
        vector<string> descriptions;
    };
    map<string, SpecialBucket> buckets;

    for (const IncomeComputationResult& headResult : result.incomeHeadResults) {
        for (const SpecialIncomeComponent& component : headResult.specialRateComponents) {
            auto& bucket = buckets[component.ruleCode];
            bucket.amount += component.amount;
            bucket.descriptions.push_back(component.description);
            checkCalculatorAmount(bucket.amount, "Aggregated special-rate income");
        }
    }

    double specialTax = 0.0;
    for (const auto& entry : buckets) {
        const string& code = entry.first;
        const SpecialBucket& bucket = entry.second;
        const SpecialIncomeRule& rule = input.rules.getSpecialIncomeRule(code);
        double annualThreshold = rule.hasAnnualThreshold
            ? rule.annualThreshold : 0.0;

        // Resident Individual/HUF may use any unused basic-exemption
        // amount against Section 112A LTCG. The annual 112A threshold
        // remains one aggregate threshold for the year.
        if (code == "112A" &&
            (input.taxpayer->getType() == TaxpayerType::HUF ||
             (input.taxpayer->getType() == TaxpayerType::Individual &&
              input.taxpayer->getResidentialStatus() == ResidentialStatus::Resident))) {
            const string profileName = getSlabProfileName(*input.taxpayer);
            const TaxSlabProfile& profile = input.rules.getSlabProfile(profileName);
            double basicExemption = 0.0;
            if (!profile.slabs.empty() && profile.slabs.front().ratePercent == 0.0) {
                basicExemption = profile.slabs.front().upperLimit;
            }
            const double otherIncome = max(0.0, result.totalIncome - bucket.amount);
            const double unusedBasicExemption = max(0.0, basicExemption - otherIncome);
            annualThreshold += unusedBasicExemption;
            if (unusedBasicExemption > 0.0) {
                result.notes.push_back(
                    "Unused basic-exemption limit was applied against Section 112A "
                    "LTCG for the resident taxpayer."
                );
            }
        }

        const double taxableAmount = rule.hasAnnualThreshold
            ? max(0.0, bucket.amount - annualThreshold)
            : bucket.amount;
        const double tax = taxableAmount * rule.ratePercent / 100.0;
        specialTax += tax;
        checkCalculatorAmount(specialTax, "Special-income tax");

        string description = code;
        if (!bucket.descriptions.empty()) {
            description = bucket.descriptions.front();
            if (bucket.descriptions.size() > 1) {
                description += " and other transactions";
            }
        }
        addCalculatorBreakdown(
            result,
            "Special-rate tax: " + description + " [" + code + "]" +
                (rule.hasAnnualThreshold ? " (annual threshold applied)" : ""),
            tax
        );
    }

    result.specialRateTax = specialTax;
    addCalculatorBreakdown(
        result,
        "Total special-rate tax before rebate",
        result.specialRateTax
    );
}


// ============================================================
// Rebate
// ============================================================


void TaxCalculator::calculateRebate(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    if (!input.rules.getBoolean("REBATE_87A_ENABLED")) {
        result.rebate = 0.0;

        addCalculatorBreakdown(
            result,
            "Rebate",
            0.0
        );

        return;
    }

    const bool residentIndividualOnly =
        input.rules.getBoolean(
            "REBATE_87A_RESIDENT_INDIVIDUAL_ONLY"
        );

    if (
        residentIndividualOnly &&
        (
            input.taxpayer->getType() !=
            TaxpayerType::Individual ||
            input.taxpayer->getResidentialStatus() !=
            ResidentialStatus::Resident
        )
    ) {
        result.rebate = 0.0;

        addCalculatorBreakdown(
            result,
            "Rebate",
            0.0
        );

        return;
    }

    const double rebateLimit =
        input.rules.getNumber(
            "REBATE_87A_TOTAL_INCOME_LIMIT"
        );

    const double rebateMaximum =
        input.rules.getNumber(
            "REBATE_87A_MAX_AMOUNT"
        );

    double taxBeforeRebate = result.slabTax;

    if (
        input.rules.getBoolean(
            "REBATE_87A_SPECIAL_RATE_TAX_ALLOWED"
        )
    ) {
        taxBeforeRebate += result.specialRateTax;
    }

    if (rebateMaximum <= 0.0) {
        result.rebate = 0.0;
        addCalculatorBreakdown(result, "Rebate", 0.0);
        return;
    }

    double rebate = 0.0;
    if (result.totalIncome <= rebateLimit) {
        rebate = min(taxBeforeRebate, rebateMaximum);
    } else if (
        input.rules.getBooleanOrDefault(
            "REBATE_87A_MARGINAL_RELIEF_ENABLED", false
        ) &&
        residentIndividualOnly &&
        input.taxpayer->getType() == TaxpayerType::Individual &&
        input.taxpayer->getResidentialStatus() == ResidentialStatus::Resident
    ) {
        // Marginal relief above the 87A threshold: tax after rebate may
        // not exceed the amount by which total income exceeds the threshold.
        const double incomeExcess = result.totalIncome - rebateLimit;
        const double marginalRelief =
            max(0.0, taxBeforeRebate - incomeExcess);
        rebate = min(rebateMaximum, marginalRelief);
        if (rebate > 0.0) {
            result.notes.push_back(
                "Section 87A marginal relief applied above the configured "
                "total-income threshold."
            );
        }
    }

    result.rebate = rebate;

    checkCalculatorAmount(
        result.rebate,
        "Rebate"
    );

    addCalculatorAdjustment(
        result,
        "REBATE_87A",
        "Section 87A rebate",
        taxBeforeRebate,
        rebate,
        "Rebate is applied before surcharge and cess."
    );

    addCalculatorBreakdown(
        result,
        "Less: Rebate",
        -result.rebate
    );
}


// ============================================================
// Surcharge
// ============================================================


void TaxCalculator::calculateSurcharge(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    double applicableRate = 0.0;

    for (int i = 1; i <= 4; ++i) {
        const string thresholdKey =
            "SURCHARGE_THRESHOLD_" + to_string(i);

        const string rateKey =
            "SURCHARGE_RATE_" + to_string(i);

        if (
            !input.rules.hasSetting(thresholdKey) ||
            !input.rules.hasSetting(rateKey)
        ) {
            continue;
        }

        const double threshold =
            input.rules.getNumber(thresholdKey);

        const double rate =
            input.rules.getNumber(rateKey);

        if (result.totalIncome > threshold) {
            applicableRate = rate;
        }
    }

    if (applicableRate <= 0.0) {
        result.surcharge = 0.0;

        addCalculatorBreakdown(
            result,
            "Surcharge",
            0.0
        );

        return;
    }

    // Surcharge applies to special-rate tax too. The surcharge on
    // specified capital-gain tax is capped at the configured percentage.
    map<string, double> specialAmounts;
    for (const IncomeComputationResult& headResult : result.incomeHeadResults) {
        for (const SpecialIncomeComponent& component : headResult.specialRateComponents) {
            specialAmounts[component.ruleCode] += component.amount;
        }
    }

    double specifiedCapitalGainTax = 0.0;
    for (const string code : {string("111A"), string("112"), string("112A")}) {
        const auto found = specialAmounts.find(code);
        if (found == specialAmounts.end()) continue;
        const SpecialIncomeRule& rule = input.rules.getSpecialIncomeRule(code);
        double taxable = found->second;
        if (rule.hasAnnualThreshold) taxable = max(0.0, taxable - rule.annualThreshold);
        specifiedCapitalGainTax += taxable * rule.ratePercent / 100.0;
    }

    const double otherSpecialTax =
        max(0.0, result.specialRateTax - specifiedCapitalGainTax);
    const double ordinaryTaxAfterRebate =
        max(0.0, result.slabTax - result.rebate);
    const double capitalGainSurchargeRate = min(
        applicableRate,
        input.rules.getNumber("SURCHARGE_SPECIFIED_CAPITAL_GAINS_MAX_PERCENT")
    );

    double surcharge =
        ordinaryTaxAfterRebate * applicableRate / 100.0 +
        otherSpecialTax * applicableRate / 100.0 +
        specifiedCapitalGainTax * capitalGainSurchargeRate / 100.0;

    // --------------------------------------------------------
    // Marginal relief
    // --------------------------------------------------------

    if (
        input.rules.getBoolean(
            "SURCHARGE_MARGINAL_RELIEF_REQUIRED"
        )
    ) {
        double nextThreshold = -1.0;

        for (int i = 1; i <= 4; ++i) {
            const string thresholdKey =
                "SURCHARGE_THRESHOLD_" + to_string(i);

            if (!input.rules.hasSetting(thresholdKey)) {
                continue;
            }

            const double threshold =
                input.rules.getNumber(thresholdKey);

            if (threshold > result.totalIncome) {
                if (
                    nextThreshold < 0.0 ||
                    threshold < nextThreshold
                ) {
                    nextThreshold = threshold;
                }
            }
        }

        if (nextThreshold > 0.0) {
            const double thresholdExcess =
                nextThreshold - result.totalIncome;

            const double taxBeforeSurcharge =
                result.slabTax +
                result.specialRateTax -
                result.rebate;

            const double taxAtThreshold =
                taxBeforeSurcharge +
                taxBeforeSurcharge *
                    applicableRate / 100.0;

            const double maximumPermittedTax =
                taxAtThreshold + thresholdExcess;

            const double actualTax =
                taxBeforeSurcharge + surcharge;

            if (actualTax > maximumPermittedTax) {
                const double relief =
                    actualTax - maximumPermittedTax;

                surcharge = max(0.0, surcharge - relief);

                result.notes.push_back(
                    "Marginal relief has been applied to "
                    "restrict the excess tax near the "
                    "surcharge threshold."
                );
            }
        }
    }

    result.surcharge = surcharge;

    checkCalculatorAmount(
        result.surcharge,
        "Surcharge"
    );

    addCalculatorBreakdown(
        result,
        "Add: Surcharge",
        result.surcharge
    );
}


// ============================================================
// Cess
// ============================================================


void TaxCalculator::calculateCess(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    const double cessRate =
        input.rules.getNumber(
            "HEALTH_AND_EDUCATION_CESS_RATE"
        );

    const double cessBase =
        result.slabTax +
        result.specialRateTax -
        result.rebate +
        result.surcharge;

    if (cessBase < 0.0) {
        throw logic_error(
            "Cess base cannot be negative."
        );
    }

    result.cess =
        cessBase * cessRate / 100.0;

    checkCalculatorAmount(
        result.cess,
        "Health and education cess"
    );

    addCalculatorBreakdown(
        result,
        "Add: Health and education cess",
        result.cess
    );

    result.totalTaxLiability =
        result.slabTax +
        result.specialRateTax -
        result.rebate +
        result.surcharge +
        result.cess;

    if (result.totalTaxLiability < 0.0) {
        result.totalTaxLiability = 0.0;
    }

    result.totalTaxLiability =
        roundToNearestRupee(
            result.totalTaxLiability
        );

    addCalculatorBreakdown(
        result,
        "Total tax liability before credits",
        result.totalTaxLiability
    );
}


// ============================================================
// Payment credits
// ============================================================


void TaxCalculator::applyPaymentCredits(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    const double totalCredits =
        input.payments.getTotalPaymentsAndCredits();

    result.totalPaymentsAndCredits = totalCredits;

    addCalculatorBreakdown(
        result,
        "Less: TDS, TCS, advance tax and self-assessment tax",
        -totalCredits
    );

    const double balance =
        result.totalTaxLiability - totalCredits;

    if (balance >= 0.0) {
        result.balanceTaxPayable = balance;
        result.refund = 0.0;
    } else {
        result.balanceTaxPayable = 0.0;
        result.refund = -balance;
    }

    result.balanceTaxPayable =
        roundToNearestRupee(
            result.balanceTaxPayable
        );

    result.refund =
        roundToNearestRupee(
            result.refund
        );

    addCalculatorBreakdown(
        result,
        "Balance tax payable",
        result.balanceTaxPayable
    );

    if (result.refund > 0.0) {
        addCalculatorBreakdown(
            result,
            "Refund",
            result.refund
        );
    }
}


// ============================================================
// Notes
// ============================================================


void TaxCalculator::addCalculationNotes(
    const TaxCalculationInput& input,
    TaxComputationResult& result
) {
    result.notes.push_back(
        "This calculation uses only data entered in the "
        "current session. No prior-year data, permanent "
        "taxpayer record, or document upload is used."
    );

    result.notes.push_back(
        "The calculator does not determine legal eligibility "
        "for exemptions, deductions, or payment credits."
    );

    result.notes.push_back(
        "Agricultural income, if declared, requires a separate "
        "rate-purpose check before this result is treated as "
        "final."
    );

    if (
        hasIncomeHead(
            input.incomeHeads,
            IncomeHead::CapitalGains
        )
    ) {
        result.notes.push_back(
            "Capital-gains entries must be classified by the "
            "applicable capital-gains category before final use."
        );
    }

    if (
        hasIncomeHead(
            input.incomeHeads,
            IncomeHead::BusinessProfession
        )
    ) {
        result.notes.push_back(
            "Business income is based on user-entered computed "
            "profit; depreciation and other business computations "
            "are not independently performed."
        );
    }
}


// ============================================================
// Public calculation methods
// ============================================================


TaxComputationResult TaxCalculator::calculate(
    const TaxCalculationInput& input
) const {
    validateInput(input);

    TaxComputationResult result;

    result.taxpayerType =
        input.taxpayer->getType();

    result.regime = input.rules.getRegimeType() == RegimeType::Old
        ? TaxRegime::Old
        : TaxRegime::New;

    // --------------------------------------------------------
    // Evaluate every income head.
    // --------------------------------------------------------

    vector<IncomeComputationResult> headResults =
        evaluateAllIncomeHeads(
            input,
            result
        );

    // Apply annual set-off before retaining the adjusted head results.
    aggregateIncomeHeads(headResults, input.rules.getRegimeType(), result);

    // Retain every evaluated and aggregated income-head result for reporting.
    result.incomeHeadResults = headResults;

    // --------------------------------------------------------
    // Deductions and total income.
    // --------------------------------------------------------

    calculateDeductions(
        input,
        result.grossTotalIncome,
        result
    );

    // --------------------------------------------------------
    // Tax computation.
    // --------------------------------------------------------

    calculateSlabTax(input, result);

    calculateSpecialIncomeTax(input, result);

    calculateRebate(input, result);

    calculateSurcharge(input, result);

    calculateCess(input, result);

    applyPaymentCredits(input, result);

    addCalculationNotes(input, result);

    return result;
}


TaxComputationResult TaxCalculator::calculate(
    const Taxpayer& taxpayer,
    const TaxRules& rules,
    Income& incomeHead,
    const Deductions& deductions,
    const Payments& payments
) const {
    TaxCalculationInput input;

    input.taxpayer = &taxpayer;
    input.rules = rules;
    input.incomeHeads.push_back(&incomeHead);
    input.deductions = deductions;
    input.payments = payments;

    return calculate(input);
}