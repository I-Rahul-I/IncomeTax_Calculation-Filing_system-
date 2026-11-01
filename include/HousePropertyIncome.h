#ifndef HOUSE_PROPERTY_INCOME_H
#define HOUSE_PROPERTY_INCOME_H

#include <string>

#include "Income.h"

using namespace std;

// Property classification for the supported computation.
enum class PropertyUse {
    SelfOccupied = 1,
    LetOut,
    DeemedLetOut
};

// Declaration describing the applicable self-occupied
// housing-loan interest ceiling.
//
// The user must confirm the relevant eligibility conditions.
// The program does not independently verify loan documents.
enum class SelfOccupiedInterestCategory {
    HigherLimitEligible = 1,
    OtherEligibleLoan
};

// Current-year details for one house property.
//
// Monetary values entered here represent the WHOLE property.
// ownershipPercent is applied once during evaluation.
//
// No property ID, database record, or asset history is required.
class HousePropertyIncome : public Income {
private:
    PropertyUse propertyUse;

    double ownershipPercent;

    // Annual value before deducting municipal taxes.
    //
    // For let-out/deemed-let-out property, this must be an
    // appropriately determined Gross Annual Value, not blindly
    // assumed to equal actual rent in every situation.
    double grossAnnualValue;

    // Eligible municipal taxes actually paid by the owner
    // during the relevant financial year.
    double municipalTaxesPaid;

    // Eligible current-year interest before applying any
    // self-occupied-property annual ceiling.
    double currentYearInterest;

    // Only the eligible pre-construction-interest instalment
    // claimable in this year, not the entire historic interest.
    double preConstructionInterestInstallment;

    SelfOccupiedInterestCategory selfOccupiedInterestCategory;

    static void validateAmount(
        double amount,
        const string& fieldName
    );

public:
    explicit HousePropertyIncome(
        const string& description = "House Property Income"
    );

    // ========================================================
    // Setters
    // ========================================================

    void setPropertyUse(PropertyUse propertyUse);

    // Must be greater than 0 and not greater than 100.
    void setOwnershipPercent(double percentage);

    void setGrossAnnualValue(double amount);

    void setMunicipalTaxesPaid(double amount);

    void setCurrentYearInterest(double amount);

    void setPreConstructionInterestInstallment(double amount);

    void setSelfOccupiedInterestCategory(
        SelfOccupiedInterestCategory category
    );

    // ========================================================
    // Getters
    // ========================================================

    PropertyUse getPropertyUse() const;

    string getPropertyUseName() const;

    double getOwnershipPercent() const;

    double getGrossAnnualValue() const;

    double getMunicipalTaxesPaid() const;

    double getCurrentYearInterest() const;

    double getPreConstructionInterestInstallment() const;

    SelfOccupiedInterestCategory
    getSelfOccupiedInterestCategory() const;

    // Combined eligible interest entered for this property,
    // before ownership allocation and aggregate ceilings.
    double getTotalInterestEntered() const;

    // Ownership share represented as a multiplier.
    // Example: 50 percent returns 0.5.
    double getOwnershipFraction() const;

    // Taxpayer's share of the entered eligible interest,
    // before any aggregate self-occupied-property ceiling.
    double getOwnedInterestAmount() const;

    // ========================================================
    // Income interface
    // ========================================================

    IncomeHead getHead() const override;

    string getHeadName() const override;

    // Compute this property's contribution under one regime.
    //
    // For self-occupied properties, the implementation will
    // preserve the eligible interest claim in the result
    // breakdown for taxpayer-level processing.
    //
    // The aggregate self-occupied interest ceiling must be
    // enforced once across the applicable properties by the
    // later aggregation module.
    //
    // A property loss is not automatically an allowed set-off
    // against salary or other income.
    IncomeComputationResult evaluate(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const override;
};

#endif