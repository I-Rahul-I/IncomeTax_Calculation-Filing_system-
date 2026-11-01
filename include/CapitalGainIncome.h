#ifndef CAPITAL_GAIN_INCOME_H
#define CAPITAL_GAIN_INCOME_H

#include <string>

#include "Income.h"

using namespace std;

// Asset classification for input and report purposes.
//
// This selection alone does not determine tax treatment.
enum class CapitalAssetType {
    ListedEquityShare = 1,
    EquityOrientedMutualFund,
    BusinessTrustUnit,
    LandOrBuilding,
    Gold,
    DebtMutualFund,
    UnlistedShare,
    Other
};

// Holding-period classification.
//
// The input/calculation module must validate this against
// the implemented rules for the selected asset.
enum class CapitalGainTerm {
    ShortTerm = 1,
    LongTerm
};

// Supported computation routes.
//
// Section112 requires additional asset-specific checks;
// choosing this route must not bypass those checks.
enum class CapitalGainTaxTreatment {
    NormalRate = 1,
    Section111A,
    Section112,
    Section112A
};

// One current-session capital-asset transfer.
//
// Amounts represent the taxpayer's own share of the transaction.
// Do not enter whole-property figures and then apply ownership
// allocation again in the aggregation module.
//
// The initial model calculates:
// sale consideration
// - transfer expenses
// - acquisition cost
// - improvement cost
// - eligible exemption claimed
//
// Asset-specific overrides, indexation, grandfathering,
// Section 50C stamp-duty substitution is modeled for land/building.
// Other asset-specific overrides, indexation, grandfathering and
// special provisions are not automatically established.
class CapitalGainIncome : public Income {
private:
    CapitalAssetType assetType;
    CapitalGainTerm gainTerm;
    CapitalGainTaxTreatment taxTreatment;

    string assetDescription;

    // Dates use YYYY-MM-DD.
    string acquisitionDate;
    string transferDate;

    double saleConsideration;
    // Stamp-duty value for land/building; 0 means not supplied.
    double stampDutyValue;
    double grandfatheredFMV;
    double acquisitionCost;
    double improvementCost;
    double transferExpenses;

    // Enter only an independently determined eligible exemption.
    // The module will not infer eligibility from a section name.
    double exemptionClaimed;
    string exemptionSection;

    bool sttPaidOnAcquisition;
    bool sttPaidOnTransfer;

    static void validateAmount(
        double amount,
        const string& fieldName
    );

    static void validateDate(
        const string& date,
        const string& fieldName
    );

    // Validate consistency of the selected term/tax route
    // and the transaction details supported by this module.
    void validateTransaction() const;

public:
    explicit CapitalGainIncome(
        const string& description = "Capital Gain Transaction"
    );

    // ========================================================
    // Asset and classification setters
    // ========================================================

    void setAssetType(CapitalAssetType type);

    void setGainTerm(CapitalGainTerm term);

    void setTaxTreatment(
        CapitalGainTaxTreatment treatment
    );

    void setAssetDescription(
        const string& description
    );

    void setAcquisitionDate(const string& date);

    void setTransferDate(const string& date);

    // ========================================================
    // Monetary setters
    // ========================================================

    void setSaleConsideration(double amount);

    void setStampDutyValue(double amount);

    // Fair market value on 31 January 2018 for eligible Section 112A assets.
    void setGrandfatheredFMV(double amount);

    void setAcquisitionCost(double amount);

    void setImprovementCost(double amount);

    void setTransferExpenses(double amount);

    // Store the amount and supporting section together.
    // A positive exemption requires a non-empty section.
    void setExemptionClaim(
        double amount,
        const string& section
    );

    // ========================================================
    // STT declarations
    // ========================================================

    void setSTTPaidOnAcquisition(bool paid);

    void setSTTPaidOnTransfer(bool paid);

    // ========================================================
    // Getters
    // ========================================================

    CapitalAssetType getAssetType() const;

    CapitalGainTerm getGainTerm() const;

    CapitalGainTaxTreatment getTaxTreatment() const;

    string getAssetTypeName() const;

    string getGainTermName() const;

    string getTaxTreatmentName() const;

    // Returns the special-rule code requested by the route:
    // "111A", "112", or "112A".
    // Returns an empty string for NormalRate.
    string getSpecialRuleCode() const;

    const string& getAssetDescription() const;

    const string& getAcquisitionDate() const;

    const string& getTransferDate() const;

    double getSaleConsideration() const;

    double getStampDutyValue() const;

    double getGrandfatheredFMV() const;

    // Section 112A grandfathered acquisition-cost basis when applicable.
    double getEffectiveAcquisitionCost() const;

    // Section 50C simplified effective consideration for land/building.
    double getEffectiveSaleConsideration() const;

    double getAcquisitionCost() const;

    double getImprovementCost() const;

    double getTransferExpenses() const;

    double getExemptionClaimed() const;

    const string& getExemptionSection() const;

    bool isSTTPaidOnAcquisition() const;

    bool isSTTPaidOnTransfer() const;

    // ========================================================
    // Computation helpers
    // ========================================================

    double getTotalEnteredCosts() const;

    // May be negative, indicating a transaction-level loss.
    double getGainBeforeExemption() const;

    // Exemption cannot create or enlarge a capital loss.
    double getGainAfterExemption() const;

    // ========================================================
    // Income interface
    // ========================================================

    IncomeHead getHead() const override;

    string getHeadName() const override;

    IncomeComputationResult evaluate(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const override;
};

#endif