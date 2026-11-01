#ifndef PAYMENTS_H
#define PAYMENTS_H

#include <string>
#include <vector>

#include "TaxRules.h"

using namespace std;

// Payment and credit categories.
enum class PaymentType {
    TaxDeductedAtSource = 1,
    TaxCollectedAtSource,
    AdvanceTax,
    SelfAssessmentTax
};

// One payment or credit entry.
//
// For TDS/TCS, amount is the tax credit claimed.
// It is not the gross receipt from which tax was deducted.
//
// For advance tax and self-assessment tax,
// amount is the tax paid.
struct PaymentEntry {
    PaymentType type = PaymentType::AdvanceTax;

    string description;

    double amount = 0.0;

    // Optional date in YYYY-MM-DD format.
    string paymentDate;

    // Optional reference, such as challan or certificate details.
    string reference;
};

// Combined annual payment credits.
//
// This object holds only credits entered for the current session.
// It does not retain prior-year balances or documents.
class Payments {
private:
    vector<PaymentEntry> entries;

    string description;

    static void validateAmount(
        double amount,
        const string& fieldName
    );

    static bool isValidPaymentDate(
        const string& value
    );

    static void validateEntry(
        const PaymentEntry& entry
    );

public:
    explicit Payments(
        const string& description =
            "Annual Tax Payments and Credits"
    );

    // ========================================================
    // Entry management
    // ========================================================

    void addEntry(
        const PaymentEntry& entry
    );

    void clearEntries();

    const vector<PaymentEntry>& getEntries() const;

    // ========================================================
    // Totals
    // ========================================================

    double getAmountByType(
        PaymentType type
    ) const;

    double getTotalTds() const;

    double getTotalTcs() const;

    double getTotalAdvanceTax() const;

    double getTotalSelfAssessmentTax() const;

    double getTotalPaymentsAndCredits() const;

    // ========================================================
    // Display helper
    // ========================================================

    static string getPaymentTypeName(
        PaymentType type
    );

    // ========================================================
    // Validation and reporting
    // ========================================================

    // Checks that entries exist and totals are valid.
    //
    // This does not determine whether a credit is legally
    // available or whether it can be claimed in the year.
    void validateAgainstRules(
        const TaxRules& rules
    ) const;

    // Returns a concise report of entered credits.
    vector<string> getReportLines() const;
};

#endif