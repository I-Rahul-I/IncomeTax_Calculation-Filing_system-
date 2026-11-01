#include "ExemptIncome.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

string trimExemptText(const string& value) {
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

void checkExemptTotal(
    double amount,
    const string& description
) {
    if (!isfinite(amount)) {
        throw overflow_error(
            description + " exceeded numeric limits."
        );
    }
}

void addExemptBreakdown(
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

ExemptIncome::ExemptIncome(
    const string& description
)
    : Income(description) {
}

// ============================================================
// Validation
// ============================================================

void ExemptIncome::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}

void ExemptIncome::validateEntry(
    const ExemptIncomeEntry& entry
) {
    // Also validates the enumeration.
    getExemptTypeName(entry.type);

    if (trimExemptText(entry.description).empty()) {
        throw invalid_argument(
            "Exempt-income description cannot be empty."
        );
    }

    validateAmount(
        entry.amount,
        "Exempt income"
    );

    if (entry.amount == 0.0) {
        throw invalid_argument(
            "Exempt-income amount must be greater than zero."
        );
    }
}

// ============================================================
// Entry management
// ============================================================

void ExemptIncome::addEntry(
    const ExemptIncomeEntry& entry
) {
    validateEntry(entry);

    ExemptIncomeEntry cleanedEntry = entry;

    cleanedEntry.description =
        trimExemptText(entry.description);

    cleanedEntry.exemptionReference =
        trimExemptText(entry.exemptionReference);

    cleanedEntry.remarks =
        trimExemptText(entry.remarks);

    entries.push_back(cleanedEntry);
}

void ExemptIncome::clearEntries() {
    entries.clear();
}

const vector<ExemptIncomeEntry>&
ExemptIncome::getEntries() const {
    return entries;
}

// ============================================================
// Totals
// ============================================================

double ExemptIncome::getAmountByType(
    ExemptIncomeType type
) const {
    getExemptTypeName(type);

    double total = 0.0;

    for (const ExemptIncomeEntry& entry : entries) {
        if (entry.type == type) {
            total += entry.amount;

            checkExemptTotal(
                total,
                "Exempt-income category total"
            );
        }
    }

    return total;
}

double ExemptIncome::getTotalExemptIncome() const {
    double total = 0.0;

    for (const ExemptIncomeEntry& entry : entries) {
        total += entry.amount;

        checkExemptTotal(
            total,
            "Total exempt income"
        );
    }

    return total;
}

double ExemptIncome::getAgriculturalIncome() const {
    return getAmountByType(
        ExemptIncomeType::AgriculturalIncome
    );
}

double ExemptIncome::getNonAgriculturalExemptIncome() const {
    return getTotalExemptIncome() -
        getAgriculturalIncome();
}

// ============================================================
// Display helper
// ============================================================

string ExemptIncome::getExemptTypeName(
    ExemptIncomeType type
) {
    switch (type) {
        case ExemptIncomeType::AgriculturalIncome:
            return "Agricultural Income";

        case ExemptIncomeType::Scholarship:
            return "Scholarship";

        case ExemptIncomeType::PPFInterest:
            return "PPF Interest";

        case ExemptIncomeType::EligibleInsuranceProceeds:
            return "Eligible Insurance Proceeds";

        case ExemptIncomeType::OtherDeclaredExemptIncome:
            return "Other Declared Exempt Income";

        default:
            throw invalid_argument(
                "Invalid exempt-income category."
            );
    }
}

// ============================================================
// Income interface
// ============================================================

IncomeHead ExemptIncome::getHead() const {
    return IncomeHead::ExemptIncome;
}

string ExemptIncome::getHeadName() const {
    return "Exempt Income";
}

IncomeComputationResult ExemptIncome::evaluate(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before evaluating exempt income."
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
                "The current exempt-income module supports "
                "Individual and HUF taxpayers only."
            );
    }

    if (!rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "Taxpayer category is disabled in this configuration."
        );
    }

    const double total = getTotalExemptIncome();

    IncomeComputationResult result;

    result.head = getHead();
    result.description = getDescription();

    // Exempt income is reported, but it is not taxable income.
    result.grossAmount = total;

    result.normalRateIncome = 0.0;

    for (const ExemptIncomeEntry& entry : entries) {
        addExemptBreakdown(
            result,
            getExemptTypeName(entry.type) +
                ": " + entry.description,
            entry.amount
        );

        if (!entry.exemptionReference.empty()) {
            result.notes.push_back(
                getExemptTypeName(entry.type) +
                " reference: " +
                entry.exemptionReference
            );
        }

        if (!entry.remarks.empty()) {
            result.notes.push_back(
                getExemptTypeName(entry.type) +
                " remarks: " +
                entry.remarks
            );
        }
    }

    addExemptBreakdown(
        result,
        "Total exempt income declared",
        total
    );

    // --------------------------------------------------------
    // Agricultural income is retained separately.
    // --------------------------------------------------------

    const double agriculturalIncome =
        getAgriculturalIncome();

    if (agriculturalIncome > 0.0) {
        result.notes.push_back(
            "Agricultural income is declared separately. "
            "The final calculator must check whether any "
            "supported rate-purpose treatment applies; "
            "this module does not automatically apply it."
        );
    }

    // --------------------------------------------------------
    // Important reporting limitations
    // --------------------------------------------------------

    result.notes.push_back(
        "The amount entered is the user-declared exempt portion. "
        "This module does not determine legal eligibility, "
        "taxable components, or required supporting conditions."
    );

    result.notes.push_back(
        "Any taxable portion of a receipt must be entered "
        "under the relevant taxable income module and must "
        "not be duplicated here."
    );

    result.notes.push_back(
        "This result is informational only. Do not add it "
        "to Gross Total Income as taxable income."
    );

    return result;
}