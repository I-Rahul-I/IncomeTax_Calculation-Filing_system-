#include "Payments.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

string trimPaymentText(const string& value) {
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

void checkPaymentTotal(
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

Payments::Payments(
    const string& description
) : description(description) {
}

// ============================================================
// Validation
// ============================================================

void Payments::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}

bool Payments::isValidPaymentDate(
    const string& value
) {
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

void Payments::validateEntry(
    const PaymentEntry& entry
) {
    // Also validates the enumeration.
    getPaymentTypeName(entry.type);

    if (trimPaymentText(entry.description).empty()) {
        throw invalid_argument(
            "Payment description cannot be empty."
        );
    }

    validateAmount(
        entry.amount,
        "Payment or credit amount"
    );

    if (entry.amount == 0.0) {
        throw invalid_argument(
            "Payment or credit amount must be greater than zero."
        );
    }

    if (!entry.paymentDate.empty()) {
        if (
            !isValidPaymentDate(entry.paymentDate) ||
            entry.paymentDate < "2025-04-01" ||
            entry.paymentDate > "2026-03-31"
        ) {
            throw invalid_argument(
                "Payment date must be a valid FY 2025-26 date "
                "in YYYY-MM-DD format."
            );
        }
    }
}

// ============================================================
// Entry management
// ============================================================

void Payments::addEntry(
    const PaymentEntry& entry
) {
    validateEntry(entry);

    PaymentEntry cleanedEntry = entry;

    cleanedEntry.description =
        trimPaymentText(entry.description);

    cleanedEntry.paymentDate =
        trimPaymentText(entry.paymentDate);

    cleanedEntry.reference =
        trimPaymentText(entry.reference);

    entries.push_back(cleanedEntry);
}

void Payments::clearEntries() {
    entries.clear();
}

const vector<PaymentEntry>&
Payments::getEntries() const {
    return entries;
}

// ============================================================
// Totals
// ============================================================

double Payments::getAmountByType(
    PaymentType type
) const {
    getPaymentTypeName(type);

    double total = 0.0;

    for (const PaymentEntry& entry : entries) {
        if (entry.type == type) {
            total += entry.amount;

            checkPaymentTotal(
                total,
                "Payment category total"
            );
        }
    }

    return total;
}

double Payments::getTotalTds() const {
    return getAmountByType(
        PaymentType::TaxDeductedAtSource
    );
}

double Payments::getTotalTcs() const {
    return getAmountByType(
        PaymentType::TaxCollectedAtSource
    );
}

double Payments::getTotalAdvanceTax() const {
    return getAmountByType(
        PaymentType::AdvanceTax
    );
}

double Payments::getTotalSelfAssessmentTax() const {
    return getAmountByType(
        PaymentType::SelfAssessmentTax
    );
}

double Payments::getTotalPaymentsAndCredits() const {
    const double total =
        getTotalTds() +
        getTotalTcs() +
        getTotalAdvanceTax() +
        getTotalSelfAssessmentTax();

    checkPaymentTotal(
        total,
        "Total payments and credits"
    );

    return total;
}

// ============================================================
// Display helper
// ============================================================

string Payments::getPaymentTypeName(
    PaymentType type
) {
    switch (type) {
        case PaymentType::TaxDeductedAtSource:
            return "Tax Deducted at Source";

        case PaymentType::TaxCollectedAtSource:
            return "Tax Collected at Source";

        case PaymentType::AdvanceTax:
            return "Advance Tax";

        case PaymentType::SelfAssessmentTax:
            return "Self-Assessment Tax";

        default:
            throw invalid_argument(
                "Invalid payment or credit category."
            );
    }
}

// ============================================================
// Validation and reporting
// ============================================================

void Payments::validateAgainstRules(
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before validating payments."
        );
    }

    if (!rules.getBoolean("SUPPORT_TDS_TCS")) {
        if (
            getTotalTds() > 0.0 ||
            getTotalTcs() > 0.0
        ) {
            throw invalid_argument(
                "TDS and TCS credits are disabled in the "
                "current configuration."
            );
        }
    }

    if (!rules.getBoolean("SUPPORT_ADVANCE_TAX")) {
        if (getTotalAdvanceTax() > 0.0) {
            throw invalid_argument(
                "Advance-tax entries are disabled in the "
                "current configuration."
            );
        }
    }

    if (!rules.getBoolean("SUPPORT_SELF_ASSESSMENT_TAX")) {
        if (getTotalSelfAssessmentTax() > 0.0) {
            throw invalid_argument(
                "Self-assessment-tax entries are disabled in "
                "the current configuration."
            );
        }
    }

    if (getTotalPaymentsAndCredits() < 0.0) {
        throw logic_error(
            "Payment totals cannot be negative."
        );
    }
}

vector<string> Payments::getReportLines() const {
    vector<string> lines;

    lines.push_back(
        "Total TDS credit: " +
        to_string(getTotalTds())
    );

    lines.push_back(
        "Total TCS credit: " +
        to_string(getTotalTcs())
    );

    lines.push_back(
        "Total advance tax: " +
        to_string(getTotalAdvanceTax())
    );

    lines.push_back(
        "Total self-assessment tax: " +
        to_string(getTotalSelfAssessmentTax())
    );

    lines.push_back(
        "Total payments and credits: " +
        to_string(getTotalPaymentsAndCredits())
    );

    for (const PaymentEntry& entry : entries) {
        string line =
            getPaymentTypeName(entry.type) +
            ": " + entry.description +
            " - " + to_string(entry.amount);

        if (!entry.paymentDate.empty()) {
            line += " | Date: " + entry.paymentDate;
        }

        if (!entry.reference.empty()) {
            line += " | Reference: " + entry.reference;
        }

        lines.push_back(line);
    }

    lines.push_back(
        "Credits are entered amounts only. Eligibility and "
        "year-wise availability are not independently verified."
    );

    return lines;
}