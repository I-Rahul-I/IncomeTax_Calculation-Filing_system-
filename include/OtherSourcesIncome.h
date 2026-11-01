#ifndef OTHER_SOURCES_INCOME_H
#define OTHER_SOURCES_INCOME_H

#include <string>
#include <vector>

#include "Income.h"

using namespace std;

// Supported ordinary income categories.
//
// The input module must avoid entering the same receipt
// under multiple categories or income heads.
enum class OtherSourceType {
    SavingsAccountInterest = 1,
    FixedDepositInterest,
    RecurringDepositInterest,
    OtherDepositInterest,
    BondInterest,
    Dividend,
    FamilyPension,
    OtherOrdinaryIncome
};

// One ordinary other-source receipt.
//
// Amounts are gross taxable receipts before:
// - the supported family-pension deduction
// - Chapter VI-A deductions
// - TDS/TCS adjustment
//
// TDS must be entered separately in the payment module.
struct OtherSourceEntry {
    OtherSourceType type =
        OtherSourceType::OtherOrdinaryIncome;

    string description;

    double amount = 0.0;
};

// Categories requiring separate special-income treatment.
//
// Online-game and VDA entries use separately determined
// taxable amounts, not gross account withdrawals or sale value.
enum class SpecialOtherSourceType {
    LotteryAndSimilarWinnings = 1,
    OnlineGameNetWinnings,
    VirtualDigitalAssetIncome
};

// A special-income entry.
//
// For this initial model, taxableAmount is supplied by the user
// after the relevant underlying computation.
//
// The application does not infer online-game net winnings
// or VDA income from bank credits alone.
struct SpecialOtherSourceEntry {
    SpecialOtherSourceType type =
        SpecialOtherSourceType::LotteryAndSimilarWinnings;

    string description;

    double taxableAmount = 0.0;

    // Optional date in YYYY-MM-DD format.
    // A later calculation may require it for timing rules.
    string relevantDate;
};

// Combined annual Other Sources input.
//
// Normal receipts and special-income components are kept
// separate so the main calculator does not apply ordinary
// slab rates to every amount indiscriminately.
class OtherSourcesIncome : public Income {
private:
    vector<OtherSourceEntry> ordinaryEntries;

    vector<SpecialOtherSourceEntry> specialEntries;

    static void validateAmount(
        double amount,
        const string& fieldName
    );

    static void validateOrdinaryEntry(
        const OtherSourceEntry& entry
    );

    static void validateSpecialEntry(
        const SpecialOtherSourceEntry& entry
    );

public:
    explicit OtherSourcesIncome(
        const string& description =
            "Annual Income from Other Sources"
    );

    // ========================================================
    // Ordinary income entries
    // ========================================================

    void addOrdinaryEntry(
        const OtherSourceEntry& entry
    );

    void clearOrdinaryEntries();

    const vector<OtherSourceEntry>&
    getOrdinaryEntries() const;

    double getOrdinaryAmountByType(
        OtherSourceType type
    ) const;

    double getTotalOrdinaryIncomeEntered() const;

    // ========================================================
    // Interest totals
    // ========================================================

    double getSavingsAccountInterest() const;

    double getFixedDepositInterest() const;

    double getRecurringDepositInterest() const;

    double getOtherDepositInterest() const;

    double getBondInterest() const;

    // Reporting total only.
    //
    // This must not be assumed to equal the eligible amount
    // for every interest-related deduction.
    double getTotalInterestEntered() const;

    // ========================================================
    // Other ordinary income totals
    // ========================================================

    double getDividendIncome() const;

    double getFamilyPensionIncome() const;

    double getOtherOrdinaryIncome() const;

    // ========================================================
    // Special-income entries
    // ========================================================

    void addSpecialEntry(
        const SpecialOtherSourceEntry& entry
    );

    void clearSpecialEntries();

    const vector<SpecialOtherSourceEntry>&
    getSpecialEntries() const;

    double getTotalSpecialIncomeEntered() const;

    // Maps the selected category to the corresponding
    // configuration rule code.
    static string getSpecialRuleCode(
        SpecialOtherSourceType type
    );

    // ========================================================
    // Display helpers
    // ========================================================

    static string getOrdinaryTypeName(
        OtherSourceType type
    );

    static string getSpecialTypeName(
        SpecialOtherSourceType type
    );

    // ========================================================
    // Income interface
    // ========================================================

    IncomeHead getHead() const override;

    string getHeadName() const override;

    // Applies the supported family-pension adjustment once
    // to the annual combined family-pension amount.
    //
    // Interest deductions under Chapter VI-A are not applied
    // here; they belong in the later deduction module.
    //
    // Special-rate amounts are returned as separate components.
    IncomeComputationResult evaluate(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const override;
};

#endif