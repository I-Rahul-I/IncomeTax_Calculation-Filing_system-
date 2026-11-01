#ifndef INCOME_H
#define INCOME_H


#include <string>
#include <vector>


#include "Taxpayer.h"
#include "TaxRules.h"


using namespace std;


// Tax regime selected for the annual computation.
enum class TaxRegime {
    Old,
    New
};


// Major income heads supported by the project.
enum class IncomeHead {
    Salary,
    HouseProperty,
    BusinessProfession,
    CapitalGains,
    OtherSources,


    // Informational reporting category, not a taxable head.
    ExemptIncome
};


// One adjustment inside an income-head calculation.
struct IncomeAdjustment {
    string code;
    string description;

    double claimedAmount = 0.0;
    double allowedAmount = 0.0;

    string explanation;
};


// A taxable component requiring special treatment.
struct SpecialIncomeComponent {
    string ruleCode;

    string description;

    double amount = 0.0;

    string relevantDate;
};


// One explanatory item for the console/PDF computation sheet.
struct IncomeBreakdownItem {
    string description;
    double amount = 0.0;
};


// Result from evaluating one income entry under one regime.
struct IncomeComputationResult {
    IncomeHead head = IncomeHead::OtherSources;

    string description;

    double grossAmount = 0.0;

    double exemptAmount = 0.0;

    double normalRateIncome = 0.0;

    vector<SpecialIncomeComponent> specialRateComponents;

    vector<IncomeAdjustment> adjustments;

    vector<IncomeBreakdownItem> breakdown;

    vector<string> notes;

    double getSpecialRateIncomeTotal() const {
        double total = 0.0;

        for (const SpecialIncomeComponent& component :
             specialRateComponents) {
            total += component.amount;
        }

        return total;
    }

    double getIncomeBeforeAggregationAdjustments() const {
        return normalRateIncome + getSpecialRateIncomeTotal();
    }

    double getAllowedHeadAdjustmentsTotal() const {
        double total = 0.0;

        for (const IncomeAdjustment& adjustment : adjustments) {
            total += adjustment.allowedAmount;
        }

        return total;
    }
};


// Abstract base for income entries.
class Income {
private:
    string description;

protected:
    explicit Income(const string& description)
        : description(description) {
    }

public:
    virtual ~Income() = default;

    const string& getDescription() const {
        return description;
    }

    virtual IncomeHead getHead() const = 0;

    virtual string getHeadName() const = 0;

    virtual IncomeComputationResult evaluate(
        const Taxpayer& taxpayer,
        const TaxRules& rules
    ) const = 0;
};


#endif