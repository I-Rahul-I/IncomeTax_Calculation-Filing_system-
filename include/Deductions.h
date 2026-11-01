#ifndef DEDUCTIONS_H
#define DEDUCTIONS_H


#include <string>
#include <vector>


#include "Taxpayer.h"
#include "TaxRules.h"
#include "IncomeComputationResult.h"


using namespace std;


// Chapter VI-A categories supported by the initial model.
//
// Each category maps to a deduction rule code in the
// configuration file, such as DEDUCTION.80C.
enum class DeductionType {
    Section80C = 1,
    Section80CCC,
    Section80CCD1,
    Section80CCD1B,
    Section80CCD2,
    Section80CCG,
    Section80D,
    Section80DD,
    Section80DDB,
    Section80E,
    Section80EE,
    Section80EEA,
    Section80EEB,
    Section80G,
    Section80GGB,
    Section80GGC,
    Section80QQB,
    Section80RRB,
    Section80TTA,
    Section80TTB,
    Section80U
};


// One user-entered deduction claim.
//
// claimedAmount is the amount the user wants considered.
// It is not automatically the allowed amount.
struct DeductionClaim {
    DeductionType type = DeductionType::Section80C;

    string description;

    double claimedAmount = 0.0;

    // Optional supporting notes for the report.
    string remarks;

    // Section 80DDB-specific inputs. claimedAmount is gross eligible
    // medical expenditure; reimbursement is subtracted before applying
    // the age-based statutory ceiling. Zero means not supplied.
    int patientAge = 0;
    double reimbursementAmount = 0.0;
};


// Result for one deduction category.
struct DeductionResult {
    DeductionType type = DeductionType::Section80C;

    string ruleCode;

    string description;

    double claimedAmount = 0.0;

    double allowedAmount = 0.0;

    string explanation;
};


// Combined annual Chapter VI-A claims.
//
// This object is independent of income heads.
//
// The final calculator must:
// 1. Evaluate all income heads.
// 2. Calculate GTI.
// 3. Apply the combined 80C/80CCC/80CCD(1) ceiling.
// 4. Apply the GTI cap to the combined allowed deductions.
// 5. Ensure no deduction is applied twice.
class Deductions {
private:
    vector<DeductionClaim> claims;

    string description;

    static void validateAmount(
        double amount,
        const string& fieldName
    );

    static void validateClaim(
        const DeductionClaim& claim
    );

public:
    explicit Deductions(
        const string& description =
            "Annual Chapter VI-A Deductions"
    );

    // ========================================================
    // Claim management
    // ========================================================

    void addClaim(
        const DeductionClaim& claim
    );

    void clearClaims();

    const vector<DeductionClaim>& getClaims() const;

    // ========================================================
    // Totals
    // ========================================================

    double getClaimedAmountByType(
        DeductionType type
    ) const;

    double getTotalClaimedAmount() const;

    // ========================================================
    // Display helpers
    // ========================================================

    static string getDeductionTypeName(
        DeductionType type
    );

    // Returns the configuration rule code for a category.
    //
    // These codes match keys such as:
    // DEDUCTION.80C
    // DEDUCTION.80CCC
    // DEDUCTION.80CCD_1
    static string getDeductionRuleCode(
        DeductionType type
    );

    // ========================================================
    // Evaluation
    // ========================================================

    // Evaluates each claimed category independently.
    //
    // It does not apply:
    // - the combined 80C/80CCC/80CCD(1) ceiling
    // - the GTI cap
    //
    // Those cross-category rules belong to TaxCalculator.
    //
    // A category with no claim is omitted from the result.
    vector<DeductionResult> evaluateClaims(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const;

    // Convenience total before the combined ceiling and GTI cap.
    double getTotalAllowedBeforeGtiCap(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const;
};


#endif