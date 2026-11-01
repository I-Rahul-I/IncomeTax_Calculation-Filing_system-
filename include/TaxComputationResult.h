#ifndef TAX_COMPUTATION_RESULT_H
#define TAX_COMPUTATION_RESULT_H

#include <string>
#include <vector>

#include "Taxpayer.h"
#include "IncomeComputationResult.h"

using namespace std;

// One line in the final computation report.
struct TaxBreakdownItem {
    string description;

    double amount = 0.0;
};

// One adjustment applied by the central calculator.
struct TaxAdjustment {
    string code;

    string description;

    double claimedAmount = 0.0;

    double allowedAmount = 0.0;

    string explanation;
};

// Complete result of one annual tax calculation.
//
// This object is produced only by TaxCalculator.
// It contains no taxpayer identity data.
struct TaxComputationResult {
    TaxpayerType taxpayerType = TaxpayerType::Individual;

    TaxRegime regime = TaxRegime::New;

    // Income-head evaluation results.
    vector<IncomeComputationResult> incomeHeadResults;

    // Final report lines.
    vector<TaxBreakdownItem> breakdown;

    // Deduction and calculation adjustments.
    vector<TaxAdjustment> adjustments;

    // Explanatory notes and limitations.
    vector<string> notes;

    // ========================================================
    // Income aggregation
    // ========================================================

    // Sum of taxable normal-rate income from all heads.
    // Exempt income is not included.
    double grossTotalIncome = 0.0;

    // ========================================================
    // Deductions
    // ========================================================

    // Chapter VI-A deductions after category limits,
    // combined 80C/80CCC/80CCD(1) ceiling, and GTI cap.
    double totalDeductions = 0.0;

    double totalIncomeBeforeRounding = 0.0;

    // Total income rounded to the nearest rupee.
    double totalIncome = 0.0;

    // ========================================================
    // Tax computation
    // ========================================================

    double slabTax = 0.0;

    double specialRateTax = 0.0;

    double rebate = 0.0;

    double surcharge = 0.0;

    double cess = 0.0;

    double totalTaxLiability = 0.0;

    // ========================================================
    // Payments and final balance
    // ========================================================

    double totalPaymentsAndCredits = 0.0;

    double balanceTaxPayable = 0.0;

    double refund = 0.0;
};

#endif