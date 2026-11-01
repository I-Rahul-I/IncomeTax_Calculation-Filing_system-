#include "ReportGenerator.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {

string trimReportText(const string& value) {
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

} // namespace

// ============================================================
// Formatting helpers
// ============================================================

string ReportGenerator::formatAmount(
    double amount
) {
    if (!isfinite(amount)) {
        throw invalid_argument(
            "Cannot format a non-finite amount."
        );
    }

    ostringstream stream;

    stream << fixed << setprecision(2) << amount;

    return stream.str();
}

string ReportGenerator::formatSignedAmount(
    double amount
) {
    if (amount < 0.0) {
        return "-" + formatAmount(-amount);
    }

    return formatAmount(amount);
}

string ReportGenerator::getTaxpayerTypeName(
    TaxpayerType type
) {
    switch (type) {
        case TaxpayerType::Individual:
            return "Individual";

        case TaxpayerType::HUF:
            return "HUF";

        default:
            return "Unknown";
    }
}

string ReportGenerator::getRegimeName(
    TaxRegime regime
) {
    switch (regime) {
        case TaxRegime::New:
            return "New Regime";

        case TaxRegime::Old:
            return "Old Regime";

        default:
            return "Unknown Regime";
    }
}

string ReportGenerator::getIncomeHeadName(
    IncomeHead head
) {
    switch (head) {
        case IncomeHead::Salary:
            return "Salary";

        case IncomeHead::HouseProperty:
            return "House Property";

        case IncomeHead::BusinessProfession:
            return "Business or Profession";

        case IncomeHead::CapitalGains:
            return "Capital Gains";

        case IncomeHead::OtherSources:
            return "Other Sources";

        case IncomeHead::ExemptIncome:
            return "Exempt Income";

        default:
            return "Unknown Income Head";
    }
}

string ReportGenerator::getReportTitle(
    const TaxComputationResult& result
) {
    return "Annual Income-Tax Computation Summary";
}

void ReportGenerator::addSectionHeading(
    vector<string>& lines,
    const string& heading,
    ReportFormat format
) {
    if (format == ReportFormat::Markdown) {
        lines.push_back("## " + heading);
    } else {
        lines.push_back(heading);
        lines.push_back(string(heading.size(), '-'));
    }

    lines.push_back("");
}

// ============================================================
// Report sections
// ============================================================

void ReportGenerator::addIncomeSection(
    const TaxComputationResult& result,
    const ReportOptions& options,
    ReportFormat format,
    vector<string>& lines
) {
    if (!options.includeIncomeBreakdown) {
        return;
    }

    addSectionHeading(
        lines,
        "Income",
        format
    );

    lines.push_back(
        "Taxpayer category: " +
        getTaxpayerTypeName(result.taxpayerType)
    );

    lines.push_back(
        "Regime: " +
        getRegimeName(result.regime)
    );

    lines.push_back("");

    if (result.incomeHeadResults.empty()) {
        lines.push_back("No taxable income head was evaluated.");
        lines.push_back("");
    }

    for (const IncomeComputationResult& headResult :
         result.incomeHeadResults) {
        lines.push_back(
            getIncomeHeadName(headResult.head) +
            " - " + headResult.description
        );

        for (const IncomeBreakdownItem& item :
             headResult.breakdown) {
            lines.push_back(
                "  " + item.description + ": " +
                formatSignedAmount(item.amount)
            );
        }

        for (const IncomeAdjustment& adjustment :
             headResult.adjustments) {
            lines.push_back(
                "  Adjustment: " + adjustment.description +
                " | Claimed: " +
                formatAmount(adjustment.claimedAmount) +
                " | Allowed: " +
                formatAmount(adjustment.allowedAmount)
            );

            if (!adjustment.explanation.empty()) {
                lines.push_back(
                    "    " + adjustment.explanation
                );
            }
        }

        lines.push_back("");
    }

    lines.push_back(
        "Gross Total Income: " +
        formatAmount(result.grossTotalIncome)
    );

    lines.push_back("");
}

void ReportGenerator::addDeductionSection(
    const TaxComputationResult& result,
    const ReportOptions& options,
    ReportFormat format,
    vector<string>& lines
) {
    if (!options.includeDeductions) {
        return;
    }

    addSectionHeading(
        lines,
        "Deductions",
        format
    );

    if (result.adjustments.empty()) {
        lines.push_back("No deduction adjustment was reported.");
        lines.push_back("");
    }

    for (const TaxAdjustment& adjustment : result.adjustments) {
        lines.push_back(
            adjustment.code + " - " +
            adjustment.description
        );

        lines.push_back(
            "  Claimed: " +
            formatAmount(adjustment.claimedAmount)
        );

        lines.push_back(
            "  Allowed: " +
            formatAmount(adjustment.allowedAmount)
        );

        if (!adjustment.explanation.empty()) {
            lines.push_back(
                "  " + adjustment.explanation
            );
        }

        lines.push_back("");
    }

    lines.push_back(
        "Total Chapter VI-A deductions: " +
        formatAmount(result.totalDeductions)
    );

    lines.push_back(
        "Total income after deductions and rounding: " +
        formatAmount(result.totalIncome)
    );

    lines.push_back("");
}

void ReportGenerator::addTaxSection(
    const TaxComputationResult& result,
    const ReportOptions& options,
    ReportFormat format,
    vector<string>& lines
) {
    if (!options.includeTaxComputation) {
        return;
    }

    addSectionHeading(
        lines,
        "Tax Computation",
        format
    );

    lines.push_back(
        "Tax on slab income before rebate: " +
        formatAmount(result.slabTax)
    );

    lines.push_back(
        "Special-rate tax before rebate: " +
        formatAmount(result.specialRateTax)
    );

    lines.push_back(
        "Rebate: " +
        formatAmount(result.rebate)
    );

    lines.push_back(
        "Surcharge: " +
        formatAmount(result.surcharge)
    );

    lines.push_back(
        "Health and education cess: " +
        formatAmount(result.cess)
    );

    lines.push_back(
        "Total tax liability before credits: " +
        formatAmount(result.totalTaxLiability)
    );

    lines.push_back("");
}

void ReportGenerator::addPaymentSection(
    const TaxComputationResult& result,
    const Payments& payments,
    const ReportOptions& options,
    ReportFormat format,
    vector<string>& lines
) {
    if (!options.includePaymentCredits) {
        return;
    }

    addSectionHeading(
        lines,
        "Payments and Credits",
        format
    );

    for (const string& line : payments.getReportLines()) {
        lines.push_back(line);
    }

    lines.push_back("");

    lines.push_back(
        "Total credits applied: " +
        formatAmount(result.totalPaymentsAndCredits)
    );

    lines.push_back(
        "Balance tax payable: " +
        formatAmount(result.balanceTaxPayable)
    );

    lines.push_back(
        "Refund: " +
        formatAmount(result.refund)
    );

    lines.push_back("");
}

void ReportGenerator::addNotesSection(
    const TaxComputationResult& result,
    const ReportOptions& options,
    ReportFormat format,
    vector<string>& lines
) {
    if (!options.includeNotes || result.notes.empty()) {
        return;
    }

    addSectionHeading(
        lines,
        "Notes",
        format
    );

    for (const string& note : result.notes) {
        const string cleanedNote =
            trimReportText(note);

        if (!cleanedNote.empty()) {
            lines.push_back("- " + cleanedNote);
        }
    }

    lines.push_back("");
}

void ReportGenerator::addDisclaimer(
    const ReportOptions& options,
    ReportFormat format,
    vector<string>& lines
) {
    if (!options.includeDisclaimer) {
        return;
    }

    addSectionHeading(
        lines,
        "Disclaimer",
        format
    );

    lines.push_back(
        "This is a session-based computation summary. "
        "It does not determine legal eligibility, verify "
        "supporting conditions, or constitute tax advice."
    );

    lines.push_back(
        "All amounts are based solely on user-entered data "
        "and the loaded configuration. No prior-year data, "
        "permanent taxpayer record, or document upload is used."
    );

    lines.push_back("");
}

// ============================================================
// Public report methods
// ============================================================

vector<string> ReportGenerator::generateReportLines(
    const TaxComputationResult& result,
    const Payments& payments,
    ReportFormat format,
    const ReportOptions& options
) const {
    vector<string> lines;

    lines.push_back(getReportTitle(result));

    if (format == ReportFormat::Markdown) {
        lines.push_back("");
    } else {
        lines.push_back(
            string(getReportTitle(result).size(), '=')
        );
        lines.push_back("");
    }

    addIncomeSection(
        result,
        options,
        format,
        lines
    );

    addDeductionSection(
        result,
        options,
        format,
        lines
    );

    addTaxSection(
        result,
        options,
        format,
        lines
    );

    addPaymentSection(
        result,
        payments,
        options,
        format,
        lines
    );

    // Put the actionable amount at the end, in an unmistakable summary.
    if (options.includeTaxComputation && options.includePaymentCredits) {
        addSectionHeading(lines, "FINAL RESULT - AMOUNT TO PAY / REFUND", format);
        lines.push_back("Assessment Year: " + Taxpayer::getAssessmentYear());
        lines.push_back("Regime: " + getRegimeName(result.regime));
        lines.push_back("Total tax liability before credits: Rs. " + formatAmount(result.totalTaxLiability));
        lines.push_back("Less: payments and credits: Rs. " + formatAmount(result.totalPaymentsAndCredits));
        if (result.balanceTaxPayable > 0.0) {
            lines.push_back("*** BALANCE TAX PAYABLE: Rs. " + formatAmount(result.balanceTaxPayable) + " ***");
            lines.push_back("Pay this balance, subject to verification of your tax return and applicable rules.");
        } else if (result.refund > 0.0) {
            lines.push_back("*** ESTIMATED REFUND: Rs. " + formatAmount(result.refund) + " ***");
            lines.push_back("Refund is an estimate and remains subject to return processing.");
        } else {
            lines.push_back("*** NET TAX PAYABLE / REFUND: Rs. 0.00 ***");
        }
        lines.push_back("");
    }

    addNotesSection(
        result,
        options,
        format,
        lines
    );

    addDisclaimer(
        options,
        format,
        lines
    );

    return lines;
}

string ReportGenerator::generateReport(
    const TaxComputationResult& result,
    const Payments& payments,
    ReportFormat format,
    const ReportOptions& options
) const {
    const vector<string> lines =
        generateReportLines(
            result,
            payments,
            format,
            options
        );

    string report;

    for (size_t i = 0; i < lines.size(); ++i) {
        report += lines[i];

        if (i + 1 < lines.size()) {
            report += "\n";
        }
    }

    return report;
}

string ReportGenerator::generateReport(
    const TaxComputationResult& result,
    ReportFormat format,
    const ReportOptions& options
) const {
    Payments emptyPayments;

    return generateReport(
        result,
        emptyPayments,
        format,
        options
    );
}