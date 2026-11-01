#include "ApplicationSession.h"

#include <algorithm>
#include <stdexcept>

using namespace std;


// ============================================================
// Session-state guards
// ============================================================


void ApplicationSession::requireRulesLoaded() const {
    if (state == SessionState::Empty) {
        throw logic_error(
            "Load the tax-rule configuration before "
            "continuing."
        );
    }

    if (!rules.isLoaded()) {
        throw logic_error(
            "Tax rules are not loaded."
        );
    }
}


void ApplicationSession::requireTaxpayerSelected() const {
    requireRulesLoaded();

    if (!taxpayer) {
        throw logic_error(
            "Select a taxpayer before continuing."
        );
    }

    if (
        state != SessionState::TaxpayerSelected &&
        state != SessionState::IncomeEntered &&
        state != SessionState::CalculationCompleted
    ) {
        throw logic_error(
            "Select a taxpayer before continuing."
        );
    }
}


void ApplicationSession::requireCalculationCompleted() const {
    if (!hasCalculationResult) {
        throw logic_error(
            "Run the calculation before generating a report."
        );
    }
}


// ============================================================
// Income-head validation
// ============================================================


void ApplicationSession::validateIncomeHeadNotPresent(
    IncomeHead head
) {
    switch (head) {
        case IncomeHead::Salary:
        case IncomeHead::HouseProperty:
        case IncomeHead::BusinessProfession:
        case IncomeHead::CapitalGains:
        case IncomeHead::OtherSources:
        case IncomeHead::ExemptIncome:
            return;

        default:
            throw invalid_argument(
                "Invalid income head."
            );
    }
}


// ============================================================
// Session control
// ============================================================


SessionState ApplicationSession::getState() const {
    return state;
}


bool ApplicationSession::hasResult() const {
    return hasCalculationResult;
}


void ApplicationSession::reset() {
    taxpayer.reset();

    rules = TaxRules(RegimeType::New);

    incomeHeads.clear();

    deductions = Deductions();

    payments = Payments();

    latestResult = TaxComputationResult();

    state = SessionState::Empty;

    hasCalculationResult = false;
}


// ============================================================
// Rules
// ============================================================


void ApplicationSession::loadRules(
    const string& commonConfigPath,
    const string& regimeConfigPath,
    RegimeType regimeType
) {
    // Replace only the regime rules. Keep the taxpayer, entered income,
    // deductions, and credits so the same inputs can be compared.
    rules = TaxRules(regimeType);
    rules.load(commonConfigPath, regimeConfigPath);

    if (taxpayer) {
        state = incomeHeads.empty()
            ? SessionState::TaxpayerSelected
            : SessionState::IncomeEntered;
    } else {
        state = SessionState::RulesLoaded;
    }

    hasCalculationResult = false;
}


const TaxRules& ApplicationSession::getRules() const {
    return rules;
}


// ============================================================
// Taxpayer
// ============================================================


void ApplicationSession::setTaxpayer(
    unique_ptr<Taxpayer> value
) {
    requireRulesLoaded();

    if (value == nullptr) {
        throw invalid_argument(
            "Taxpayer object cannot be null."
        );
    }

    // Validate the taxpayer against the loaded configuration.
    string supportKey;

    switch (value->getType()) {
        case TaxpayerType::Individual:
            supportKey = "SUPPORT_INDIVIDUAL";
            break;

        case TaxpayerType::HUF:
            supportKey = "SUPPORT_HUF";
            break;

        default:
            throw invalid_argument(
                "The current session supports Individual and "
                "HUF taxpayers only."
            );
    }

    if (!rules.getBoolean(supportKey)) {
        throw invalid_argument(
            "Selected taxpayer category is disabled in the "
            "current configuration."
        );
    }

    taxpayer = move(value);

    state = SessionState::TaxpayerSelected;

    hasCalculationResult = false;
}


const Taxpayer& ApplicationSession::getTaxpayer() const {
    requireTaxpayerSelected();

    return *taxpayer;
}


// ============================================================
// Income heads
// ============================================================


void ApplicationSession::addIncomeHead(
    unique_ptr<Income> incomeHead
) {
    requireTaxpayerSelected();

    if (incomeHead == nullptr) {
        throw invalid_argument(
            "Income-head object cannot be null."
        );
    }

    const IncomeHead head = incomeHead->getHead();

    validateIncomeHeadNotPresent(head);

    for (const unique_ptr<Income>& existing : incomeHeads) {
        if (existing->getHead() == head) {
            throw invalid_argument(
                "An income head of this type has already "
                "been added: " + existing->getHeadName()
            );
        }
    }

    incomeHeads.push_back(move(incomeHead));

    state = SessionState::IncomeEntered;

    hasCalculationResult = false;
}


bool ApplicationSession::hasIncomeHead(
    IncomeHead head
) const {
    validateIncomeHeadNotPresent(head);

    for (const unique_ptr<Income>& income : incomeHeads) {
        if (income->getHead() == head) {
            return true;
        }
    }

    return false;
}


Income* ApplicationSession::getIncomeHead(
    IncomeHead head
) {
    validateIncomeHeadNotPresent(head);

    for (unique_ptr<Income>& income : incomeHeads) {
        if (income->getHead() == head) {
            return income.get();
        }
    }

    return nullptr;
}


const Income* ApplicationSession::getIncomeHead(
    IncomeHead head
) const {
    validateIncomeHeadNotPresent(head);

    for (const unique_ptr<Income>& income : incomeHeads) {
        if (income->getHead() == head) {
            return income.get();
        }
    }

    return nullptr;
}


void ApplicationSession::removeIncomeHead(
    IncomeHead head
) {
    requireTaxpayerSelected();

    validateIncomeHeadNotPresent(head);

    auto it = remove_if(
        incomeHeads.begin(),
        incomeHeads.end(),
        [head](const unique_ptr<Income>& income) {
            return income->getHead() == head;
        }
    );

    if (it == incomeHeads.end()) {
        throw invalid_argument(
            "No income head of the selected type exists."
        );
    }

    incomeHeads.erase(it, incomeHeads.end());

    state = SessionState::IncomeEntered;

    hasCalculationResult = false;
}


void ApplicationSession::clearIncomeHeads() {
    requireTaxpayerSelected();

    incomeHeads.clear();

    state = SessionState::TaxpayerSelected;

    hasCalculationResult = false;
}


size_t ApplicationSession::getIncomeHeadCount() const {
    return incomeHeads.size();
}


// ============================================================
// Deductions and payments
// ============================================================


Deductions& ApplicationSession::getDeductions() {
    return deductions;
}


const Deductions& ApplicationSession::getDeductions() const {
    return deductions;
}


Payments& ApplicationSession::getPayments() {
    return payments;
}


const Payments& ApplicationSession::getPayments() const {
    return payments;
}


// ============================================================
// Calculation and report
// ============================================================


const TaxComputationResult& ApplicationSession::calculate() {
    requireTaxpayerSelected();

    if (incomeHeads.empty()) {
        throw invalid_argument(
            "Add at least one income head before calculating."
        );
    }

    TaxCalculationInput input;

    input.taxpayer = taxpayer.get();
    input.rules = rules;
    input.deductions = deductions;
    input.payments = payments;

    for (unique_ptr<Income>& income : incomeHeads) {
        input.incomeHeads.push_back(income.get());
    }

    TaxCalculator calculator;

    latestResult = calculator.calculate(input);

    state = SessionState::CalculationCompleted;

    hasCalculationResult = true;

    return latestResult;
}


const TaxComputationResult&
ApplicationSession::getLatestResult() const {
    requireCalculationCompleted();

    return latestResult;
}


string ApplicationSession::generateReport(
    ReportFormat format,
    const ReportOptions& options
) const {
    requireCalculationCompleted();

    ReportGenerator generator;

    string report = generator.generateReport(
        latestResult,
        payments,
        format,
        options
    );

    // Include the basic return-header particulars shown in the sample
    // computation format. Identity remains session-only.
    string header =
        "Taxpayer Name            : " + taxpayer->getName() + "\n" +
        "Financial Year           : " + Taxpayer::getFinancialYear() + "\n" +
        "Assessment Year          : " + Taxpayer::getAssessmentYear() + "\n" +
        "Taxpayer Category        : " + taxpayer->getCategoryName() + "\n" +
        "Residential Status       : " + residentialStatusToString(taxpayer->getResidentialStatus()) + "\n" +
        "PAN                      : " + (taxpayer->getPAN().empty() ? "Not provided" : taxpayer->getPAN()) + "\n";
    if (const auto* individual = dynamic_cast<const IndividualTaxpayer*>(taxpayer.get())) {
        header += "Age Category             : " + individual->getAgeCategoryName() + "\n";
    }
    header += "\n";
    return header + report;
}