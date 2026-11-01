#include "HousePropertyIncome.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

void addPropertyBreakdown(
    IncomeComputationResult& result,
    const string& description,
    double amount
) {
    IncomeBreakdownItem item;

    item.description = description;
    item.amount = amount;

    result.breakdown.push_back(item);
}

void addPropertyAdjustment(
    IncomeComputationResult& result,
    const string& code,
    const string& description,
    double claimedAmount,
    double allowedAmount,
    const string& explanation
) {
    IncomeAdjustment adjustment;

    adjustment.code = code;
    adjustment.description = description;
    adjustment.claimedAmount = claimedAmount;
    adjustment.allowedAmount = allowedAmount;
    adjustment.explanation = explanation;

    result.adjustments.push_back(adjustment);
}

} // namespace

// ============================================================
// Constructor
// ============================================================

HousePropertyIncome::HousePropertyIncome(
    const string& description
)
    : Income(description),
      propertyUse(PropertyUse::SelfOccupied),
      ownershipPercent(100.0),
      grossAnnualValue(0.0),
      municipalTaxesPaid(0.0),
      currentYearInterest(0.0),
      preConstructionInterestInstallment(0.0),
      selfOccupiedInterestCategory(
          SelfOccupiedInterestCategory::HigherLimitEligible
      ) {
}

// ============================================================
// Validation
// ============================================================

void HousePropertyIncome::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}

// ============================================================
// Setters
// ============================================================

void HousePropertyIncome::setPropertyUse(
    PropertyUse propertyUse
) {
    switch (propertyUse) {
        case PropertyUse::SelfOccupied:
        case PropertyUse::LetOut:
        case PropertyUse::DeemedLetOut:
            this->propertyUse = propertyUse;
            return;

        default:
            throw invalid_argument(
                "Invalid house-property use category."
            );
    }
}

void HousePropertyIncome::setOwnershipPercent(
    double percentage
) {
    if (
        !isfinite(percentage) ||
        percentage <= 0.0 ||
        percentage > 100.0
    ) {
        throw invalid_argument(
            "Ownership percentage must be greater than "
            "zero and not greater than 100."
        );
    }

    ownershipPercent = percentage;
}

void HousePropertyIncome::setGrossAnnualValue(
    double amount
) {
    validateAmount(amount, "Gross Annual Value");
    grossAnnualValue = amount;
}

void HousePropertyIncome::setMunicipalTaxesPaid(
    double amount
) {
    validateAmount(amount, "Municipal taxes paid");
    municipalTaxesPaid = amount;
}

void HousePropertyIncome::setCurrentYearInterest(
    double amount
) {
    validateAmount(amount, "Current-year housing-loan interest");
    currentYearInterest = amount;
}

void HousePropertyIncome::setPreConstructionInterestInstallment(
    double amount
) {
    validateAmount(
        amount,
        "Current-year pre-construction-interest instalment"
    );

    preConstructionInterestInstallment = amount;
}

void HousePropertyIncome::setSelfOccupiedInterestCategory(
    SelfOccupiedInterestCategory category
) {
    switch (category) {
        case SelfOccupiedInterestCategory::HigherLimitEligible:
        case SelfOccupiedInterestCategory::OtherEligibleLoan:
            selfOccupiedInterestCategory = category;
            return;

        default:
            throw invalid_argument(
                "Invalid self-occupied interest category."
            );
    }
}

// ============================================================
// Getters
// ============================================================

PropertyUse HousePropertyIncome::getPropertyUse() const {
    return propertyUse;
}

string HousePropertyIncome::getPropertyUseName() const {
    switch (propertyUse) {
        case PropertyUse::SelfOccupied:
            return "Self-Occupied";

        case PropertyUse::LetOut:
            return "Let-Out";

        case PropertyUse::DeemedLetOut:
            return "Deemed Let-Out";

        default:
            throw logic_error(
                "Unexpected house-property use category."
            );
    }
}

double HousePropertyIncome::getOwnershipPercent() const {
    return ownershipPercent;
}

double HousePropertyIncome::getGrossAnnualValue() const {
    return grossAnnualValue;
}

double HousePropertyIncome::getMunicipalTaxesPaid() const {
    return municipalTaxesPaid;
}

double HousePropertyIncome::getCurrentYearInterest() const {
    return currentYearInterest;
}

double HousePropertyIncome::
getPreConstructionInterestInstallment() const {
    return preConstructionInterestInstallment;
}

SelfOccupiedInterestCategory
HousePropertyIncome::getSelfOccupiedInterestCategory() const {
    return selfOccupiedInterestCategory;
}

double HousePropertyIncome::getTotalInterestEntered() const {
    const double total =
        currentYearInterest +
        preConstructionInterestInstallment;

    if (!isfinite(total)) {
        throw overflow_error(
            "Combined housing-loan interest is too large."
        );
    }

    return total;
}

double HousePropertyIncome::getOwnershipFraction() const {
    return ownershipPercent / 100.0;
}

double HousePropertyIncome::getOwnedInterestAmount() const {
    const double amount =
        getTotalInterestEntered() * getOwnershipFraction();

    if (!isfinite(amount)) {
        throw overflow_error(
            "Allocated housing-loan interest is too large."
        );
    }

    return amount;
}

// ============================================================
// Income interface
// ============================================================

IncomeHead HousePropertyIncome::getHead() const {
    return IncomeHead::HouseProperty;
}

string HousePropertyIncome::getHeadName() const {
    return "House Property";
}

IncomeComputationResult HousePropertyIncome::evaluate(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before evaluating house-property income."
        );
    }

    // Current implementation coverage matches the enabled
    // categories in our configuration files.
    if (
        taxpayer.getType() != TaxpayerType::Individual &&
        taxpayer.getType() != TaxpayerType::HUF
    ) {
        throw invalid_argument(
            "The current house-property module supports "
            "Individual and HUF taxpayers only."
        );
    }

    const string supportKey =
        taxpayer.getType() == TaxpayerType::Individual
            ? "SUPPORT_INDIVIDUAL"
            : "SUPPORT_HUF";

    if (!rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "The selected taxpayer category is not enabled "
            "in this regime configuration."
        );
    }

    IncomeComputationResult result;

    result.head = getHead();
    result.description = getDescription();

    const double ownershipFraction =
        getOwnershipFraction();

    const double ownedInterest =
        getOwnedInterestAmount();

    result.notes.push_back(
        "Property use: " + getPropertyUseName() + "."
    );

    addPropertyBreakdown(
        result,
        "Ownership percentage",
        ownershipPercent
    );

    addPropertyBreakdown(
        result,
        "Whole-property current-year interest entered",
        currentYearInterest
    );

    addPropertyBreakdown(
        result,
        "Whole-property eligible pre-construction instalment entered",
        preConstructionInterestInstallment
    );

    addPropertyBreakdown(
        result,
        "Taxpayer's share of eligible interest entered",
        ownedInterest
    );

    // --------------------------------------------------------
    // Self-occupied property
    // --------------------------------------------------------

    if (propertyUse == PropertyUse::SelfOccupied) {
        // Do not silently ignore rental figures if a property
        // has been selected as self-occupied.
        if (grossAnnualValue != 0.0) {
            throw invalid_argument(
                "Self-occupied property must have zero entered "
                "Gross Annual Value in this model."
            );
        }

        const DeductionRule& interestRule =
            rules.getDeductionRule("24B_SELF_OCCUPIED");

        if (
            interestRule.allowed &&
            !interestRule.hasMaximumAmount
        ) {
            throw logic_error(
                "Self-occupied interest requires a configured "
                "aggregate ceiling."
            );
        }

        result.grossAmount = 0.0;
        result.normalRateIncome = 0.0;

        addPropertyBreakdown(
            result,
            "Self-occupied annual value",
            0.0
        );

        if (municipalTaxesPaid > 0.0) {
            result.notes.push_back(
                "Municipal taxes entered for this self-occupied "
                "property are not deducted in this computation."
            );
        }

        if (!interestRule.allowed) {
            addPropertyAdjustment(
                result,
                "24B_SELF_OCCUPIED",
                "Self-occupied housing-loan interest",
                ownedInterest,
                0.0,
                "Not allowed under the selected regime."
            );

            addPropertyBreakdown(
                result,
                "Property income contribution",
                0.0
            );

            return result;
        }

        const string pendingCode =
            selfOccupiedInterestCategory ==
                SelfOccupiedInterestCategory::HigherLimitEligible
                    ? "24B_SELF_OCCUPIED_PENDING_HIGHER"
                    : "24B_SELF_OCCUPIED_PENDING_OTHER";

        addPropertyAdjustment(
            result,
            pendingCode,
            "Self-occupied interest awaiting aggregate limit",
            ownedInterest,
            0.0,
            "Pending taxpayer-level aggregation. The annual "
            "ceiling must not be applied once per property."
        );

        result.notes.push_back(
            "This property-level result is provisional: eligible "
            "self-occupied interest must be processed by the "
            "house-property aggregation module."
        );

        addPropertyBreakdown(
            result,
            "Contribution before aggregate interest adjustment",
            0.0
        );

        return result;
    }

    // --------------------------------------------------------
    // Let-out / deemed-let-out property
    // --------------------------------------------------------

    const double standardDeductionPercent =
        rules.getNumber(
            "HOUSE_PROPERTY_STANDARD_DEDUCTION_PERCENT"
        );

    if (
        standardDeductionPercent < 0.0 ||
        standardDeductionPercent > 100.0
    ) {
        throw invalid_argument(
            "House-property standard deduction percentage "
            "must be between zero and 100."
        );
    }

    const double ownedAnnualValue =
        grossAnnualValue * ownershipFraction;

    const double ownedMunicipalTaxes =
        municipalTaxesPaid * ownershipFraction;

    // Do not floor this intermediate value automatically.
    // The final property contribution may represent a loss.
    const double netAnnualValue =
        ownedAnnualValue - ownedMunicipalTaxes;

    const double standardDeduction =
        max(0.0, netAnnualValue) *
        standardDeductionPercent / 100.0;

    const DeductionRule& interestRule =
        rules.getDeductionRule("24B_LET_OUT");

    double allowedInterest = 0.0;

    if (interestRule.allowed) {
        allowedInterest = ownedInterest;

        if (interestRule.hasMaximumAmount) {
            allowedInterest = min(
                allowedInterest,
                interestRule.maximumAmount
            );
        }
    }

    const double propertyIncome =
        netAnnualValue -
        standardDeduction -
        allowedInterest;

    if (
        !isfinite(ownedAnnualValue) ||
        !isfinite(ownedMunicipalTaxes) ||
        !isfinite(netAnnualValue) ||
        !isfinite(standardDeduction) ||
        !isfinite(propertyIncome)
    ) {
        throw overflow_error(
            "House-property computation exceeded numeric limits."
        );
    }

    result.grossAmount = ownedAnnualValue;
    result.normalRateIncome = propertyIncome;

    addPropertyBreakdown(
        result,
        "Taxpayer's share of Gross Annual Value",
        ownedAnnualValue
    );

    addPropertyBreakdown(
        result,
        "Less: Eligible municipal taxes paid",
        -ownedMunicipalTaxes
    );

    addPropertyBreakdown(
        result,
        "Net Annual Value",
        netAnnualValue
    );

    addPropertyAdjustment(
        result,
        "HOUSE_PROPERTY_STANDARD",
        "House-property standard deduction",
        standardDeduction,
        standardDeduction,
        "Calculated from positive Net Annual Value."
    );

    addPropertyBreakdown(
        result,
        "Less: House-property standard deduction",
        -standardDeduction
    );

    addPropertyAdjustment(
        result,
        "24B_LET_OUT",
        "Let-out housing-loan interest",
        ownedInterest,
        allowedInterest,
        interestRule.allowed
            ? "Allowed at property level under configured rules."
            : "Not allowed under the selected regime."
    );

    addPropertyBreakdown(
        result,
        "Less: Eligible housing-loan interest",
        -allowedInterest
    );

    addPropertyBreakdown(
        result,
        "Property income / loss before head aggregation",
        propertyIncome
    );

    result.notes.push_back(
        "Gross Annual Value and eligible interest are based "
        "on the user's declared figures."
    );

    result.notes.push_back(
        "The ownership share has already been applied. "
        "Do not apply it again during aggregation."
    );

    if (propertyIncome < 0.0) {
        result.notes.push_back(
            "A property-level loss is not automatically an allowed "
            "set-off against salary or another income head. "
            "Regime-specific aggregation is still required."
        );
    }

    return result;
}