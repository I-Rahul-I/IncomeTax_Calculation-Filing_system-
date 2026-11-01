#ifndef BUSINESS_INCOME_H
#define BUSINESS_INCOME_H

#include <string>
#include <vector>

#include "Income.h"

using namespace std;

// Identifies whether the entry relates to business or profession.
enum class BusinessActivityType {
    Business = 1,
    Profession
};

enum class BusinessComputationMethod {
    Normal = 1,
    Section44AD,
    Section44ADA
};

// One expense included in the accounting expenses entered.
//
// amount:
// The total expense included in the accounts.
//
// disallowedAmount:
// The portion of that expense not deductible in this computation.
//
// The disallowed amount is already part of amount.
// It must not be treated as an additional expense.
struct BusinessExpense {
    string description;

    double amount = 0.0;
    double disallowedAmount = 0.0;

    string explanation;
};

// Normal-method computation for one business/professional activity.
//
// Accounting profit is derived from the entered figures:
// receipts + other income - expenses - book depreciation.
//
// Book depreciation is then added back and eligible tax
// depreciation is deducted.
//
// No previous-year records or asset database are maintained.
class BusinessIncome : public Income {
private:
    BusinessActivityType activityType;

    string activityName;
    string natureOfActivity;

    double grossReceipts;
    double otherBusinessIncome;

    vector<BusinessExpense> expenses;

    // Book depreciation must not also be included in expenses.
    double bookDepreciation;

    // User-declared eligible tax depreciation.
    // The module does not calculate depreciation from asset blocks.
    double taxDepreciation;

    BusinessComputationMethod computationMethod;
    double cashReceipts;
    double claimedPresumptiveProfit;

    static void validateAmount(
        double amount,
        const string& fieldName
    );

    static void validateExpense(
        const BusinessExpense& expense
    );

public:
    explicit BusinessIncome(
        const string& description =
            "Business or Profession Income"
    );

    // ========================================================
    // Activity details
    // ========================================================

    void setActivityType(BusinessActivityType type);

    void setActivityName(const string& name);

    void setNatureOfActivity(const string& nature);

    BusinessActivityType getActivityType() const;

    string getActivityTypeName() const;

    const string& getActivityName() const;

    const string& getNatureOfActivity() const;

    // ========================================================
    // Receipts and depreciation
    // ========================================================

    void setGrossReceipts(double amount);

    void setOtherBusinessIncome(double amount);

    void setBookDepreciation(double amount);

    void setTaxDepreciation(double amount);

    void setComputationMethod(BusinessComputationMethod method);
    void setCashReceipts(double amount);
    void setClaimedPresumptiveProfit(double amount);

    double getGrossReceipts() const;

    double getOtherBusinessIncome() const;

    double getBookDepreciation() const;

    double getTaxDepreciation() const;

    BusinessComputationMethod getComputationMethod() const;
    double getCashReceipts() const;
    double getClaimedPresumptiveProfit() const;

    // ========================================================
    // Expense entries
    // ========================================================

    // The input module can add several named expenses.
    //
    // For a simplified total-only input, add one entry such as:
    // description = "Combined business expenses".
    void addExpense(const BusinessExpense& expense);

    void clearExpenses();

    const vector<BusinessExpense>& getExpenses() const;

    double getTotalExpenses() const;

    double getTotalDisallowedExpenses() const;

    double getTotalAllowedExpenses() const;

    // ========================================================
    // Computation helpers
    // ========================================================

    double getTotalReceipts() const;

    // Total receipts minus entered expenses and book depreciation.
    double getAccountingProfit() const;

    // Accounting profit plus disallowed expenses and
    // book-depreciation add-back, before tax depreciation.
    double getProfitBeforeTaxDepreciation() const;

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