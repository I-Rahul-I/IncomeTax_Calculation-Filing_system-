#include "SalaryIncome.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

const double AMOUNT_TOLERANCE = 0.01;

bool validSalaryDate(const string& value) {
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

double readPercentage(
    const TaxRules& rules,
    const string& key
) {
    const double percentage = rules.getNumber(key);

    if (percentage < 0.0 || percentage > 100.0) {
        throw invalid_argument(
            "Invalid percentage in configuration: " + key
        );
    }

    return percentage;
}

void addBreakdown(
    IncomeComputationResult& result,
    const string& description,
    double amount
) {
    IncomeBreakdownItem item;
    item.description = description;
    item.amount = amount;

    result.breakdown.push_back(item);
}

void addAdjustment(
    IncomeComputationResult& result,
    const string& code,
    const string& description,
    double claimed,
    double allowed,
    const string& explanation
) {
    IncomeAdjustment adjustment;

    adjustment.code = code;
    adjustment.description = description;
    adjustment.claimedAmount = claimed;
    adjustment.allowedAmount = allowed;
    adjustment.explanation = explanation;

    result.adjustments.push_back(adjustment);
}

} // namespace

// ============================================================
// Constructor
// ============================================================

SalaryIncome::SalaryIncome(const string& description)
    : Income(description),
      basicSalary(0.0),
      dearnessAllowance(0.0),
      bonus(0.0),
      commission(0.0),
      houseRentAllowance(0.0),
      leaveTravelAllowance(0.0),
      otherTaxableAllowances(0.0),
      taxablePerquisites(0.0),
      ltaExemptionClaimed(0.0),
      professionalTaxPaid(0.0) {
}

// ============================================================
// Validation
// ============================================================

void SalaryIncome::validateAmount(
    double amount,
    const string& fieldName
) {
    if (!isfinite(amount) || amount < 0.0) {
        throw invalid_argument(
            fieldName + " must be finite and non-negative."
        );
    }
}

void SalaryIncome::validateHRAPeriod(
    const HRAPeriod& period
) {
    if (
        !validSalaryDate(period.startDate) ||
        !validSalaryDate(period.endDate)
    ) {
        throw invalid_argument(
            "HRA period dates must be valid YYYY-MM-DD dates."
        );
    }

    if (
        period.startDate < "2025-04-01" ||
        period.endDate > "2026-03-31" ||
        period.startDate > period.endDate
    ) {
        throw invalid_argument(
            "HRA period must lie within FY 2025-26."
        );
    }

    validateAmount(period.basicSalary, "HRA-period basic salary");

    validateAmount(
        period.eligibleDearnessAllowance,
        "HRA-period eligible DA"
    );

    validateAmount(
        period.eligibleTurnoverCommission,
        "HRA-period eligible commission"
    );

    validateAmount(period.hraReceived, "HRA-period HRA");
    validateAmount(period.rentPaid, "HRA-period rent");
}

// ============================================================
// Setters
// ============================================================

void SalaryIncome::setBasicSalary(double amount) {
    validateAmount(amount, "Basic salary");
    basicSalary = amount;
}

void SalaryIncome::setDearnessAllowance(double amount) {
    validateAmount(amount, "Dearness allowance");
    dearnessAllowance = amount;
}

void SalaryIncome::setBonus(double amount) {
    validateAmount(amount, "Bonus");
    bonus = amount;
}

void SalaryIncome::setCommission(double amount) {
    validateAmount(amount, "Commission");
    commission = amount;
}

void SalaryIncome::setHouseRentAllowance(double amount) {
    validateAmount(amount, "House Rent Allowance");
    houseRentAllowance = amount;
}

void SalaryIncome::setLeaveTravelAllowance(double amount) {
    validateAmount(amount, "Leave Travel Allowance");
    leaveTravelAllowance = amount;
}

void SalaryIncome::setOtherTaxableAllowances(double amount) {
    validateAmount(amount, "Other taxable allowances");
    otherTaxableAllowances = amount;
}

void SalaryIncome::setTaxablePerquisites(double amount) {
    validateAmount(amount, "Taxable perquisites");
    taxablePerquisites = amount;
}

void SalaryIncome::setLTAExemptionClaimed(double amount) {
    validateAmount(amount, "LTA exemption claim");
    ltaExemptionClaimed = amount;
}

void SalaryIncome::setProfessionalTaxPaid(double amount) {
    validateAmount(amount, "Professional tax");
    professionalTaxPaid = amount;
}

// ============================================================
// HRA periods
// ============================================================

void SalaryIncome::addHRAPeriod(const HRAPeriod& period) {
    validateHRAPeriod(period);

    for (const HRAPeriod& existing : hraPeriods) {
        const bool overlaps =
            period.startDate <= existing.endDate &&
            existing.startDate <= period.endDate;

        if (overlaps) {
            throw invalid_argument(
                "HRA periods cannot overlap."
            );
        }
    }

    hraPeriods.push_back(period);
}

void SalaryIncome::clearHRAPeriods() {
    hraPeriods.clear();
}

const vector<HRAPeriod>& SalaryIncome::getHRAPeriods() const {
    return hraPeriods;
}

// ============================================================
// Getters
// ============================================================

double SalaryIncome::getBasicSalary() const {
    return basicSalary;
}

double SalaryIncome::getDearnessAllowance() const {
    return dearnessAllowance;
}

double SalaryIncome::getBonus() const {
    return bonus;
}

double SalaryIncome::getCommission() const {
    return commission;
}

double SalaryIncome::getHouseRentAllowance() const {
    return houseRentAllowance;
}

double SalaryIncome::getLeaveTravelAllowance() const {
    return leaveTravelAllowance;
}

double SalaryIncome::getOtherTaxableAllowances() const {
    return otherTaxableAllowances;
}

double SalaryIncome::getTaxablePerquisites() const {
    return taxablePerquisites;
}

double SalaryIncome::getLTAExemptionClaimed() const {
    return ltaExemptionClaimed;
}

double SalaryIncome::getProfessionalTaxPaid() const {
    return professionalTaxPaid;
}

double SalaryIncome::getGrossSalary() const {
    const double gross =
        basicSalary +
        dearnessAllowance +
        bonus +
        commission +
        houseRentAllowance +
        leaveTravelAllowance +
        otherTaxableAllowances +
        taxablePerquisites;

    if (!isfinite(gross)) {
        throw overflow_error(
            "Combined salary amount is too large."
        );
    }

    return gross;
}

// ============================================================
// HRA calculation
// ============================================================

double SalaryIncome::calculateHRAExemption(
    const TaxRules& rules
) const {
    if (
        hraPeriods.empty() ||
        !rules.getBoolean("HRA_EXEMPTION_ALLOWED")
    ) {
        return 0.0;
    }

    const double metroPercent =
        readPercentage(rules, "HRA_METRO_SALARY_PERCENT");

    const double nonMetroPercent =
        readPercentage(rules, "HRA_NON_METRO_SALARY_PERCENT");

    const double rentSalaryPercent =
        readPercentage(rules, "HRA_RENT_SALARY_PERCENT");

    double exemption = 0.0;

    for (const HRAPeriod& period : hraPeriods) {
        const double eligibleSalary =
            period.basicSalary +
            period.eligibleDearnessAllowance +
            period.eligibleTurnoverCommission;

        if (!isfinite(eligibleSalary)) {
            throw overflow_error(
                "HRA salary basis is too large."
            );
        }

        const double cityPercent =
            period.eligibleMetroCity
                ? metroPercent
                : nonMetroPercent;

        const double rentExcess = max(
            0.0,
            period.rentPaid -
                eligibleSalary * rentSalaryPercent / 100.0
        );

        const double salaryLimit =
            eligibleSalary * cityPercent / 100.0;

        const double periodExemption = min(
            period.hraReceived,
            min(rentExcess, salaryLimit)
        );

        exemption += periodExemption;
    }

    if (!isfinite(exemption)) {
        throw overflow_error(
            "Total HRA exemption is too large."
        );
    }

    return min(exemption, houseRentAllowance);
}

// ============================================================
// Income interface
// ============================================================

IncomeHead SalaryIncome::getHead() const {
    return IncomeHead::Salary;
}

string SalaryIncome::getHeadName() const {
    return "Salary";
}

IncomeComputationResult SalaryIncome::evaluate(
    const Taxpayer& taxpayer,
    const TaxRules& rules
) const {
    if (!rules.isLoaded()) {
        throw logic_error(
            "Load tax rules before evaluating salary income."
        );
    }

    if (taxpayer.getType() != TaxpayerType::Individual) {
        throw invalid_argument(
            "The salary module supports Individual taxpayers only."
        );
    }

    // Validate aggregate HRA-period inputs even when evaluating
    // the new regime, so inconsistent user data is not concealed.
    double periodBasic = 0.0;
    double periodDA = 0.0;
    double periodCommission = 0.0;
    double periodHRA = 0.0;

    for (const HRAPeriod& period : hraPeriods) {
        validateHRAPeriod(period);

        periodBasic += period.basicSalary;
        periodDA += period.eligibleDearnessAllowance;
        periodCommission += period.eligibleTurnoverCommission;
        periodHRA += period.hraReceived;
    }

    if (
        !isfinite(periodBasic) ||
        !isfinite(periodDA) ||
        !isfinite(periodCommission) ||
        !isfinite(periodHRA)
    ) {
        throw overflow_error(
            "Combined HRA-period amounts are too large."
        );
    }

    if (
        periodBasic > basicSalary + AMOUNT_TOLERANCE ||
        periodDA > dearnessAllowance + AMOUNT_TOLERANCE ||
        periodCommission > commission + AMOUNT_TOLERANCE ||
        periodHRA > houseRentAllowance + AMOUNT_TOLERANCE
    ) {
        throw invalid_argument(
            "HRA-period totals cannot exceed their annual "
            "salary components."
        );
    }

    if (
        ltaExemptionClaimed >
        leaveTravelAllowance + AMOUNT_TOLERANCE
    ) {
        throw invalid_argument(
            "LTA exemption claimed cannot exceed LTA received."
        );
    }

    IncomeComputationResult result;

    result.head = getHead();
    result.description = getDescription();
    result.grossAmount = getGrossSalary();

    addBreakdown(result, "Basic salary", basicSalary);
    addBreakdown(result, "Dearness allowance", dearnessAllowance);
    addBreakdown(result, "Bonus", bonus);
    addBreakdown(result, "Commission", commission);

    addBreakdown(
        result,
        "House Rent Allowance received",
        houseRentAllowance
    );

    addBreakdown(
        result,
        "Leave Travel Allowance received",
        leaveTravelAllowance
    );

    addBreakdown(
        result,
        "Other taxable allowances",
        otherTaxableAllowances
    );

    addBreakdown(
        result,
        "Taxable perquisites",
        taxablePerquisites
    );

    addBreakdown(result, "Gross salary", result.grossAmount);

    // --------------------------------------------------------
    // Allowance exemptions
    // --------------------------------------------------------

    const double hraExemption =
        calculateHRAExemption(rules);

    const bool ltaAllowed =
        rules.getBoolean("LTA_EXEMPTION_ALLOWED");

    const double ltaExemption =
        ltaAllowed
            ? min(ltaExemptionClaimed, leaveTravelAllowance)
            : 0.0;

    result.exemptAmount = hraExemption + ltaExemption;

    addBreakdown(
        result,
        "Less: HRA exemption",
        -hraExemption
    );

    addBreakdown(
        result,
        "Less: LTA exemption",
        -ltaExemption
    );

    if (
        !rules.getBoolean("HRA_EXEMPTION_ALLOWED") &&
        houseRentAllowance > 0.0
    ) {
        result.notes.push_back(
            "HRA exemption is not available under this regime."
        );
    }

    if (
        rules.getBoolean("HRA_EXEMPTION_ALLOWED") &&
        houseRentAllowance > 0.0 &&
        hraPeriods.empty()
    ) {
        result.notes.push_back(
            "No HRA exemption was calculated because no "
            "eligible rental-period details were entered."
        );
    }

    if (ltaExemptionClaimed > 0.0) {
        result.notes.push_back(
            ltaAllowed
                ? "LTA eligibility is based on the user's "
                  "declared eligible claim. Travel and block-year "
                  "conditions are not independently verified."
                : "The entered LTA exemption claim is not "
                  "allowed under this regime."
        );
    }

    const double salaryAfterExemptions = max(
        0.0,
        result.grossAmount - result.exemptAmount
    );

    // --------------------------------------------------------
    // Standard deduction
    // --------------------------------------------------------

    const DeductionRule& standardRule =
        rules.getDeductionRule("STANDARD_SALARY");

    double standardDeduction = 0.0;

    if (standardRule.allowed) {
        if (!standardRule.hasMaximumAmount) {
            throw logic_error(
                "Salary standard deduction requires a configured cap."
            );
        }

        standardDeduction = min(
            salaryAfterExemptions,
            standardRule.maximumAmount
        );
    }

    addAdjustment(
        result,
        "STANDARD_SALARY",
        "Salary standard deduction",
        standardDeduction,
        standardDeduction,
        "Automatically calculated once for combined annual salary."
    );

    addBreakdown(
        result,
        "Less: Salary standard deduction",
        -standardDeduction
    );

    // --------------------------------------------------------
    // Professional tax
    // --------------------------------------------------------

    const DeductionRule& professionalRule =
        rules.getDeductionRule("PROFESSIONAL_TAX");

    double professionalDeduction = 0.0;

    if (professionalRule.allowed) {
        professionalDeduction = professionalTaxPaid;

        if (professionalRule.hasMaximumAmount) {
            professionalDeduction = min(
                professionalDeduction,
                professionalRule.maximumAmount
            );
        }
    }

    // The supported salary model does not create a salary loss.
    professionalDeduction = min(
        professionalDeduction,
        max(
            0.0,
            salaryAfterExemptions - standardDeduction
        )
    );

    addAdjustment(
        result,
        "PROFESSIONAL_TAX",
        "Professional tax paid",
        professionalTaxPaid,
        professionalDeduction,
        professionalRule.allowed
            ? "Allowed within this salary computation."
            : "Not allowed under this regime."
    );

    addBreakdown(
        result,
        "Less: Professional tax",
        -professionalDeduction
    );

    // --------------------------------------------------------
    // Final salary contribution
    // --------------------------------------------------------

    result.normalRateIncome = max(
        0.0,
        salaryAfterExemptions -
            standardDeduction -
            professionalDeduction
    );

    addBreakdown(
        result,
        "Taxable salary income",
        result.normalRateIncome
    );

    result.notes.push_back(
        "Salary adjustments above are already included in taxable "
        "salary and must not be deducted again after GTI."
    );

    return result;
}