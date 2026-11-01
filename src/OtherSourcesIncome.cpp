#include "OtherSourcesIncome.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

string trimOtherSourceText(const string& value) {
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

void checkOtherSourceTotal(
    double amount,
    const string& description
) {
    if (!isfinite(amount)) {
        throw overflow_error(
            description + " exceeded numeric limits."
        );
    }
}

bool validOtherSourceDate(const string& value) {
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

void addOtherSourceBreakdown(
    IncomeComputationResult& result,
    const string& description,
    double amount
) {
    IncomeBreakdownItem item;

    item.description = description;
    item.amount = amount;

    result.breakdown.push_back(item);
}

} // namespace

// ============================================================
// Constructor
// ============================================================

OtherSourcesIncome::OtherSourcesIncome(
    const string& description
)
    : Income(description) {
}

// ============================================================
// Validation
// ============================================================

void OtherSourcesIncome::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}

void OtherSourcesIncome::validateOrdinaryEntry(
    const OtherSourceEntry& entry
) {
    // Also validates the enumeration.
    getOrdinaryTypeName(entry.type);

    if (trimOtherSourceText(entry.description).empty()) {
        throw invalid_argument(
            "Other-source entry description cannot be empty."
        );
    }

    validateAmount(
        entry.amount,
        "Other-source income"
    );
}

void OtherSourcesIncome::validateSpecialEntry(
    const SpecialOtherSourceEntry& entry
) {
    // Also validates the enumeration.
    getSpecialTypeName(entry.type);

    if (trimOtherSourceText(entry.description).empty()) {
        throw invalid_argument(
            "Special-income description cannot be empty."
        );
    }

    validateAmount(
        entry.taxableAmount,
        "Special taxable income"
    );

    if (!entry.relevantDate.empty()) {
        if (
            !validOtherSourceDate(entry.relevantDate) ||
            entry.relevantDate < "2025-04-01" ||
            entry.relevantDate > "2026-03-31"
        ) {
            throw invalid_argument(
                "Special-income date must be a valid "
                "FY 2025-26 date in YYYY-MM-DD format."
            );
        }
    }
}

// ============================================================
// Ordinary income entries
// ============================================================

void OtherSourcesIncome::addOrdinaryEntry(
    const OtherSourceEntry& entry
) {
    validateOrdinaryEntry(entry);

    OtherSourceEntry cleanedEntry = entry;

    cleanedEntry.description =
        trimOtherSourceText(entry.description);

    ordinaryEntries.push_back(cleanedEntry);
}

void OtherSourcesIncome::clearOrdinaryEntries() {
    ordinaryEntries.clear();
}

const vector<OtherSourceEntry>&
OtherSourcesIncome::getOrdinaryEntries() const {
    return ordinaryEntries;
}

double OtherSourcesIncome::getOrdinaryAmountByType(
    OtherSourceType type
) const {
    getOrdinaryTypeName(type);

    double total = 0.0;

    for (const OtherSourceEntry& entry : ordinaryEntries) {
        if (entry.type == type) {
            total += entry.amount;

            checkOtherSourceTotal(
                total,
                "Other-source category total"
            );
        }
    }

    return total;
}

double OtherSourcesIncome::
getTotalOrdinaryIncomeEntered() const {
    double total = 0.0;

    for (const OtherSourceEntry& entry : ordinaryEntries) {
        total += entry.amount;

        checkOtherSourceTotal(
            total,
            "Total ordinary other-source income"
        );
    }

    return total;
}

// ============================================================
// Interest totals
// ============================================================

double OtherSourcesIncome::getSavingsAccountInterest() const {
    return getOrdinaryAmountByType(
        OtherSourceType::SavingsAccountInterest
    );
}

double OtherSourcesIncome::getFixedDepositInterest() const {
    return getOrdinaryAmountByType(
        OtherSourceType::FixedDepositInterest
    );
}

double OtherSourcesIncome::getRecurringDepositInterest() const {
    return getOrdinaryAmountByType(
        OtherSourceType::RecurringDepositInterest
    );
}

double OtherSourcesIncome::getOtherDepositInterest() const {
    return getOrdinaryAmountByType(
        OtherSourceType::OtherDepositInterest
    );
}

double OtherSourcesIncome::getBondInterest() const {
    return getOrdinaryAmountByType(
        OtherSourceType::BondInterest
    );
}

double OtherSourcesIncome::getTotalInterestEntered() const {
    const double total =
        getSavingsAccountInterest() +
        getFixedDepositInterest() +
        getRecurringDepositInterest() +
        getOtherDepositInterest() +
        getBondInterest();

    checkOtherSourceTotal(
        total,
        "Total interest income"
    );

    return total;
}

// ============================================================
// Other ordinary income totals
// ============================================================

double OtherSourcesIncome::getDividendIncome() const {
    return getOrdinaryAmountByType(
        OtherSourceType::Dividend
    );
}

double OtherSourcesIncome::getFamilyPensionIncome() const {
    return getOrdinaryAmountByType(
        OtherSourceType::FamilyPension
    );
}

double OtherSourcesIncome::getOtherOrdinaryIncome() const {
    return getOrdinaryAmountByType(
        OtherSourceType::OtherOrdinaryIncome
    );
}

// ============================================================
// Special-income entries
// ============================================================

void OtherSourcesIncome::addSpecialEntry(
    const SpecialOtherSourceEntry& entry
) {
    validateSpecialEntry(entry);

    SpecialOtherSourceEntry cleanedEntry = entry;

    cleanedEntry.description =
        trimOtherSourceText(entry.description);

    specialEntries.push_back(cleanedEntry);
}

void OtherSourcesIncome::clearSpecialEntries() {
    specialEntries.clear();
}

const vector<SpecialOtherSourceEntry>&
OtherSourcesIncome::getSpecialEntries() const {
    return specialEntries;
}

double OtherSourcesIncome::
getTotalSpecialIncomeEntered() const {
    double total = 0.0;

    for (const SpecialOtherSourceEntry& entry : specialEntries) {
        total += entry.taxableAmount;

        checkOtherSourceTotal(
            total,
            "Total special other-source income"
        );
    }

    return total;
}

string OtherSourcesIncome::getSpecialRuleCode(
    SpecialOtherSourceType type
) {
    switch (type) {
        case SpecialOtherSourceType::LotteryAndSimilarWinnings:
            return "115BB";

        case SpecialOtherSourceType::OnlineGameNetWinnings:
            return "115BBJ";

        case SpecialOtherSourceType::VirtualDigitalAssetIncome:
            return "115BBH";

        default:
            throw invalid_argument(
                "Invalid special other-source category."
            );
    }
}

// ============================================================
// Display helpers
// ============================================================

string OtherSourcesIncome::getOrdinaryTypeName(
    OtherSourceType type
) {
    switch (type) {
        case OtherSourceType::SavingsAccountInterest:
            return "Savings Account Interest";

        case OtherSourceType::FixedDepositInterest:
            return "Fixed Deposit Interest";

        case OtherSourceType::RecurringDepositInterest:
            return "Recurring Deposit Interest";

        case OtherSourceType::OtherDepositInterest:
            return "Other Deposit Interest";

        case OtherSourceType::BondInterest:
            return "Bond Interest";

        case OtherSourceType::Dividend:
            return "Dividend Income";

        case OtherSourceType::FamilyPension:
            return "Family Pension";

        case OtherSourceType::OtherOrdinaryIncome:
            return "Other Ordinary Income";

        default:
            throw invalid_argument(
                "Invalid ordinary other-source category."
            );
    }
}

string OtherSourcesIncome::getSpecialTypeName(
    SpecialOtherSourceType type
) {
    switch (type) {
        case SpecialOtherSourceType::LotteryAndSimilarWinnings:
            return "Lottery and Similar Winnings";

        case SpecialOtherSourceType::OnlineGameNetWinnings:
            return "Online Game Net Winnings";

        case SpecialOtherSourceType::VirtualDigitalAssetIncome:
            return "Virtual Digital Asset Income";

        default:
            throw invalid_argument(
                "Invalid special other-source category."
            );
    }
}

// ============================================================
// Income interface
// ============================================================

IncomeHead OtherSourcesIncome::getHead() const {
    return IncomeHead::OtherSources;
}

string OtherSourcesIncome::getHeadName() const {
    return "Other Sources";
}

IncomeComputationResult OtherSourcesIncome::evaluate(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before evaluating other-source income."
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
                "The current other-sources module supports "
                "Individual and HUF taxpayers only."
            );
    }

    if (!rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "Taxpayer category is disabled in this configuration."
        );
    }

    const double ordinaryTotal =
        getTotalOrdinaryIncomeEntered();

    const double specialTotal =
        getTotalSpecialIncomeEntered();

    const double familyPension =
        getFamilyPensionIncome();

    if (
        familyPension > 0.0 &&
        taxpayer.getType() != TaxpayerType::Individual
    ) {
        throw invalid_argument(
            "Family-pension input is supported only for "
            "Individual taxpayers."
        );
    }

    IncomeComputationResult result;

    result.head = getHead();
    result.description = getDescription();

    result.grossAmount = ordinaryTotal + specialTotal;

    checkOtherSourceTotal(
        result.grossAmount,
        "Combined other-source income"
    );

    // --------------------------------------------------------
    // Ordinary receipts
    // --------------------------------------------------------

    for (const OtherSourceEntry& entry : ordinaryEntries) {
        addOtherSourceBreakdown(
            result,
            getOrdinaryTypeName(entry.type) +
                ": " + entry.description,
            entry.amount
        );
    }

    addOtherSourceBreakdown(
        result,
        "Total ordinary other-source income before adjustments",
        ordinaryTotal
    );

    // --------------------------------------------------------
    // Family pension: apply once to the annual total
    // --------------------------------------------------------

    double familyPensionDeduction = 0.0;

    if (familyPension > 0.0) {
        const double percentage = rules.getNumber(
            "FAMILY_PENSION_DEDUCTION_PERCENT"
        );

        const double maximum = rules.getNumber(
            "FAMILY_PENSION_DEDUCTION_MAX"
        );

        if (
            percentage < 0.0 ||
            percentage > 100.0 ||
            maximum < 0.0
        ) {
            throw invalid_argument(
                "Invalid family-pension deduction configuration."
            );
        }

        familyPensionDeduction = min(
            familyPension,
            min(
                maximum,
                familyPension * percentage / 100.0
            )
        );

        IncomeAdjustment adjustment;

        adjustment.code = "FAMILY_PENSION_DEDUCTION";
        adjustment.description =
            "Annual family-pension deduction";

        adjustment.claimedAmount =
            familyPensionDeduction;

        adjustment.allowedAmount =
            familyPensionDeduction;

        adjustment.explanation =
            "Calculated once from combined annual family "
            "pension using the selected regime configuration.";

        result.adjustments.push_back(adjustment);

        addOtherSourceBreakdown(
            result,
            "Less: Family-pension deduction",
            -familyPensionDeduction
        );
    }

    result.normalRateIncome =
        ordinaryTotal - familyPensionDeduction;

    checkOtherSourceTotal(
        result.normalRateIncome,
        "Taxable ordinary other-source income"
    );

    addOtherSourceBreakdown(
        result,
        "Ordinary other-source income after adjustments",
        result.normalRateIncome
    );

    // --------------------------------------------------------
    // Special-income components
    // --------------------------------------------------------

    for (const SpecialOtherSourceEntry& entry : specialEntries) {
        const string ruleCode =
            getSpecialRuleCode(entry.type);

        // Require the rule explicitly. Tax is calculated later.
        rules.getSpecialIncomeRule(ruleCode);

        if (entry.taxableAmount == 0.0) {
            continue;
        }

        SpecialIncomeComponent component;

        component.ruleCode = ruleCode;
        component.description = entry.description;
        component.amount = entry.taxableAmount;
        component.relevantDate = entry.relevantDate;

        result.specialRateComponents.push_back(component);

        addOtherSourceBreakdown(
            result,
            getSpecialTypeName(entry.type) +
                ": " + entry.description,
            entry.taxableAmount
        );

        if (
            entry.type ==
            SpecialOtherSourceType::OnlineGameNetWinnings
        ) {
            result.notes.push_back(
                "Online-game income is based on user-declared "
                "taxable net winnings, not gross withdrawals."
            );
        }

        if (
            entry.type ==
            SpecialOtherSourceType::VirtualDigitalAssetIncome
        ) {
            result.notes.push_back(
                "VDA amount is user-declared taxable income. "
                "Transaction costs, loss restrictions and "
                "income-head classification are not independently "
                "determined by this module."
            );
        }

        if (entry.relevantDate.empty()) {
            result.notes.push_back(
                "No date was provided for special-income entry: " +
                entry.description +
                ". Any calculation requiring income timing "
                "must request that information before proceeding."
            );
        }
    }

    // --------------------------------------------------------
    // Report notes
    // --------------------------------------------------------

    result.notes.push_back(
        "Interest deductions under Chapter VI-A are evaluated "
        "separately. No 80TTA or 80TTB deduction has been "
        "subtracted by this module."
    );

    result.notes.push_back(
        "TDS and TCS are separate payment credits and have not "
        "been subtracted from these gross income entries."
    );

    result.notes.push_back(
        "Any family-pension adjustment shown above is already "
        "included in ordinary taxable income and must not "
        "be deducted again after GTI."
    );

    if (getDividendIncome() > 0.0) {
        result.notes.push_back(
            "Dividend income remains identifiable in the original "
            "entries for later component-specific tax handling."
        );
    }

    return result;
}