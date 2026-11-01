#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <string>
#include <vector>

#include "TaxComputationResult.h"
#include "Payments.h"

using namespace std;

// Supported report formats.
enum class ReportFormat {
    PlainText = 1,
    Markdown
};

// Options controlling report detail.
struct ReportOptions {
    bool includeIncomeBreakdown = true;

    bool includeDeductions = true;

    bool includeTaxComputation = true;

    bool includePaymentCredits = true;

    bool includeNotes = true;

    bool includeDisclaimer = true;
};

// Generates a human-readable annual computation report.
//
// The generator only formats an already calculated result.
// It does not recalculate tax or alter any amount.
class ReportGenerator {
private:
    static string formatAmount(
        double amount
    );

    static string formatSignedAmount(
        double amount
    );

    static string getTaxpayerTypeName(
        TaxpayerType type
    );

    static string getRegimeName(
        TaxRegime regime
    );

    static string getIncomeHeadName(
        IncomeHead head
    );

    static string getReportTitle(
        const TaxComputationResult& result
    );

    static void addSectionHeading(
        vector<string>& lines,
        const string& heading,
        ReportFormat format
    );

    static void addIncomeSection(
        const TaxComputationResult& result,
        const ReportOptions& options,
        ReportFormat format,
        vector<string>& lines
    );

    static void addDeductionSection(
        const TaxComputationResult& result,
        const ReportOptions& options,
        ReportFormat format,
        vector<string>& lines
    );

    static void addTaxSection(
        const TaxComputationResult& result,
        const ReportOptions& options,
        ReportFormat format,
        vector<string>& lines
    );

    static void addPaymentSection(
        const TaxComputationResult& result,
        const Payments& payments,
        const ReportOptions& options,
        ReportFormat format,
        vector<string>& lines
    );

    static void addNotesSection(
        const TaxComputationResult& result,
        const ReportOptions& options,
        ReportFormat format,
        vector<string>& lines
    );

    static void addDisclaimer(
        const ReportOptions& options,
        ReportFormat format,
        vector<string>& lines
    );

public:
    ReportGenerator() = default;

    // Generates a report from a completed calculation.
    //
    // Payment details are optional. If supplied, the report
    // includes itemized TDS, TCS, advance-tax and
    // self-assessment-tax entries.
    string generateReport(
        const TaxComputationResult& result,
        const Payments& payments,
        ReportFormat format = ReportFormat::PlainText,
        const ReportOptions& options = ReportOptions()
    ) const;

    // Generates a report without itemized payment entries.
    string generateReport(
        const TaxComputationResult& result,
        ReportFormat format = ReportFormat::PlainText,
        const ReportOptions& options = ReportOptions()
    ) const;

    // Returns the report as separate lines.
    vector<string> generateReportLines(
        const TaxComputationResult& result,
        const Payments& payments,
        ReportFormat format = ReportFormat::PlainText,
        const ReportOptions& options = ReportOptions()
    ) const;
};

#endif