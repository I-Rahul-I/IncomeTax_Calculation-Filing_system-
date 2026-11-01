#include "CapitalGainIncome.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

string trimCapitalText(const string& value) {
    const string whitespace = " \t\n\r\f\v";
    const size_t first = value.find_first_not_of(whitespace);

    if (first == string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(whitespace);

    return value.substr(first, last - first + 1);
}

bool isValidCapitalDate(const string& value) {
    if (
        value.size() != 10 ||
        value[4] != '-' ||
        value[7] != '-'
    ) {
        return false;
    }

    for (size_t i = 0; i < value.size(); ++i) {
        if (i == 4 || i == 7) {
            continue;
        }

        if (value[i] < '0' || value[i] > '9') {
            return false;
        }
    }

    const int year = stoi(value.substr(0, 4));
    const int month = stoi(value.substr(5, 2));
    const int day = stoi(value.substr(8, 2));

    if (year < 1 || month < 1 || month > 12) {
        return false;
    }

    int monthDays[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (
        year % 400 == 0 ||
        (year % 4 == 0 && year % 100 != 0)
    ) {
        monthDays[1] = 29;
    }

    return day >= 1 && day <= monthDays[month - 1];
}

void checkCapitalNumber(
    double value,
    const string& description
) {
    if (!isfinite(value)) {
        throw overflow_error(
            description + " exceeded numeric limits."
        );
    }
}

void addCapitalBreakdown(
    IncomeComputationResult& result,
    const string& description,
    double amount
) {
    IncomeBreakdownItem item;
    item.description = description;
    item.amount = amount;

    result.breakdown.push_back(item);
}

bool isSpecifiedEquityAsset(CapitalAssetType type) {
    return
        type == CapitalAssetType::ListedEquityShare ||
        type == CapitalAssetType::EquityOrientedMutualFund ||
        type == CapitalAssetType::BusinessTrustUnit;
}

} // namespace

// ============================================================
// Constructor
// ============================================================

CapitalGainIncome::CapitalGainIncome(
    const string& description
)
    : Income(description),
      assetType(CapitalAssetType::Other),
      gainTerm(CapitalGainTerm::ShortTerm),
      taxTreatment(CapitalGainTaxTreatment::NormalRate),
      assetDescription(""),
      acquisitionDate(""),
      transferDate(""),
      saleConsideration(0.0),
      stampDutyValue(0.0),
      grandfatheredFMV(0.0),
      acquisitionCost(0.0),
      improvementCost(0.0),
      transferExpenses(0.0),
      exemptionClaimed(0.0),
      exemptionSection(""),
      sttPaidOnAcquisition(false),
      sttPaidOnTransfer(false) {
}

// ============================================================
// Validation helpers
// ============================================================

void CapitalGainIncome::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}

void CapitalGainIncome::validateDate(
    const string& date,
    const string& fieldName
) {
    if (!isValidCapitalDate(date)) {
        throw invalid_argument(
            fieldName + " must be a valid YYYY-MM-DD date."
        );
    }
}

void CapitalGainIncome::validateTransaction() const {
    validateDate(acquisitionDate, "Acquisition date");
    validateDate(transferDate, "Transfer date");

    if (acquisitionDate > transferDate) {
        throw invalid_argument(
            "Acquisition date cannot be after transfer date."
        );
    }

    if (
        transferDate < "2025-04-01" ||
        transferDate > "2026-03-31"
    ) {
        throw invalid_argument(
            "Transfer date must fall within FY 2025-26."
        );
    }

    if (assetDescription.empty()) {
        throw invalid_argument(
            "Enter an asset description before evaluation."
        );
    }

    // Section 50AA specified debt mutual funds acquired on/after
    // 1 April 2023 are deemed short-term regardless of holding period
    // and their gains are taxed at applicable normal rates. For other
    // debt funds, the caller must provide the correct classification.
    if (assetType == CapitalAssetType::DebtMutualFund) {
        if (acquisitionDate >= "2023-04-01") {
            if (gainTerm != CapitalGainTerm::ShortTerm ||
                taxTreatment != CapitalGainTaxTreatment::NormalRate) {
                throw invalid_argument(
                    "For a specified debt mutual fund acquired on/after "
                    "2023-04-01, use Short-Term and Normal Rate under "
                    "the Section 50AA route."
                );
            }
        } else if (gainTerm == CapitalGainTerm::ShortTerm) {
            if (taxTreatment != CapitalGainTaxTreatment::NormalRate) {
                throw invalid_argument(
                    "Short-term debt mutual-fund gains use normal rates."
                );
            }
        } else if (taxTreatment != CapitalGainTaxTreatment::Section112) {
            throw invalid_argument(
                "Long-term debt mutual-fund gains must use Section 112."
            );
        }
    }

    // A positive exemption is an independently determined amount supplied
    // by the taxpayer. Apply it only against positive gain, but make no
    // legal eligibility representation; see the result's limitation note.

    if (
        isSpecifiedEquityAsset(assetType) &&
        improvementCost > 0.0
    ) {
        throw invalid_argument(
            "Improvement cost is not supported for the "
            "selected securities transaction."
        );
    }

    if (isSpecifiedEquityAsset(assetType)) {
        // Listed shares/equity-oriented funds use this supported
        // holding-period classification. Business-trust units
        // require independently verified term classification
        // because listing details are not collected here.
        if (assetType != CapitalAssetType::BusinessTrustUnit) {
            // Comparing YYYY-MM-DD dates implements a calendar
            // anniversary, rather than assuming 365 days.
            string anniversary = acquisitionDate;

            const int acquisitionYear =
                stoi(acquisitionDate.substr(0, 4));

            if (acquisitionYear >= 9999) {
                throw invalid_argument(
                    "Acquisition year is outside the supported range."
                );
            }

            const string nextYear =
                to_string(acquisitionYear + 1);

            anniversary =
                string(4 - nextYear.size(), '0') +
                nextYear +
                acquisitionDate.substr(4);

            // Calendar anniversary for 29 February.
            if (!isValidCapitalDate(anniversary)) {
                anniversary =
                    anniversary.substr(0, 4) + "-02-28";
            }

            const bool declaredLongTerm =
                gainTerm == CapitalGainTerm::LongTerm;

            const bool exceedsOneYear =
                transferDate > anniversary;

            if (declaredLongTerm != exceedsOneYear) {
                throw invalid_argument(
                    "Declared term does not match the supported "
                    "12-month securities holding-period check."
                );
            }
        }

        if (
            gainTerm == CapitalGainTerm::ShortTerm &&
            taxTreatment != CapitalGainTaxTreatment::Section111A
        ) {
            throw invalid_argument(
                "This initial securities module supports "
                "short-term transfers only through Section 111A."
            );
        }

        if (
            gainTerm == CapitalGainTerm::LongTerm &&
            taxTreatment != CapitalGainTaxTreatment::Section112A
        ) {
            throw invalid_argument(
                "This initial securities module supports "
                "long-term transfers only through Section 112A."
            );
        }

        if (!sttPaidOnTransfer) {
            throw invalid_argument(
                "The supported 111A/112A route requires STT "
                "on transfer. Special exceptions are not implemented."
            );
        }

        if (
            taxTreatment == CapitalGainTaxTreatment::Section112A &&
            assetType == CapitalAssetType::ListedEquityShare &&
            !sttPaidOnAcquisition
        ) {
            throw invalid_argument(
                "This 112A equity-share route requires STT "
                "on acquisition. Acquisition exceptions need "
                "a separate implementation."
            );
        }

        if (
            taxTreatment == CapitalGainTaxTreatment::Section112A &&
            acquisitionDate < "2018-02-01" &&
            grandfatheredFMV <= 0.0
        ) {
            throw invalid_argument(
                "Section 112A assets acquired before 2018-02-01 require "
                "the fair market value as of 2018-01-31 for the "
                "grandfathered cost calculation."
            );
        }
    } else {
        // Supported non-security assets here are land/building
        // and gold. Term classification remains user-declared.
        if (gainTerm == CapitalGainTerm::ShortTerm) {
            if (
                taxTreatment !=
                CapitalGainTaxTreatment::NormalRate
            ) {
                throw invalid_argument(
                    "The selected short-term asset must use "
                    "the supported normal-rate route."
                );
            }
        } else if (
            taxTreatment != CapitalGainTaxTreatment::Section112
        ) {
            throw invalid_argument(
                "The selected long-term asset must use "
                "the supported Section 112 route."
            );
        }
    }

    if (
        taxTreatment == CapitalGainTaxTreatment::Section111A &&
        gainTerm != CapitalGainTerm::ShortTerm
    ) {
        throw invalid_argument(
            "Section 111A requires a short-term classification."
        );
    }

    if (
        (
            taxTreatment == CapitalGainTaxTreatment::Section112 ||
            taxTreatment == CapitalGainTaxTreatment::Section112A
        ) &&
        gainTerm != CapitalGainTerm::LongTerm
    ) {
        throw invalid_argument(
            "The selected route requires a long-term classification."
        );
    }
}

// ============================================================
// Setters
// ============================================================

void CapitalGainIncome::setAssetType(CapitalAssetType type) {
    switch (type) {
        case CapitalAssetType::ListedEquityShare:
        case CapitalAssetType::EquityOrientedMutualFund:
        case CapitalAssetType::BusinessTrustUnit:
        case CapitalAssetType::LandOrBuilding:
        case CapitalAssetType::Gold:
        case CapitalAssetType::DebtMutualFund:
        case CapitalAssetType::UnlistedShare:
        case CapitalAssetType::Other:
            assetType = type;
            return;

        default:
            throw invalid_argument("Invalid capital asset type.");
    }
}

void CapitalGainIncome::setGainTerm(CapitalGainTerm term) {
    if (
        term != CapitalGainTerm::ShortTerm &&
        term != CapitalGainTerm::LongTerm
    ) {
        throw invalid_argument("Invalid capital-gain term.");
    }

    gainTerm = term;
}

void CapitalGainIncome::setTaxTreatment(
    CapitalGainTaxTreatment treatment
) {
    switch (treatment) {
        case CapitalGainTaxTreatment::NormalRate:
        case CapitalGainTaxTreatment::Section111A:
        case CapitalGainTaxTreatment::Section112:
        case CapitalGainTaxTreatment::Section112A:
            taxTreatment = treatment;
            return;

        default:
            throw invalid_argument(
                "Invalid capital-gain tax treatment."
            );
    }
}

void CapitalGainIncome::setAssetDescription(
    const string& description
) {
    const string cleaned = trimCapitalText(description);

    if (cleaned.empty()) {
        throw invalid_argument(
            "Asset description cannot be empty."
        );
    }

    assetDescription = cleaned;
}

void CapitalGainIncome::setAcquisitionDate(const string& date) {
    validateDate(date, "Acquisition date");
    acquisitionDate = date;
}

void CapitalGainIncome::setTransferDate(const string& date) {
    validateDate(date, "Transfer date");
    transferDate = date;
}

void CapitalGainIncome::setSaleConsideration(double amount) {
    validateAmount(amount, "Sale consideration");
    saleConsideration = amount;
}

void CapitalGainIncome::setStampDutyValue(double amount) {
    validateAmount(amount, "Stamp-duty value");
    stampDutyValue = amount;
}

void CapitalGainIncome::setGrandfatheredFMV(double amount) {
    validateAmount(amount, "Grandfathered fair market value");
    grandfatheredFMV = amount;
}

void CapitalGainIncome::setAcquisitionCost(double amount) {
    validateAmount(amount, "Acquisition cost");
    acquisitionCost = amount;
}

void CapitalGainIncome::setImprovementCost(double amount) {
    validateAmount(amount, "Improvement cost");
    improvementCost = amount;
}

void CapitalGainIncome::setTransferExpenses(double amount) {
    validateAmount(amount, "Transfer expenses");
    transferExpenses = amount;
}

void CapitalGainIncome::setExemptionClaim(
    double amount,
    const string& section
) {
    validateAmount(amount, "Capital-gain exemption");

    const string cleanedSection = trimCapitalText(section);

    if (amount > 0.0 && cleanedSection.empty()) {
        throw invalid_argument(
            "A positive exemption requires a section reference."
        );
    }

    exemptionClaimed = amount;
    exemptionSection = cleanedSection;
}

void CapitalGainIncome::setSTTPaidOnAcquisition(bool paid) {
    sttPaidOnAcquisition = paid;
}

void CapitalGainIncome::setSTTPaidOnTransfer(bool paid) {
    sttPaidOnTransfer = paid;
}

// ============================================================
// Getters
// ============================================================

CapitalAssetType CapitalGainIncome::getAssetType() const {
    return assetType;
}

CapitalGainTerm CapitalGainIncome::getGainTerm() const {
    return gainTerm;
}

CapitalGainTaxTreatment
CapitalGainIncome::getTaxTreatment() const {
    return taxTreatment;
}

string CapitalGainIncome::getAssetTypeName() const {
    switch (assetType) {
        case CapitalAssetType::ListedEquityShare:
            return "Listed Equity Share";

        case CapitalAssetType::EquityOrientedMutualFund:
            return "Equity-Oriented Mutual Fund";

        case CapitalAssetType::BusinessTrustUnit:
            return "Business Trust Unit";

        case CapitalAssetType::LandOrBuilding:
            return "Land or Building";

        case CapitalAssetType::Gold:
            return "Gold";

        case CapitalAssetType::DebtMutualFund:
            return "Debt Mutual Fund";

        case CapitalAssetType::UnlistedShare:
            return "Unlisted Share";

        case CapitalAssetType::Other:
            return "Other";

        default:
            throw logic_error("Unexpected capital asset type.");
    }
}

string CapitalGainIncome::getGainTermName() const {
    return gainTerm == CapitalGainTerm::ShortTerm
        ? "Short-Term"
        : "Long-Term";
}

string CapitalGainIncome::getTaxTreatmentName() const {
    switch (taxTreatment) {
        case CapitalGainTaxTreatment::NormalRate:
            return "Normal Slab Rates";

        case CapitalGainTaxTreatment::Section111A:
            return "Section 111A";

        case CapitalGainTaxTreatment::Section112:
            return "Section 112";

        case CapitalGainTaxTreatment::Section112A:
            return "Section 112A";

        default:
            throw logic_error("Unexpected capital tax treatment.");
    }
}

string CapitalGainIncome::getSpecialRuleCode() const {
    switch (taxTreatment) {
        case CapitalGainTaxTreatment::NormalRate:
            return "";

        case CapitalGainTaxTreatment::Section111A:
            return "111A";

        case CapitalGainTaxTreatment::Section112:
            return "112";

        case CapitalGainTaxTreatment::Section112A:
            return "112A";

        default:
            throw logic_error("Unexpected capital tax treatment.");
    }
}

const string& CapitalGainIncome::getAssetDescription() const {
    return assetDescription;
}

const string& CapitalGainIncome::getAcquisitionDate() const {
    return acquisitionDate;
}

const string& CapitalGainIncome::getTransferDate() const {
    return transferDate;
}

double CapitalGainIncome::getSaleConsideration() const {
    return saleConsideration;
}

double CapitalGainIncome::getStampDutyValue() const {
    return stampDutyValue;
}

double CapitalGainIncome::getGrandfatheredFMV() const {
    return grandfatheredFMV;
}

double CapitalGainIncome::getEffectiveAcquisitionCost() const {
    if (taxTreatment != CapitalGainTaxTreatment::Section112A ||
        acquisitionDate >= "2018-02-01" ||
        grandfatheredFMV <= 0.0) {
        return acquisitionCost;
    }

    // Section 55 grandfathering: higher of actual cost and the lower of
    // 31-Jan-2018 FMV and full sale consideration.
    const double cappedFMV = min(grandfatheredFMV, saleConsideration);
    const double deemedCost = max(acquisitionCost, cappedFMV);
    checkCapitalNumber(deemedCost, "Grandfathered acquisition cost");
    return deemedCost;
}

double CapitalGainIncome::getEffectiveSaleConsideration() const {
    if (assetType != CapitalAssetType::LandOrBuilding ||
        stampDutyValue <= 0.0) {
        return saleConsideration;
    }

    // Simplified Section 50C tolerance: stamp value substitutes only
    // when it exceeds actual consideration by more than 10%.
    const double toleranceCeiling = saleConsideration * 1.10;
    checkCapitalNumber(toleranceCeiling, "Section 50C tolerance ceiling");

    return stampDutyValue > toleranceCeiling
        ? stampDutyValue
        : saleConsideration;
}

double CapitalGainIncome::getAcquisitionCost() const {
    return acquisitionCost;
}

double CapitalGainIncome::getImprovementCost() const {
    return improvementCost;
}

double CapitalGainIncome::getTransferExpenses() const {
    return transferExpenses;
}

double CapitalGainIncome::getExemptionClaimed() const {
    return exemptionClaimed;
}

const string& CapitalGainIncome::getExemptionSection() const {
    return exemptionSection;
}

bool CapitalGainIncome::isSTTPaidOnAcquisition() const {
    return sttPaidOnAcquisition;
}

bool CapitalGainIncome::isSTTPaidOnTransfer() const {
    return sttPaidOnTransfer;
}

// ============================================================
// Computation helpers
// ============================================================

double CapitalGainIncome::getTotalEnteredCosts() const {
    const double total =
        getEffectiveAcquisitionCost() + improvementCost + transferExpenses;

    checkCapitalNumber(total, "Total capital costs");

    return total;
}

double CapitalGainIncome::getGainBeforeExemption() const {
    const double gain =
        getEffectiveSaleConsideration() - getTotalEnteredCosts();

    checkCapitalNumber(gain, "Capital gain");

    return gain;
}

double CapitalGainIncome::getGainAfterExemption() const {
    const double gain = getGainBeforeExemption();

    if (gain <= 0.0) {
        return gain;
    }

    return max(0.0, gain - exemptionClaimed);
}

// ============================================================
// Evaluation
// ============================================================

IncomeHead CapitalGainIncome::getHead() const {
    return IncomeHead::CapitalGains;
}

string CapitalGainIncome::getHeadName() const {
    return "Capital Gains";
}

IncomeComputationResult CapitalGainIncome::evaluate(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before evaluating capital gains."
        );
    }

    if (
        taxpayer.getType() != TaxpayerType::Individual &&
        taxpayer.getType() != TaxpayerType::HUF
    ) {
        throw invalid_argument(
            "Current capital-gains coverage supports "
            "Individual and HUF taxpayers only."
        );
    }

    const string supportKey =
        taxpayer.getType() == TaxpayerType::Individual
            ? "SUPPORT_INDIVIDUAL"
            : "SUPPORT_HUF";

    if (!rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "Taxpayer category is disabled in this configuration."
        );
    }

    validateTransaction();

    const double gain = getGainAfterExemption();

    IncomeComputationResult result;

    result.head = getHead();
    result.description = getDescription();
    const double effectiveConsideration =
        getEffectiveSaleConsideration();

    result.grossAmount = effectiveConsideration;

    addCapitalBreakdown(
        result, "Actual sale consideration", saleConsideration
    );

    if (
        assetType == CapitalAssetType::LandOrBuilding &&
        effectiveConsideration > saleConsideration
    ) {
        addCapitalBreakdown(
            result,
            "Section 50C deemed consideration (stamp-duty value substituted)",
            effectiveConsideration - saleConsideration
        );
        result.notes.push_back(
            "Section 50C simplified route applied: stamp-duty value "
            "exceeded actual consideration by more than 10%. "
            "Agreement-date/registration-date alternatives and "
            "valuation-officer reference are not modeled."
        );
    } else if (
        assetType == CapitalAssetType::LandOrBuilding &&
        stampDutyValue > 0.0
    ) {
        result.notes.push_back(
            "Section 50C 10% tolerance applied: stamp-duty value "
            "did not exceed actual consideration by more than 10%."
        );
    }

    addCapitalBreakdown(
        result, "Less: Acquisition cost", -getEffectiveAcquisitionCost()
    );

    if (getEffectiveAcquisitionCost() != acquisitionCost) {
        result.notes.push_back(
            "Section 112A grandfathered cost applied: effective acquisition "
            "cost is the higher of actual cost and the lower of 31-Jan-2018 "
            "FMV and sale consideration. Verify the asset's eligibility and "
            "FMV evidence."
        );
    }

    addCapitalBreakdown(
        result, "Less: Improvement cost", -improvementCost
    );

    addCapitalBreakdown(
        result, "Less: Transfer expenses", -transferExpenses
    );

    addCapitalBreakdown(
        result, "Transaction gain / loss", gain
    );

    result.notes.push_back(
        "Asset: " + assetDescription + "."
    );

    result.notes.push_back(
        "Classification: " + getGainTermName() +
        "; treatment: " + getTaxTreatmentName() + "."
    );

    result.notes.push_back(
        "Transfer date: " + transferDate + "."
    );

    result.notes.push_back(
        "Entered transfer expenses must exclude STT."
    );

    if (exemptionClaimed > 0.0) {
        result.notes.push_back(
            "The user-entered exemption claim under " + exemptionSection +
            " was applied up to the positive transaction gain. "
            "The software does not validate the statutory section's "
            "eligibility, investment timing, amount cap, or supporting evidence."
        );
    }

    if (assetType == CapitalAssetType::DebtMutualFund &&
        acquisitionDate >= "2023-04-01") {
        result.notes.push_back(
            "Section 50AA deemed-short-term treatment was applied to "
            "the specified debt mutual-fund acquisition date supplied."
        );
    }

    if (
        assetType == CapitalAssetType::BusinessTrustUnit ||
        assetType == CapitalAssetType::Gold
    ) {
        result.notes.push_back(
            "Holding-period classification is user-declared "
            "for this asset type and requires verification."
        );
    }

    // Preserve capital-loss identity for the future aggregation
    // module. Do not offset it against normal income here.
    if (gain < 0.0) {
        IncomeAdjustment pendingLoss;

        pendingLoss.code =
            gainTerm == CapitalGainTerm::ShortTerm
                ? "CAPITAL_LOSS_PENDING_ST"
                : "CAPITAL_LOSS_PENDING_LT";

        pendingLoss.description =
            getGainTermName() +
            " capital loss awaiting aggregation";

        pendingLoss.claimedAmount = -gain;
        pendingLoss.allowedAmount = 0.0;

        pendingLoss.explanation =
            "Pending capital-head loss set-off. Not an ordinary "
            "income deduction or Chapter VI-A deduction.";

        result.adjustments.push_back(pendingLoss);

        result.notes.push_back(
            "Capital loss is pending aggregation and has not "
            "been subtracted from ordinary income."
        );

        return result;
    }

    if (gain == 0.0) {
        return result;
    }

    // Metadata lets the annual aggregation engine classify positive
    // capital gains by holding period before applying statutory loss set-off.
    IncomeAdjustment gainClassification;
    gainClassification.code =
        gainTerm == CapitalGainTerm::ShortTerm
            ? "CAPITAL_GAIN_ST"
            : "CAPITAL_GAIN_LT";
    gainClassification.description =
        getGainTermName() + " capital gain classification";
    gainClassification.claimedAmount = gain;
    gainClassification.allowedAmount = 0.0;
    gainClassification.explanation =
        "Internal annual aggregation marker; not a deduction.";
    result.adjustments.push_back(gainClassification);

    if (
        taxTreatment == CapitalGainTaxTreatment::NormalRate
    ) {
        result.normalRateIncome = gain;
    } else {
        const string code = getSpecialRuleCode();

        // Require the configured rule even though tax itself
        // is calculated later after annual aggregation.
        rules.getSpecialIncomeRule(code);

        SpecialIncomeComponent component;

        component.ruleCode = code;
        component.description = assetDescription;
        component.amount = gain;
        component.relevantDate = transferDate;

        result.specialRateComponents.push_back(component);

        result.notes.push_back(
            "Annual thresholds, permitted capital-loss set-off, "
            "basic-exemption adjustments, and special-rate tax "
            "must be calculated after aggregation."
        );
    }

    return result;
}