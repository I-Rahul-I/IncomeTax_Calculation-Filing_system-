#ifndef TAX_RULES_H
#define TAX_RULES_H

#include <map>
#include <string>
#include <vector>

using namespace std;

// Identifies the regime configuration being loaded.
enum class RegimeType {
    Old,
    New
};

// One band in a progressive slab table.
//
// Amounts are expressed in rupees.
// Rates are expressed as percentages:
// for example, 5 means 5%, not 0.05.
//
// The taxable portion of a band is the amount above lowerLimit,
// capped at upperLimit.
//
// When hasUpperLimit is false, the band has no upper limit.
struct TaxSlab {
    double lowerLimit = 0.0;
    double upperLimit = 0.0;
    double ratePercent = 0.0;

    bool hasUpperLimit = true;
};

// A named slab table.
//
// Examples of profile names we may configure:
// INDIVIDUAL_RESIDENT_BELOW_60
// INDIVIDUAL_RESIDENT_60_TO_79
// INDIVIDUAL_RESIDENT_80_PLUS
// INDIVIDUAL_NON_RESIDENT
// HUF
//
// A profile is selected explicitly by the calculation module.
// Missing profiles must not silently fall back to another category.
struct TaxSlabProfile {
    string name;

    vector<TaxSlab> slabs;
};

// Rule for one supported deduction.
//
// This describes eligibility and a monetary ceiling only.
// Section-specific calculations and combined limits will be
// handled separately by the deduction/calculation modules.
struct DeductionRule {
    string code;

    bool allowed = false;

    bool hasMaximumAmount = false;
    double maximumAmount = 0.0;
};

// A configured special-income rule.
//
// The threshold, when present, is an annual aggregate threshold,
// not an allowance to apply independently to every transaction.
//
// Eligibility, rate selection, and other legal conditions belong
// in the relevant income/calculation module.
struct SpecialIncomeRule {
    string code;

    double ratePercent = 0.0;

    bool hasAnnualThreshold = false;
    double annualThreshold = 0.0;
};

// One advance-tax checkpoint.
//
// A cumulative target means payments made on or before this
// due date are compared with the required cumulative amount.
struct AdvanceTaxCheckpoint {
    int installmentNumber = 0;

    string dueDate;

    double cumulativePercent = 0.0;
};

class TaxRules {
private:
    RegimeType regime;

    string financialYear;
    string assessmentYear;

    // Additional settings:
    // rebate limits, cess, interest rates, rounding settings,
    // applicability flags, and other configured parameters.
    map<string, string> settings;

    map<string, TaxSlabProfile> slabProfiles;

    map<string, DeductionRule> deductionRules;

    map<string, SpecialIncomeRule> specialIncomeRules;

    vector<AdvanceTaxCheckpoint> advanceTaxCheckpoints;

    bool loaded;

    // Remove all previously loaded values before a new load.
    void reset();

    // Read and parse one configuration file.
    // isCommonFile distinguishes shared rules from regime rules.
    void loadFile(
        const string& filePath,
        bool isCommonFile
    );

    // Parse one non-empty, non-comment configuration entry.
    void parseEntry(
        const string& key,
        const string& value,
        const string& filePath,
        int lineNumber,
        bool isCommonFile
    );

    // Validate the loaded structure.
    // This checks configuration consistency, not legal correctness.
    void validate() const;

    static string trim(const string& value);

    static vector<string> split(
        const string& value,
        char delimiter
    );

    static double parseNumber(
        const string& value,
        const string& context
    );

    static int parseInteger(
        const string& value,
        const string& context
    );

    static bool parseBoolean(
        const string& value,
        const string& context
    );

public:
    explicit TaxRules(RegimeType regime = RegimeType::New);

    // Load shared settings first, then regime-specific rules.
    //
    // Invalid, missing, or duplicate entries will cause an
    // exception instead of silently producing a zero-tax result.
    //
    // Required settings must be present before the loader marks
    // the rules as ready for use.
    void load(
        const string& commonFilePath,
        const string& regimeFilePath
    );

    bool isLoaded() const;

    RegimeType getRegimeType() const;

    string getRegimeName() const;

    const string& getFinancialYear() const;

    const string& getAssessmentYear() const;

    // ========================================================
    // General configuration settings
    // ========================================================

    bool hasSetting(const string& key) const;

    // Required getters throw if a key is missing or malformed.
    string getString(const string& key) const;

    double getNumber(const string& key) const;

    int getInteger(const string& key) const;

    bool getBoolean(const string& key) const;

    // Optional getters use the supplied default only when the
    // key is absent. A malformed existing value still throws.
    string getStringOrDefault(
        const string& key,
        const string& defaultValue
    ) const;

    double getNumberOrDefault(
        const string& key,
        double defaultValue
    ) const;

    int getIntegerOrDefault(
        const string& key,
        int defaultValue
    ) const;

    bool getBooleanOrDefault(
        const string& key,
        bool defaultValue
    ) const;

    // ========================================================
    // Slab profiles
    // ========================================================

    bool hasSlabProfile(
        const string& profileName
    ) const;

    const TaxSlabProfile& getSlabProfile(
        const string& profileName
    ) const;

    // ========================================================
    // Deduction rules
    // ========================================================

    bool hasDeductionRule(
        const string& code
    ) const;

    const DeductionRule& getDeductionRule(
        const string& code
    ) const;

    // ========================================================
    // Special-income rules
    // ========================================================

    bool hasSpecialIncomeRule(
        const string& code
    ) const;

    const SpecialIncomeRule& getSpecialIncomeRule(
        const string& code
    ) const;

    // ========================================================
    // Advance-tax checkpoints
    // ========================================================

    const vector<AdvanceTaxCheckpoint>&
    getAdvanceTaxCheckpoints() const;
};

#endif