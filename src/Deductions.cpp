#include "Deductions.h"


#include <algorithm>
#include <cmath>
#include <stdexcept>


using namespace std;


namespace {


string trimDeductionText(const string& value) {
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


void checkDeductionTotal(
    double amount,
    const string& description
) {
    if (!isfinite(amount)) {
        throw overflow_error(
            description + " exceeded numeric limits."
        );
    }
}


} // namespace


// ============================================================
// Constructor
// ============================================================


Deductions::Deductions(
    const string& description
) : description(description) {
}


// ============================================================
// Validation
// ============================================================


void Deductions::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}


void Deductions::validateClaim(
    const DeductionClaim& claim
) {
    // Also validates the enumeration.
    getDeductionTypeName(claim.type);

    // Also validates the rule-code mapping.
    getDeductionRuleCode(claim.type);

    if (trimDeductionText(claim.description).empty()) {
        throw invalid_argument(
            "Deduction claim description cannot be empty."
        );
    }

    validateAmount(
        claim.claimedAmount,
        "Claimed deduction"
    );
    validateAmount(claim.reimbursementAmount, "Medical reimbursement");
    if (claim.patientAge < 0 || claim.patientAge > 120) {
        throw invalid_argument("Patient age must be between 0 and 120.");
    }
    if (claim.type == DeductionType::Section80DDB && claim.patientAge == 0) {
        throw invalid_argument("Section 80DDB requires the patient's age.");
    }
    if (claim.reimbursementAmount > claim.claimedAmount) {
        throw invalid_argument("Reimbursement cannot exceed the medical expense claim.");
    }

    if (claim.claimedAmount == 0.0) {
        throw invalid_argument(
            "Claimed deduction amount must be greater than zero."
        );
    }
}


// ============================================================
// Claim management
// ============================================================


void Deductions::addClaim(
    const DeductionClaim& claim
) {
    validateClaim(claim);

    DeductionClaim cleanedClaim = claim;

    cleanedClaim.description =
        trimDeductionText(claim.description);

    cleanedClaim.remarks =
        trimDeductionText(claim.remarks);

    claims.push_back(cleanedClaim);
}


void Deductions::clearClaims() {
    claims.clear();
}


const vector<DeductionClaim>&
Deductions::getClaims() const {
    return claims;
}


// ============================================================
// Totals
// ============================================================


double Deductions::getClaimedAmountByType(
    DeductionType type
) const {
    getDeductionTypeName(type);

    double total = 0.0;

    for (const DeductionClaim& claim : claims) {
        if (claim.type == type) {
            total += claim.claimedAmount;

            checkDeductionTotal(
                total,
                "Deduction category claimed total"
            );
        }
    }

    return total;
}


double Deductions::getTotalClaimedAmount() const {
    double total = 0.0;

    for (const DeductionClaim& claim : claims) {
        total += claim.claimedAmount;

        checkDeductionTotal(
            total,
            "Total claimed deductions"
        );
    }

    return total;
}


// ============================================================
// Display helpers
// ============================================================


string Deductions::getDeductionTypeName(
    DeductionType type
) {
    switch (type) {
        case DeductionType::Section80C:
            return "Section 80C";

        case DeductionType::Section80CCC:
            return "Section 80CCC";

        case DeductionType::Section80CCD1:
            return "Section 80CCD(1)";

        case DeductionType::Section80CCD1B:
            return "Section 80CCD(1B)";

        case DeductionType::Section80CCD2:
            return "Section 80CCD(2)";

        case DeductionType::Section80CCG:
            return "Section 80CCG";

        case DeductionType::Section80D:
            return "Section 80D";

        case DeductionType::Section80DD:
            return "Section 80DD";

        case DeductionType::Section80DDB:
            return "Section 80DDB";

        case DeductionType::Section80E:
            return "Section 80E";

        case DeductionType::Section80EE:
            return "Section 80EE";

        case DeductionType::Section80EEA:
            return "Section 80EEA";

        case DeductionType::Section80EEB:
            return "Section 80EEB";

        case DeductionType::Section80G:
            return "Section 80G";

        case DeductionType::Section80GGB:
            return "Section 80GGB";

        case DeductionType::Section80GGC:
            return "Section 80GGC";

        case DeductionType::Section80QQB:
            return "Section 80QQB";

        case DeductionType::Section80RRB:
            return "Section 80RRB";

        case DeductionType::Section80TTA:
            return "Section 80TTA";

        case DeductionType::Section80TTB:
            return "Section 80TTB";

        case DeductionType::Section80U:
            return "Section 80U";

        default:
            throw invalid_argument(
                "Invalid Chapter VI-A deduction category."
            );
    }
}


string Deductions::getDeductionRuleCode(
    DeductionType type
) {
    switch (type) {
        case DeductionType::Section80C:
            return "80C";

        case DeductionType::Section80CCC:
            return "80CCC";

        case DeductionType::Section80CCD1:
            return "80CCD_1";

        case DeductionType::Section80CCD1B:
            return "80CCD_1B";

        case DeductionType::Section80CCD2:
            return "80CCD_2";

        case DeductionType::Section80CCG:
            return "80CCG";

        case DeductionType::Section80D:
            return "80D";

        case DeductionType::Section80DD:
            return "80DD";

        case DeductionType::Section80DDB:
            return "80DDB";

        case DeductionType::Section80E:
            return "80E";

        case DeductionType::Section80EE:
            return "80EE";

        case DeductionType::Section80EEA:
            return "80EEA";

        case DeductionType::Section80EEB:
            return "80EEB";

        case DeductionType::Section80G:
            return "80G";

        case DeductionType::Section80GGB:
            return "80GGB";

        case DeductionType::Section80GGC:
            return "80GGC";

        case DeductionType::Section80QQB:
            return "80QQB";

        case DeductionType::Section80RRB:
            return "80RRB";

        case DeductionType::Section80TTA:
            return "80TTA";

        case DeductionType::Section80TTB:
            return "80TTB";

        case DeductionType::Section80U:
            return "80U";

        default:
            throw invalid_argument(
                "Invalid Chapter VI-A deduction category."
            );
    }
}


// ============================================================
// Evaluation
// ============================================================


vector<DeductionResult> Deductions::evaluateClaims(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before evaluating deductions."
        );
    }

    string supportKey;

    switch (taxpayer.getType()) {
        case TaxpayerType::Individual:
            supportKey = "SUPPORT_INDIVIDUAL";
            break;

        case TaxpayerType::HUF:
            supportKey = "SUPPORT_HUF";
            break;

        default:
            throw invalid_argument(
                "The current deduction module supports "
                "Individual and HUF taxpayers only."
            );
    }

    if (!rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "Taxpayer category is disabled in this configuration."
        );
    }

    vector<DeductionResult> results;

    // Evaluate every category that has at least one claim.
    for (int code = 1; code <= 21; ++code) {
        const DeductionType type =
            static_cast<DeductionType>(code);

        const double claimed =
            getClaimedAmountByType(type);

        if (claimed == 0.0) {
            continue;
        }

        const string ruleCode =
            getDeductionRuleCode(type);

        // This also enforces that the category is configured.
        const DeductionRule& rule =
            rules.getDeductionRule(ruleCode);

        if (!rule.allowed) {
            DeductionResult result;

            result.type = type;
            result.ruleCode = ruleCode;
            result.description =
                getDeductionTypeName(type);
            result.claimedAmount = claimed;
            result.allowedAmount = 0.0;
            result.explanation =
                "This deduction is not allowed under the "
                "currently loaded regime configuration.";

            results.push_back(result);

            continue;
        }

        DeductionResult result;

        result.type = type;
        result.ruleCode = ruleCode;
        result.description =
            getDeductionTypeName(type);
        result.claimedAmount = claimed;

        double eligibleClaim = claimed;
        double applicableMaximum = rule.maximumAmount;
        bool hasApplicableMaximum = rule.hasMaximumAmount;

        // Section 80DDB: deduction is net of reimbursement and is
        // limited to Rs 40,000, or Rs 1,00,000 for a senior patient.
        if (type == DeductionType::Section80DDB) {
            eligibleClaim = 0.0;
            applicableMaximum = 40000.0;
            for (const DeductionClaim& detail : claims) {
                if (detail.type == type) {
                    eligibleClaim += max(0.0, detail.claimedAmount - detail.reimbursementAmount);
                    if (detail.patientAge >= 60) {
                        applicableMaximum = 100000.0;
                    }
                }
            }
            hasApplicableMaximum = true;
        }

        // Section-specific taxpayer eligibility. A supported rule can be
        // present in configuration while the claim remains ineligible for
        // this taxpayer category/residency.
        bool taxpayerEligible = true;
        string ineligibleReason;
        if (type == DeductionType::Section80CCG) {
            taxpayerEligible = false; // Rajiv Gandhi Equity Savings Scheme is closed to fresh claims.
            ineligibleReason = "Section 80CCG is not available for a fresh FY 2025-26 claim.";
        } else if (type == DeductionType::Section80EE ||
                   type == DeductionType::Section80EEA ||
                   type == DeductionType::Section80EEB ||
                   type == DeductionType::Section80E) {
            taxpayerEligible = taxpayer.getType() == TaxpayerType::Individual;
            ineligibleReason = "This deduction is available to eligible individuals, not HUFs.";
        } else if (type == DeductionType::Section80QQB ||
                   type == DeductionType::Section80RRB) {
            taxpayerEligible = taxpayer.getType() == TaxpayerType::Individual &&
                taxpayer.getResidentialStatus() == ResidentialStatus::Resident;
            ineligibleReason = "This deduction requires an eligible resident individual claimant.";
        } else if (type == DeductionType::Section80GGB) {
            taxpayerEligible = false; // App currently supports only individual/HUF entities.
            ineligibleReason = "Section 80GGB is for eligible Indian companies; company taxpayers are not supported in this application.";
        }

        if (!taxpayerEligible) {
            result.allowedAmount = 0.0;
            result.explanation = ineligibleReason;
            results.push_back(result);
            continue;
        }

        double allowed = eligibleClaim;
        if (hasApplicableMaximum) {
            allowed = min(allowed, applicableMaximum);
        }
        result.allowedAmount = allowed;

        if (hasApplicableMaximum && eligibleClaim > allowed) {
            result.explanation =
                "Claim restricted to the applicable statutory maximum "
                "amount. The GTI cap has not yet been applied.";
        } else {
            result.explanation =
                "Claim allowed subject to later combined-ceiling "
                "and GTI-cap checks.";
        }

        results.push_back(result);
    }

    // --------------------------------------------------------
    // Important structural note
    // --------------------------------------------------------
    //
    // 80C, 80CCC and 80CCD(1) have a combined ceiling.
    // This module intentionally does not merge or cap them
    // here, because that aggregate rule belongs to
    // TaxCalculator.
    //
    // TaxCalculator must inspect these three codes together
    // before applying the GTI cap.

    return results;
}


double Deductions::getTotalAllowedBeforeGtiCap(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    const vector<DeductionResult> results =
        evaluateClaims(taxpayer, rules);

    double total = 0.0;

    for (const DeductionResult& result : results) {
        total += result.allowedAmount;

        checkDeductionTotal(
            total,
            "Total allowed deductions before GTI cap"
        );
    }

    return total;
}