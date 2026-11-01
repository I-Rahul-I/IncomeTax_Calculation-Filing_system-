#include "BusinessIncome.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

string trimBusinessText(const string& value) {
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

void checkBusinessTotal(
    double amount,
    const string& description
) {
    if (!isfinite(amount)) {
        throw overflow_error(
            description + " exceeded numeric limits."
        );
    }
}

void addBusinessBreakdown(
    IncomeComputationResult& result,
    const string& description,
    double amount
) {
    IncomeBreakdownItem item;

    item.description = description;
    item.amount = amount;

    result.breakdown.push_back(item);
}

void addBusinessAdjustment(
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

BusinessIncome::BusinessIncome(const string& description)
    : Income(description),
      activityType(BusinessActivityType::Business),
      activityName(""),
      natureOfActivity(""),
      grossReceipts(0.0),
      otherBusinessIncome(0.0),
      bookDepreciation(0.0),
      taxDepreciation(0.0),
      computationMethod(BusinessComputationMethod::Normal),
      cashReceipts(0.0),
      claimedPresumptiveProfit(0.0) {
}

// ============================================================
// Validation
// ============================================================

void BusinessIncome::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}

void BusinessIncome::validateExpense(
    const BusinessExpense& expense
) {
    if (trimBusinessText(expense.description).empty()) {
        throw invalid_argument(
            "Business expense description cannot be empty."
        );
    }

    validateAmount(
        expense.amount,
        "Business expense amount"
    );

    validateAmount(
        expense.disallowedAmount,
        "Disallowed expense amount"
    );

    if (expense.disallowedAmount > expense.amount) {
        throw invalid_argument(
            "The disallowed portion cannot exceed "
            "the corresponding expense amount."
        );
    }
}

// ============================================================
// Activity details
// ============================================================

void BusinessIncome::setActivityType(
    BusinessActivityType type
) {
    switch (type) {
        case BusinessActivityType::Business:
        case BusinessActivityType::Profession:
            activityType = type;
            return;

        default:
            throw invalid_argument(
                "Invalid business/profession activity type."
            );
    }
}

void BusinessIncome::setActivityName(
    const string& name
) {
    const string cleanedName = trimBusinessText(name);

    if (cleanedName.empty()) {
        throw invalid_argument(
            "Business/profession name cannot be empty."
        );
    }

    activityName = cleanedName;
}

void BusinessIncome::setNatureOfActivity(
    const string& nature
) {
    const string cleanedNature = trimBusinessText(nature);

    if (cleanedNature.empty()) {
        throw invalid_argument(
            "Nature of business/profession cannot be empty."
        );
    }

    natureOfActivity = cleanedNature;
}

BusinessActivityType BusinessIncome::getActivityType() const {
    return activityType;
}

string BusinessIncome::getActivityTypeName() const {
    switch (activityType) {
        case BusinessActivityType::Business:
            return "Business";

        case BusinessActivityType::Profession:
            return "Profession";

        default:
            throw logic_error(
                "Unexpected business activity type."
            );
    }
}

const string& BusinessIncome::getActivityName() const {
    return activityName;
}

const string& BusinessIncome::getNatureOfActivity() const {
    return natureOfActivity;
}

// ============================================================
// Receipts and depreciation
// ============================================================

void BusinessIncome::setGrossReceipts(double amount) {
    validateAmount(amount, "Gross receipts");
    grossReceipts = amount;
}

void BusinessIncome::setOtherBusinessIncome(double amount) {
    validateAmount(amount, "Other business income");
    otherBusinessIncome = amount;
}

void BusinessIncome::setBookDepreciation(double amount) {
    validateAmount(amount, "Book depreciation");
    bookDepreciation = amount;
}

void BusinessIncome::setTaxDepreciation(double amount) {
    validateAmount(amount, "Eligible tax depreciation");
    taxDepreciation = amount;
}

void BusinessIncome::setComputationMethod(BusinessComputationMethod method) {
    switch (method) {
        case BusinessComputationMethod::Normal:
        case BusinessComputationMethod::Section44AD:
        case BusinessComputationMethod::Section44ADA:
            computationMethod = method;
            return;
        default:
            throw invalid_argument("Invalid business computation method.");
    }
}

void BusinessIncome::setCashReceipts(double amount) {
    validateAmount(amount, "Cash receipts");
    cashReceipts = amount;
}

void BusinessIncome::setClaimedPresumptiveProfit(double amount) {
    validateAmount(amount, "Claimed presumptive profit");
    claimedPresumptiveProfit = amount;
}

double BusinessIncome::getGrossReceipts() const {
    return grossReceipts;
}

double BusinessIncome::getOtherBusinessIncome() const {
    return otherBusinessIncome;
}

double BusinessIncome::getBookDepreciation() const {
    return bookDepreciation;
}

double BusinessIncome::getTaxDepreciation() const {
    return taxDepreciation;
}

BusinessComputationMethod BusinessIncome::getComputationMethod() const {
    return computationMethod;
}

double BusinessIncome::getCashReceipts() const {
    return cashReceipts;
}

double BusinessIncome::getClaimedPresumptiveProfit() const {
    return claimedPresumptiveProfit;
}

// ============================================================
// Expense entries
// ============================================================

void BusinessIncome::addExpense(
    const BusinessExpense& expense
) {
    validateExpense(expense);

    BusinessExpense cleanedExpense = expense;

    cleanedExpense.description =
        trimBusinessText(expense.description);

    expenses.push_back(cleanedExpense);
}

void BusinessIncome::clearExpenses() {
    expenses.clear();
}

const vector<BusinessExpense>&
BusinessIncome::getExpenses() const {
    return expenses;
}

double BusinessIncome::getTotalExpenses() const {
    double total = 0.0;

    for (const BusinessExpense& expense : expenses) {
        total += expense.amount;
        checkBusinessTotal(total, "Total business expenses");
    }

    return total;
}

double BusinessIncome::getTotalDisallowedExpenses() const {
    double total = 0.0;

    for (const BusinessExpense& expense : expenses) {
        total += expense.disallowedAmount;

        checkBusinessTotal(
            total,
            "Total disallowed business expenses"
        );
    }

    return total;
}

double BusinessIncome::getTotalAllowedExpenses() const {
    double total = 0.0;

    for (const BusinessExpense& expense : expenses) {
        total += expense.amount - expense.disallowedAmount;

        checkBusinessTotal(
            total,
            "Total allowed business expenses"
        );
    }

    return total;
}

// ============================================================
// Computation helpers
// ============================================================

double BusinessIncome::getTotalReceipts() const {
    const double total =
        grossReceipts + otherBusinessIncome;

    checkBusinessTotal(total, "Total business receipts");

    return total;
}

double BusinessIncome::getAccountingProfit() const {
    const double profit =
        getTotalReceipts() -
        getTotalExpenses() -
        bookDepreciation;

    checkBusinessTotal(profit, "Accounting profit");

    return profit;
}

double BusinessIncome::
getProfitBeforeTaxDepreciation() const {
    // Algebraically equivalent to:
    // accounting profit
    // + disallowed expenses
    // + book depreciation.
    //
    // Calculate directly to avoid unnecessary cancellation.
    const double profit =
        getTotalReceipts() - getTotalAllowedExpenses();

    checkBusinessTotal(
        profit,
        "Profit before tax depreciation"
    );

    return profit;
}

// ============================================================
// Income interface
// ============================================================

IncomeHead BusinessIncome::getHead() const {
    return IncomeHead::BusinessProfession;
}

string BusinessIncome::getHeadName() const {
    return "Business or Profession";
}

IncomeComputationResult BusinessIncome::evaluate(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before evaluating business income."
        );
    }

    if (activityName.empty() || natureOfActivity.empty()) {
        throw invalid_argument(
            "Enter the activity name and nature before "
            "evaluating business income."
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
                "The current business-income module supports "
                "Individual and HUF taxpayers only."
            );
    }

    if (!rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "The selected taxpayer category is not enabled "
            "in this regime configuration."
        );
    }

    const double totalReceipts = getTotalReceipts();
    const double totalExpenses = getTotalExpenses();

    const double totalDisallowed =
        getTotalDisallowedExpenses();

    const double accountingProfit =
        getAccountingProfit();

    const double profitBeforeDepreciation =
        getProfitBeforeTaxDepreciation();

    double computedIncome = profitBeforeDepreciation - taxDepreciation;
    string methodNote = "normal accounting method";

    if (computationMethod == BusinessComputationMethod::Section44AD ||
        computationMethod == BusinessComputationMethod::Section44ADA) {
        if (taxpayer.getType() != TaxpayerType::Individual &&
            taxpayer.getType() != TaxpayerType::HUF) {
            throw invalid_argument("The selected presumptive route is not available for this taxpayer type.");
        }
        if (taxpayer.getResidentialStatus() != ResidentialStatus::Resident) {
            throw invalid_argument("Sections 44AD/44ADA require a resident taxpayer.");
        }
        if (cashReceipts > totalReceipts) {
            throw invalid_argument("Cash receipts cannot exceed gross receipts.");
        }
        const double cashRatio = totalReceipts > 0.0 ? cashReceipts / totalReceipts : 0.0;
        const bool mostlyDigital = cashRatio <= 0.05;

        if (computationMethod == BusinessComputationMethod::Section44AD) {
            if (activityType != BusinessActivityType::Business) {
                throw invalid_argument("Section 44AD is for eligible business, not a profession.");
            }
            const double receiptLimit = mostlyDigital ? 30000000.0 : 20000000.0;
            if (grossReceipts > receiptLimit) {
                throw invalid_argument("Gross receipts exceed the Section 44AD limit for the entered cash ratio.");
            }
            const double digitalReceipts = totalReceipts - cashReceipts;
            const double statutoryFloor = digitalReceipts * 0.06 + cashReceipts * 0.08;
            computedIncome = max(statutoryFloor, claimedPresumptiveProfit);
            methodNote = "Section 44AD presumptive business computation";
        } else {
            if (activityType != BusinessActivityType::Profession ||
                taxpayer.getType() != TaxpayerType::Individual) {
                throw invalid_argument("Section 44ADA is available only to a resident Individual carrying on an eligible specified profession.");
            }
            const double receiptLimit = mostlyDigital ? 7500000.0 : 5000000.0;
            if (grossReceipts > receiptLimit) {
                throw invalid_argument("Gross receipts exceed the Section 44ADA limit for the entered cash ratio.");
            }
            computedIncome = max(totalReceipts * 0.50, claimedPresumptiveProfit);
            methodNote = "Section 44ADA presumptive profession computation";
        }
    }

    checkBusinessTotal(computedIncome, "Business/profession income");

    IncomeComputationResult result;

    result.head = getHead();
    result.description = getDescription();
    result.grossAmount = totalReceipts;
    result.normalRateIncome = computedIncome;

    if (computationMethod != BusinessComputationMethod::Normal) {
        addBusinessBreakdown(result, "Gross receipts", totalReceipts);
        addBusinessBreakdown(result, "Cash receipts", cashReceipts);
        addBusinessBreakdown(result, "Claimed presumptive profit (higher amount)",
                             claimedPresumptiveProfit);
        addBusinessBreakdown(result, "Taxable presumptive business/profession income",
                             computedIncome);
        result.notes.push_back("Activity: " + activityName + ".");
        result.notes.push_back("Nature of activity: " + natureOfActivity + ".");
        result.notes.push_back("Computation method: " + methodNote + ".");
        result.notes.push_back(
            "Presumptive income is computed from gross receipts and cash/non-cash "
            "mix. Business expenses and depreciation are deemed covered and "
            "are not deducted separately. Verify eligibility and all statutory conditions."
        );
        IncomeAdjustment adjustment;
        adjustment.code = computationMethod == BusinessComputationMethod::Section44AD
            ? "44AD" : "44ADA";
        adjustment.description = methodNote;
        adjustment.claimedAmount = claimedPresumptiveProfit;
        adjustment.allowedAmount = computedIncome;
        adjustment.explanation = "Presumptive statutory floor or higher declared profit applied.";
        result.adjustments.push_back(adjustment);
        return result;
    }

    // --------------------------------------------------------
    // Report details
    // --------------------------------------------------------

    addBusinessBreakdown(
        result,
        "Gross receipts / turnover",
        grossReceipts
    );

    addBusinessBreakdown(
        result,
        "Other business income",
        otherBusinessIncome
    );

    addBusinessBreakdown(
        result,
        "Total business receipts",
        totalReceipts
    );

    for (const BusinessExpense& expense : expenses) {
        addBusinessBreakdown(
            result,
            "Expense: " + expense.description,
            -expense.amount
        );
    }

    addBusinessBreakdown(
        result,
        "Total accounting expenses, excluding book depreciation",
        totalExpenses
    );

    addBusinessBreakdown(
        result,
        "Less: Book depreciation",
        -bookDepreciation
    );

    addBusinessBreakdown(
        result,
        "Accounting profit / loss",
        accountingProfit
    );

    addBusinessBreakdown(
        result,
        "Add back: Disallowed expense portions",
        totalDisallowed
    );

    addBusinessBreakdown(
        result,
        "Add back: Book depreciation",
        bookDepreciation
    );

    addBusinessBreakdown(
        result,
        "Profit / loss before eligible tax depreciation",
        profitBeforeDepreciation
    );

    addBusinessBreakdown(
        result,
        "Less: Eligible tax depreciation",
        -taxDepreciation
    );

    addBusinessBreakdown(
        result,
        "Business/profession income before head aggregation",
        computedIncome
    );

    // --------------------------------------------------------
    // Expense adjustments
    // --------------------------------------------------------

    for (const BusinessExpense& expense : expenses) {
        const string explanation =
            expense.explanation.empty()
                ? "Based on user-declared allowable and "
                  "disallowable expense portions."
                : expense.explanation;

        addBusinessAdjustment(
            result,
            "BUSINESS_EXPENSE",
            expense.description,
            expense.amount,
            expense.amount - expense.disallowedAmount,
            explanation
        );
    }

    addBusinessAdjustment(
        result,
        "BUSINESS_TAX_DEPRECIATION",
        "Eligible tax depreciation",
        taxDepreciation,
        taxDepreciation,
        "User-declared eligible ordinary tax depreciation. "
        "Book depreciation is separately added back."
    );

    // --------------------------------------------------------
    // Scope and aggregation notes
    // --------------------------------------------------------

    result.notes.push_back(
        "Activity: " + activityName + "."
    );

    result.notes.push_back(
        "Nature of activity: " + natureOfActivity + "."
    );

    result.notes.push_back(
        "Computation method: " + methodNote + "."
    );

    if (computationMethod != BusinessComputationMethod::Normal) {
        result.notes.push_back(
            "Presumptive income is computed from gross receipts and cash/non-cash "
            "mix. Expenses and depreciation are deemed covered and are not "
            "deducted separately under this route. Verify eligible activity, "
            "cash receipt classification, and section-specific conditions."
        );
    }

    result.notes.push_back(
        "Entered expenses and tax depreciation are assumed "
        "eligible under both compared regimes. Regime-specific "
        "incentive deductions and additional depreciation "
        "are not supported by this module."
    );

    result.notes.push_back(
        "Book depreciation must not also be entered as an "
        "expense. Other business receipts must not be entered "
        "again under another income head."
    );

    result.notes.push_back(
        "Expense and depreciation adjustments above are already "
        "included in computed business income. Do not deduct "
        "them again after calculating GTI."
    );

    if (computedIncome < 0.0) {
        result.notes.push_back(
            "This entry produces a business/profession loss. "
            "The aggregation module must apply permitted "
            "loss-set-off restrictions before calculating GTI."
        );
    }

    if (
        profitBeforeDepreciation >= 0.0 &&
        computedIncome < 0.0
    ) {
        result.notes.push_back(
            "Tax depreciation exceeds profit before depreciation. "
            "Any unabsorbed-depreciation treatment requires "
            "separate aggregation handling."
        );
    }

    return result;
}