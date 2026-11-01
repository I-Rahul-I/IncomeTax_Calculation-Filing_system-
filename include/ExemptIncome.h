#ifndef EXEMPT_INCOME_H
#define EXEMPT_INCOME_H

#include <string>
#include <vector>

#include "Income.h"

using namespace std;

// Reporting categories.
//
// Selecting a category records the user's declaration;
// it does not independently establish legal eligibility.
enum class ExemptIncomeType {
    AgriculturalIncome = 1,
    Scholarship,
    PPFInterest,
    EligibleInsuranceProceeds,
    OtherDeclaredExemptIncome
};

// One amount declared exempt.
//
// amount must represent only the eligible exempt portion,
// not an entire receipt that may contain a taxable component.
struct ExemptIncomeEntry {
    ExemptIncomeType type =
        ExemptIncomeType::OtherDeclaredExemptIncome;

    string description;

    double amount = 0.0;

    // Optional supporting section/rule reference.
    // This is explanatory text, not an executable rule.
    string exemptionReference;

    // Optional qualifications for the report.
    string remarks;
};

// Combined annual exempt-income information.
//
// No previous-year data, permanent taxpayer record,
// or document upload is required.
class ExemptIncome : public Income {
private:
    vector<ExemptIncomeEntry> entries;

    static void validateAmount(
        double amount,
        const string& fieldName
    );

    static void validateEntry(
        const ExemptIncomeEntry& entry
    );

public:
    explicit ExemptIncome(
        const string& description =
            "Annual Exempt Income"
    );

    // ========================================================
    // Entry management
    // ========================================================

    void addEntry(
        const ExemptIncomeEntry& entry
    );

    void clearEntries();

    const vector<ExemptIncomeEntry>& getEntries() const;

    // ========================================================
    // Totals
    // ========================================================

    double getAmountByType(
        ExemptIncomeType type
    ) const;

    double getTotalExemptIncome() const;

    // Agricultural income must remain separately available
    // for any supported rate-purpose calculation.
    double getAgriculturalIncome() const;

    double getNonAgriculturalExemptIncome() const;

    // ========================================================
    // Display helper
    // ========================================================

    static string getExemptTypeName(
        ExemptIncomeType type
    );

    // ========================================================
    // Income interface
    // ========================================================

    IncomeHead getHead() const override;

    string getHeadName() const override;

    // Returns exempt amounts and report breakdown.
    //
    // normalRateIncome remains zero.
    // No special-rate taxable components are created.
    //
    // This result is informational and must not be added
    // to GTI as taxable income.
    //
    // The aggregation/calculation module must separately
    // inspect agricultural income before finalizing tax.
    IncomeComputationResult evaluate(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const override;
};

#endif