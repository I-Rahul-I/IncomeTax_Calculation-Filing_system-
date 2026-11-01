#ifndef SALARY_INCOME_H
#define SALARY_INCOME_H

#include <string>
#include <vector>

#include "Income.h"

using namespace std;

// One period used to calculate an HRA exemption.
//
// Multiple periods allow for changes in rent, salary,
// HRA received, or city classification during the year.
//
// All amounts belong to this period, not automatically
// to the entire financial year.
struct HRAPeriod {
    string description;

    // Dates use YYYY-MM-DD.
    string startDate;
    string endDate;

    double basicSalary = 0.0;

    // Only DA forming part of retirement benefits.
    double eligibleDearnessAllowance = 0.0;

    // Only commission qualifying for the HRA salary basis.
    // Do not enter every kind of commission here.
    double eligibleTurnoverCommission = 0.0;

    double hraReceived = 0.0;
    double rentPaid = 0.0;

    // Whether the location qualifies for the higher HRA
    // city percentage under the configured rules.
    bool eligibleMetroCity = false;
};

// Combined annual salary entry.
//
// The original user inputs remain unchanged when this object
// is evaluated separately under Old and New Regime rules.
class SalaryIncome : public Income {
private:
    double basicSalary;
    double dearnessAllowance;

    double bonus;
    double commission;

    double houseRentAllowance;
    double leaveTravelAllowance;

    double otherTaxableAllowances;
    double taxablePerquisites;

    // Taxpayer's claimed eligible LTA amount.
    // The final allowed amount is determined during evaluation.
    double ltaExemptionClaimed;

    double professionalTaxPaid;

    // HRA details used only when an exemption is claimed.
    vector<HRAPeriod> hraPeriods;

    // Validate monetary values before storing them.
    static void validateAmount(
        double amount,
        const string& fieldName
    );

    static void validateHRAPeriod(
        const HRAPeriod& period
    );

    // Calculate an HRA exemption from the provided periods.
    // The returned total must also respect annual HRA received.
    double calculateHRAExemption(
        const TaxRules& rules
    ) const;

public:
    explicit SalaryIncome(
        const string& description = "Annual Salary Income"
    );

    // ========================================================
    // Salary component setters
    // ========================================================

    void setBasicSalary(double amount);

    void setDearnessAllowance(double amount);

    void setBonus(double amount);

    void setCommission(double amount);

    void setHouseRentAllowance(double amount);

    void setLeaveTravelAllowance(double amount);

    void setOtherTaxableAllowances(double amount);

    void setTaxablePerquisites(double amount);

    void setLTAExemptionClaimed(double amount);

    void setProfessionalTaxPaid(double amount);

    // ========================================================
    // HRA period management
    // ========================================================

    void addHRAPeriod(const HRAPeriod& period);

    void clearHRAPeriods();

    const vector<HRAPeriod>& getHRAPeriods() const;

    // ========================================================
    // Salary component getters
    // ========================================================

    double getBasicSalary() const;

    double getDearnessAllowance() const;

    double getBonus() const;

    double getCommission() const;

    double getHouseRentAllowance() const;

    double getLeaveTravelAllowance() const;

    double getOtherTaxableAllowances() const;

    double getTaxablePerquisites() const;

    double getLTAExemptionClaimed() const;

    double getProfessionalTaxPaid() const;

    // Sum of all entered annual salary components before
    // the supported exemptions and head-level deductions.
    double getGrossSalary() const;

    // ========================================================
    // Income interface
    // ========================================================

    IncomeHead getHead() const override;

    string getHeadName() const override;

    IncomeComputationResult evaluate(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const override;
};

#endif