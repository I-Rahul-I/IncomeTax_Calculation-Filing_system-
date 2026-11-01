#ifndef TAX_CALCULATOR_H
#define TAX_CALCULATOR_H


#include <string>
#include <vector>


#include "Taxpayer.h"
#include "TaxRules.h"
#include "Income.h"
#include "Deductions.h"
#include "Payments.h"
#include "TaxComputationResult.h"


using namespace std;


// Inputs for one complete calculation.
//
// Every object must belong to the same taxpayer and the same
// annual session. The calculator does not merge data from
// multiple taxpayers or prior-year sessions.
struct TaxCalculationInput {
    // Taxpayer is abstract, so the calculator receives a pointer
    // to an IndividualTaxpayer, HUFTaxpayer, or another concrete
    // Taxpayer-derived object.
    const Taxpayer* taxpayer = nullptr;

    TaxRules rules;

    vector<Income*> incomeHeads;

    Deductions deductions;

    Payments payments;
};


// Central annual tax calculator.
//
// This class owns the final ordering of calculations.
// Individual income modules must not independently apply:
// - GTI
// - the overall Chapter VI-A GTI cap
// - slab tax
// - rebate
// - surcharge
// - cess
// - payment credits
class TaxCalculator {
private:
    static void validateInput(
        const TaxCalculationInput& input
    );

    static void validateIncomeHeadPointers(
        const vector<Income*>& incomeHeads
    );

    static vector<IncomeComputationResult>
    evaluateAllIncomeHeads(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );

    static void aggregateIncomeHeads(
        vector<IncomeComputationResult>& headResults,
        RegimeType regime,
        TaxComputationResult& result
    );

    static void calculateDeductions(
        const TaxCalculationInput& input,
        double grossTotalIncome,
        TaxComputationResult& result
    );

    static void calculateSlabTax(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );

    static void calculateSpecialIncomeTax(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );

    static void calculateRebate(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );

    static void calculateSurcharge(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );

    static void calculateCess(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );

    static void applyPaymentCredits(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );

    static void addCalculationNotes(
        const TaxCalculationInput& input,
        TaxComputationResult& result
    );


public:
    TaxCalculator() = default;


    // Performs the complete annual calculation.
    //
    // The returned result includes:
    // - income-head evaluation and aggregation
    // - GTI
    // - Chapter VI-A deductions and GTI cap
    // - total income after rounding
    // - slab tax
    // - special-rate tax components
    // - rebate
    // - surcharge
    // - cess
    // - payment credits
    // - final liability or refund
    TaxComputationResult calculate(
        const TaxCalculationInput& input
    ) const;


    // Convenience overload for a single owned income head.
    TaxComputationResult calculate(
        const Taxpayer& taxpayer,
        const TaxRules& rules,
        Income& incomeHead,
        const Deductions& deductions,
        const Payments& payments
    ) const;
};


#endif