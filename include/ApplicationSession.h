#ifndef APPLICATION_SESSION_H
#define APPLICATION_SESSION_H
#include <memory>
#include <string>
#include <vector>
#include "Taxpayer.h"
#include "TaxRules.h"
#include "Income.h"
#include "Deductions.h"
#include "Payments.h"
#include "TaxCalculator.h"
#include "TaxComputationResult.h"
#include "ReportGenerator.h"
using namespace std;
// Current state of the annual session.
enum class SessionState {Empty = 1,RulesLoaded,TaxpayerSelected,IncomeEntered,CalculationCompleted};
// Owns all data for one annual tax-computation session.
// The session enforces a simple workflow:
// 1. Load rules.
// 2. Select or update the taxpayer.
// 3. Add income heads.
// 4. Add deductions and payment credits.
// 5. Calculate.
// 6. Generate a report.
// It does not retain data after the application closes.
class ApplicationSession {
private:
    unique_ptr<Taxpayer> taxpayer;
    TaxRules rules;
    vector<unique_ptr<Income>> incomeHeads;
    Deductions deductions;
    Payments payments;
    TaxComputationResult latestResult;
    SessionState state = SessionState::Empty;
    bool hasCalculationResult = false;
    void requireRulesLoaded() const;
    void requireTaxpayerSelected() const;
    void requireCalculationCompleted() const;
    static void validateIncomeHeadNotPresent(
        IncomeHead head
    );
public:
    ApplicationSession() = default;
    // Non-copyable because it owns income-head objects.
    ApplicationSession(
        const ApplicationSession&
    ) = delete;
    ApplicationSession& operator=(
        const ApplicationSession&
    ) = delete;
    // Movable is allowed for internal ownership transfer.
    ApplicationSession(ApplicationSession&&) = default;
    ApplicationSession& operator=(
        ApplicationSession&&
    ) = default;
    // ========================================================
    // Session control
    // ========================================================
    SessionState getState() const;
    bool hasResult() const;
    // Clears every entered value and returns the session
    // to its initial empty state.
    void reset();
    // ========================================================
    // Rules
    // ========================================================
    // Loads the common configuration file.
    void loadRules(
        const string& commonConfigPath,
        const string& regimeConfigPath,
        RegimeType regimeType = RegimeType::New
    );
    const TaxRules& getRules() const;
    // ========================================================
    // Taxpayer
    // ========================================================
    void setTaxpayer(
        unique_ptr<Taxpayer> value
    );
    const Taxpayer& getTaxpayer() const;
    // ========================================================
    // Income heads
    // ========================================================
    // Adds an income head owned by the session.
    // Only one object per income head is permitted.
    void addIncomeHead(
        unique_ptr<Income> incomeHead
    );
    bool hasIncomeHead(
        IncomeHead head
    ) const;
    Income* getIncomeHead(
        IncomeHead head
    );
    const Income* getIncomeHead(
        IncomeHead head
    ) const;
    // Removes one income head.
    void removeIncomeHead(
        IncomeHead head
    );
    // Removes every income head.
    void clearIncomeHeads();
    size_t getIncomeHeadCount() const;
    // ========================================================
    // Deductions and payments
    // ========================================================
    Deductions& getDeductions();
    const Deductions& getDeductions() const;
    Payments& getPayments();
    const Payments& getPayments() const;
    // ========================================================
    // Calculation and report
    // ========================================================
    // Runs the complete annual calculation.
    //
    // A successful calculation replaces the previous result.
    const TaxComputationResult& calculate();
    const TaxComputationResult& getLatestResult() const;
    // Generates a report from the latest result.
    string generateReport(
        ReportFormat format = ReportFormat::PlainText,
        const ReportOptions& options = ReportOptions()
    ) const;
};
#endif