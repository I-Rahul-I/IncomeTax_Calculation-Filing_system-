#include "TaxRules.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {

bool startsWith(
    const string& text,
    const string& prefix
) {
    return text.compare(0, prefix.size(), prefix) == 0;
}

string upperCase(string value) {
    for (char& character : value) {
        if (character >= 'a' && character <= 'z') {
            character = static_cast<char>(
                character - 'a' + 'A'
            );
        }
    }

    return value;
}

bool approximatelyEqual(double first, double second) {
    return fabs(first - second) <= 0.000001;
}

bool validDate(const string& value) {
    // Required format: YYYY-MM-DD.
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

    int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    const bool leapYear =
        (year % 400 == 0) ||
        (year % 4 == 0 && year % 100 != 0);

    if (leapYear) {
        daysInMonth[1] = 29;
    }

    return day >= 1 && day <= daysInMonth[month - 1];
}

void requireFieldCount(
    const vector<string>& fields,
    size_t expected,
    const string& key
) {
    if (fields.size() != expected) {
        throw invalid_argument(
            key + " requires " +
            to_string(expected) +
            " comma-separated fields."
        );
    }
}

} // namespace

// ============================================================
// Construction and state
// ============================================================

TaxRules::TaxRules(RegimeType regime)
    : regime(regime),
      loaded(false) {

    if (
        regime != RegimeType::Old &&
        regime != RegimeType::New
    ) {
        throw invalid_argument("Invalid regime type.");
    }
}

void TaxRules::reset() {
    financialYear.clear();
    assessmentYear.clear();

    settings.clear();
    slabProfiles.clear();
    deductionRules.clear();
    specialIncomeRules.clear();
    advanceTaxCheckpoints.clear();

    loaded = false;
}

void TaxRules::load(
    const string& commonFilePath,
    const string& regimeFilePath
) {
    reset();

    try {
        loadFile(commonFilePath, true);
        loadFile(regimeFilePath, false);

        financialYear = getString("FINANCIAL_YEAR");
        assessmentYear = getString("ASSESSMENT_YEAR");

        validate();

        loaded = true;
    }
    catch (...) {
        // Do not leave a partially loaded configuration available.
        reset();
        throw;
    }
}

bool TaxRules::isLoaded() const {
    return loaded;
}

RegimeType TaxRules::getRegimeType() const {
    return regime;
}

string TaxRules::getRegimeName() const {
    return regime == RegimeType::Old
        ? "Old Tax Regime"
        : "New Tax Regime";
}

const string& TaxRules::getFinancialYear() const {
    return financialYear;
}

const string& TaxRules::getAssessmentYear() const {
    return assessmentYear;
}

// ============================================================
// Parsing helpers
// ============================================================

string TaxRules::trim(const string& value) {
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

vector<string> TaxRules::split(
    const string& value,
    char delimiter
) {
    vector<string> fields;
    size_t start = 0;

    while (true) {
        const size_t position =
            value.find(delimiter, start);

        if (position == string::npos) {
            fields.push_back(trim(value.substr(start)));
            break;
        }

        fields.push_back(
            trim(value.substr(start, position - start))
        );

        start = position + 1;
    }

    return fields;
}

double TaxRules::parseNumber(
    const string& value,
    const string& context
) {
    istringstream parser(value);

    double result;
    char extra;

    if (
        !(parser >> result) ||
        (parser >> extra) ||
        !isfinite(result)
    ) {
        throw invalid_argument(
            "Invalid number for " + context + ": " + value
        );
    }

    return result;
}

int TaxRules::parseInteger(
    const string& value,
    const string& context
) {
    istringstream parser(value);

    int result;
    char extra;

    if (
        !(parser >> result) ||
        (parser >> extra)
    ) {
        throw invalid_argument(
            "Invalid integer for " + context + ": " + value
        );
    }

    return result;
}

bool TaxRules::parseBoolean(
    const string& value,
    const string& context
) {
    const string normalized = upperCase(trim(value));

    if (
        normalized == "TRUE" ||
        normalized == "YES" ||
        normalized == "1"
    ) {
        return true;
    }

    if (
        normalized == "FALSE" ||
        normalized == "NO" ||
        normalized == "0"
    ) {
        return false;
    }

    throw invalid_argument(
        "Invalid boolean for " + context + ": " + value
    );
}

// ============================================================
// Read configuration files
// ============================================================

void TaxRules::loadFile(
    const string& filePath,
    bool isCommonFile
) {
    ifstream file(filePath);

    if (!file.is_open()) {
        throw runtime_error(
            "Cannot open tax configuration file: " + filePath
        );
    }

    string line;
    int lineNumber = 0;

    while (getline(file, line)) {
        ++lineNumber;

        // Accept UTF-8 files with a byte-order mark.
        if (
            lineNumber == 1 &&
            line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF
        ) {
            line.erase(0, 3);
        }

        // Everything after '#' is a comment.
        const size_t commentPosition = line.find('#');

        if (commentPosition != string::npos) {
            line.erase(commentPosition);
        }

        line = trim(line);

        if (line.empty()) {
            continue;
        }

        const size_t separator = line.find('=');

        if (separator == string::npos) {
            throw runtime_error(
                filePath + ":" + to_string(lineNumber) +
                ": Expected KEY=VALUE."
            );
        }

        const string key =
            trim(line.substr(0, separator));

        const string value =
            trim(line.substr(separator + 1));

        if (key.empty() || value.empty()) {
            throw runtime_error(
                filePath + ":" + to_string(lineNumber) +
                ": Key and value must not be empty."
            );
        }

        try {
            parseEntry(
                key,
                value,
                filePath,
                lineNumber,
                isCommonFile
            );
        }
        catch (const exception& error) {
            throw runtime_error(
                filePath + ":" + to_string(lineNumber) +
                ": " + error.what()
            );
        }
    }

    if (file.bad()) {
        throw runtime_error(
            "Error reading configuration file: " + filePath
        );
    }
}

void TaxRules::parseEntry(
    const string& key,
    const string& value,
    const string& filePath,
    int lineNumber,
    bool isCommonFile
) {
    // File location is added by loadFile() when reporting errors.
    (void)filePath;
    (void)lineNumber;

    // --------------------------------------------------------
    // SLAB.PROFILE=LOWER,UPPER,RATE
    // Multiple entries form one profile.
    // --------------------------------------------------------

    if (startsWith(key, "SLAB.")) {
        if (isCommonFile) {
            throw invalid_argument(
                "Slab profiles belong in a regime file."
            );
        }

        const string profileName = key.substr(5);

        if (profileName.empty()) {
            throw invalid_argument("Slab profile name is empty.");
        }

        const vector<string> fields = split(value, ',');
        requireFieldCount(fields, 3, key);

        TaxSlab slab;

        slab.lowerLimit = parseNumber(fields[0], key);
        slab.ratePercent = parseNumber(fields[2], key);

        slab.hasUpperLimit =
            upperCase(fields[1]) != "INF";

        if (slab.hasUpperLimit) {
            slab.upperLimit = parseNumber(fields[1], key);
        }

        TaxSlabProfile& profile = slabProfiles[profileName];

        profile.name = profileName;
        profile.slabs.push_back(slab);

        return;
    }

    // --------------------------------------------------------
    // DEDUCTION.CODE=TRUE_OR_FALSE,MAXIMUM_OR_NONE
    // --------------------------------------------------------

    if (startsWith(key, "DEDUCTION.")) {
        if (isCommonFile) {
            throw invalid_argument(
                "Deduction eligibility belongs in a regime file."
            );
        }

        const string code = key.substr(10);

        if (code.empty()) {
            throw invalid_argument("Deduction code is empty.");
        }

        if (hasDeductionRule(code)) {
            throw invalid_argument(
                "Duplicate deduction rule: " + code
            );
        }

        const vector<string> fields = split(value, ',');
        requireFieldCount(fields, 2, key);

        DeductionRule rule;

        rule.code = code;
        rule.allowed = parseBoolean(fields[0], key);

        rule.hasMaximumAmount =
            upperCase(fields[1]) != "NONE";

        if (rule.hasMaximumAmount) {
            rule.maximumAmount = parseNumber(fields[1], key);
        }

        deductionRules.emplace(code, rule);

        return;
    }

    // --------------------------------------------------------
    // SPECIAL.CODE=RATE,ANNUAL_THRESHOLD_OR_NONE
    // --------------------------------------------------------

    if (startsWith(key, "SPECIAL.")) {
        const string code = key.substr(8);

        if (code.empty()) {
            throw invalid_argument("Special-income code is empty.");
        }

        if (hasSpecialIncomeRule(code)) {
            throw invalid_argument(
                "Duplicate special-income rule: " + code
            );
        }

        const vector<string> fields = split(value, ',');
        requireFieldCount(fields, 2, key);

        SpecialIncomeRule rule;

        rule.code = code;
        rule.ratePercent = parseNumber(fields[0], key);

        rule.hasAnnualThreshold =
            upperCase(fields[1]) != "NONE";

        if (rule.hasAnnualThreshold) {
            rule.annualThreshold = parseNumber(fields[1], key);
        }

        specialIncomeRules.emplace(code, rule);

        return;
    }

    // --------------------------------------------------------
    // ADVANCE.NUMBER=YYYY-MM-DD,CUMULATIVE_PERCENT
    // --------------------------------------------------------

    if (startsWith(key, "ADVANCE.")) {
        if (!isCommonFile) {
            throw invalid_argument(
                "Advance-tax checkpoints belong in the common file."
            );
        }

        AdvanceTaxCheckpoint checkpoint;

        checkpoint.installmentNumber =
            parseInteger(key.substr(8), key);

        const vector<string> fields = split(value, ',');
        requireFieldCount(fields, 2, key);

        checkpoint.dueDate = fields[0];
        checkpoint.cumulativePercent =
            parseNumber(fields[1], key);

        for (const AdvanceTaxCheckpoint& existing :
             advanceTaxCheckpoints) {
            if (
                existing.installmentNumber ==
                checkpoint.installmentNumber
            ) {
                throw invalid_argument(
                    "Duplicate advance-tax checkpoint: " + key
                );
            }
        }

        advanceTaxCheckpoints.push_back(checkpoint);

        sort(
            advanceTaxCheckpoints.begin(),
            advanceTaxCheckpoints.end(),
            [](const AdvanceTaxCheckpoint& first,
               const AdvanceTaxCheckpoint& second) {
                return first.installmentNumber <
                       second.installmentNumber;
            }
        );

        return;
    }

    // --------------------------------------------------------
    // Ordinary named settings
    // --------------------------------------------------------

    if (key == "REGIME" && isCommonFile) {
        throw invalid_argument(
            "REGIME belongs in a regime-specific file."
        );
    }

    if (
        (key == "FINANCIAL_YEAR" || key == "ASSESSMENT_YEAR") &&
        !isCommonFile
    ) {
        throw invalid_argument(
            key + " belongs in the common file."
        );
    }

    if (hasSetting(key)) {
        throw invalid_argument(
            "Duplicate configuration setting: " + key
        );
    }

    settings.emplace(key, value);
}

// ============================================================
// Configuration validation
// ============================================================

void TaxRules::validate() const {
    if (
        financialYear != "2025-26" ||
        assessmentYear != "2026-27"
    ) {
        throw invalid_argument(
            "This application requires FY 2025-26 / AY 2026-27."
        );
    }

    const string expectedRegime =
        regime == RegimeType::Old ? "OLD" : "NEW";

    if (upperCase(getString("REGIME")) != expectedRegime) {
        throw invalid_argument(
            "Regime file does not match the requested regime."
        );
    }

    const double cessRate =
        getNumber("HEALTH_AND_EDUCATION_CESS_RATE");

    if (cessRate < 0.0 || cessRate > 100.0) {
        throw invalid_argument(
            "Cess rate must be between 0 and 100."
        );
    }

    if (slabProfiles.empty()) {
        throw invalid_argument(
            "At least one slab profile must be configured."
        );
    }

    for (const auto& entry : slabProfiles) {
        const TaxSlabProfile& profile = entry.second;

        if (profile.slabs.empty()) {
            throw invalid_argument(
                "Empty slab profile: " + profile.name
            );
        }

        double expectedLowerLimit = 0.0;

        for (size_t i = 0; i < profile.slabs.size(); ++i) {
            const TaxSlab& slab = profile.slabs[i];

            if (
                slab.lowerLimit < 0.0 ||
                !approximatelyEqual(
                    slab.lowerLimit,
                    expectedLowerLimit
                )
            ) {
                throw invalid_argument(
                    "Slabs must start at zero and be contiguous "
                    "in profile: " + profile.name
                );
            }

            if (
                slab.ratePercent < 0.0 ||
                slab.ratePercent > 100.0
            ) {
                throw invalid_argument(
                    "Invalid slab rate in profile: " + profile.name
                );
            }

            const bool isLast =
                i + 1 == profile.slabs.size();

            if (!slab.hasUpperLimit) {
                if (!isLast) {
                    throw invalid_argument(
                        "Only the last slab may use INF: " +
                        profile.name
                    );
                }
            } else {
                if (slab.upperLimit <= slab.lowerLimit) {
                    throw invalid_argument(
                        "Slab upper limit must exceed lower limit: " +
                        profile.name
                    );
                }

                if (isLast) {
                    throw invalid_argument(
                        "The last slab must use INF: " +
                        profile.name
                    );
                }

                expectedLowerLimit = slab.upperLimit;
            }
        }
    }

    for (const auto& entry : deductionRules) {
        const DeductionRule& rule = entry.second;

        if (
            rule.hasMaximumAmount &&
            rule.maximumAmount < 0.0
        ) {
            throw invalid_argument(
                "Negative deduction limit: " + rule.code
            );
        }
    }

    for (const auto& entry : specialIncomeRules) {
        const SpecialIncomeRule& rule = entry.second;

        if (
            rule.ratePercent < 0.0 ||
            rule.ratePercent > 100.0 ||
            (rule.hasAnnualThreshold &&
             rule.annualThreshold < 0.0)
        ) {
            throw invalid_argument(
                "Invalid special-income rule: " + rule.code
            );
        }
    }

    if (advanceTaxCheckpoints.size() != 4) {
        throw invalid_argument(
            "Configure four standard advance-tax checkpoints."
        );
    }

    double previousPercent = 0.0;
    string previousDate;

    for (size_t i = 0; i < advanceTaxCheckpoints.size(); ++i) {
        const AdvanceTaxCheckpoint& checkpoint =
            advanceTaxCheckpoints[i];

        if (
            checkpoint.installmentNumber !=
            static_cast<int>(i + 1)
        ) {
            throw invalid_argument(
                "Advance-tax checkpoints must be numbered 1 to 4."
            );
        }

        if (
            !validDate(checkpoint.dueDate) ||
            checkpoint.dueDate < "2025-04-01" ||
            checkpoint.dueDate > "2026-03-31" ||
            (!previousDate.empty() &&
             checkpoint.dueDate <= previousDate)
        ) {
            throw invalid_argument(
                "Invalid or unordered advance-tax due date."
            );
        }

        if (
            checkpoint.cumulativePercent <= previousPercent ||
            checkpoint.cumulativePercent > 100.0
        ) {
            throw invalid_argument(
                "Advance-tax percentages must increase up to 100."
            );
        }

        previousDate = checkpoint.dueDate;
        previousPercent = checkpoint.cumulativePercent;
    }

    if (!approximatelyEqual(previousPercent, 100.0)) {
        throw invalid_argument(
            "The final advance-tax target must be 100 percent."
        );
    }
}

// ============================================================
// General settings
// ============================================================

bool TaxRules::hasSetting(const string& key) const {
    return settings.find(key) != settings.end();
}

string TaxRules::getString(const string& key) const {
    const auto position = settings.find(key);

    if (position == settings.end()) {
        throw out_of_range(
            "Missing required configuration setting: " + key
        );
    }

    return position->second;
}

double TaxRules::getNumber(const string& key) const {
    return parseNumber(getString(key), key);
}

int TaxRules::getInteger(const string& key) const {
    return parseInteger(getString(key), key);
}

bool TaxRules::getBoolean(const string& key) const {
    return parseBoolean(getString(key), key);
}

string TaxRules::getStringOrDefault(
    const string& key,
    const string& defaultValue
) const {
    return hasSetting(key)
        ? getString(key)
        : defaultValue;
}

double TaxRules::getNumberOrDefault(
    const string& key,
    double defaultValue
) const {
    return hasSetting(key)
        ? getNumber(key)
        : defaultValue;
}

int TaxRules::getIntegerOrDefault(
    const string& key,
    int defaultValue
) const {
    return hasSetting(key)
        ? getInteger(key)
        : defaultValue;
}

bool TaxRules::getBooleanOrDefault(
    const string& key,
    bool defaultValue
) const {
    return hasSetting(key)
        ? getBoolean(key)
        : defaultValue;
}

// ============================================================
// Structured rule access
// ============================================================

bool TaxRules::hasSlabProfile(
    const string& profileName
) const {
    return slabProfiles.find(profileName) != slabProfiles.end();
}

const TaxSlabProfile& TaxRules::getSlabProfile(
    const string& profileName
) const {
    const auto position = slabProfiles.find(profileName);

    if (position == slabProfiles.end()) {
        throw out_of_range(
            "Missing slab profile: " + profileName
        );
    }

    return position->second;
}

bool TaxRules::hasDeductionRule(
    const string& code
) const {
    return deductionRules.find(code) != deductionRules.end();
}

const DeductionRule& TaxRules::getDeductionRule(
    const string& code
) const {
    const auto position = deductionRules.find(code);

    if (position == deductionRules.end()) {
        throw out_of_range(
            "Missing deduction rule: " + code
        );
    }

    return position->second;
}

bool TaxRules::hasSpecialIncomeRule(
    const string& code
) const {
    return specialIncomeRules.find(code) !=
           specialIncomeRules.end();
}

const SpecialIncomeRule& TaxRules::getSpecialIncomeRule(
    const string& code
) const {
    const auto position = specialIncomeRules.find(code);

    if (position == specialIncomeRules.end()) {
        throw out_of_range(
            "Missing special-income rule: " + code
        );
    }

    return position->second;
}

const vector<AdvanceTaxCheckpoint>&
TaxRules::getAdvanceTaxCheckpoints() const {
    return advanceTaxCheckpoints;
}