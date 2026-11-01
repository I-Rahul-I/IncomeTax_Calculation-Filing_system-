#include "Taxpayer.h"

#include <stdexcept>

using namespace std;

namespace {

// Remove leading and trailing whitespace.
string trim(const string& value) {
    const string whitespace = " \t\n\r\f\v";

    const size_t first = value.find_first_not_of(whitespace);

    if (first == string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(whitespace);

    return value.substr(first, last - first + 1);
}

// PAN is optional. When supplied, normalize letters to uppercase.
// Detailed format validation will be handled by the input module.
string normalizePAN(const string& value) {
    string normalized = trim(value);

    for (char& character : normalized) {
        if (character >= 'a' && character <= 'z') {
            character = static_cast<char>(
                character - 'a' + 'A'
            );
        }
    }

    return normalized;
}

bool isValidResidentialStatus(ResidentialStatus status) {
    switch (status) {
        case ResidentialStatus::Resident:
        case ResidentialStatus::NonResident:
        case ResidentialStatus::ResidentButNotOrdinarilyResident:
            return true;

        default:
            return false;
    }
}

} // namespace

// ============================================================
// Taxpayer
// ============================================================

Taxpayer::Taxpayer(
    const string& name,
    const string& pan,
    TaxpayerType type,
    ResidentialStatus residentialStatus
)
    : name(""),
      pan(""),
      type(type),
      residentialStatus(ResidentialStatus::Resident) {

    setName(name);
    setPAN(pan);
    setResidentialStatus(residentialStatus);
}

const string& Taxpayer::getName() const {
    return name;
}

const string& Taxpayer::getPAN() const {
    return pan;
}

TaxpayerType Taxpayer::getType() const {
    return type;
}

ResidentialStatus Taxpayer::getResidentialStatus() const {
    return residentialStatus;
}

void Taxpayer::setName(const string& name) {
    const string cleanedName = trim(name);

    if (cleanedName.empty()) {
        throw invalid_argument(
            "Taxpayer name cannot be empty."
        );
    }

    this->name = cleanedName;
}

void Taxpayer::setPAN(const string& pan) {
    this->pan = normalizePAN(pan);
}

void Taxpayer::setResidentialStatus(ResidentialStatus status) {
    if (!isValidResidentialStatus(status)) {
        throw invalid_argument(
            "Invalid residential status."
        );
    }

    residentialStatus = status;
}

string Taxpayer::getFinancialYear() {
    return "2025-26";
}

string Taxpayer::getAssessmentYear() {
    return "2026-27";
}

// ============================================================
// IndividualTaxpayer
// ============================================================

IndividualTaxpayer::IndividualTaxpayer(
    const string& name,
    const string& pan,
    ResidentialStatus residentialStatus,
    int age
)
    : Taxpayer(
          name,
          pan,
          TaxpayerType::Individual,
          residentialStatus
      ),
      age(0) {

    setAge(age);
}

int IndividualTaxpayer::getAge() const {
    return age;
}

void IndividualTaxpayer::setAge(int age) {
    if (age < 0) {
        throw invalid_argument(
            "Age cannot be negative."
        );
    }

    this->age = age;
}

AgeCategory IndividualTaxpayer::getAgeCategory() const {
    if (age < 60) {
        return AgeCategory::Below60;
    }

    if (age < 80) {
        return AgeCategory::SeniorCitizen;
    }

    return AgeCategory::SuperSeniorCitizen;
}

string IndividualTaxpayer::getAgeCategoryName() const {
    switch (getAgeCategory()) {
        case AgeCategory::Below60:
            return "Below 60 years";

        case AgeCategory::SeniorCitizen:
            return "Senior Citizen (60 to below 80 years)";

        case AgeCategory::SuperSeniorCitizen:
            return "Super Senior Citizen (80 years and above)";

        default:
            throw logic_error(
                "Unexpected age category."
            );
    }
}

string IndividualTaxpayer::getCategoryName() const {
    return taxpayerTypeToString(getType());
}

// ============================================================
// HUFTaxpayer
// ============================================================

HUFTaxpayer::HUFTaxpayer(
    const string& name,
    const string& pan,
    ResidentialStatus residentialStatus
)
    : Taxpayer(
          name,
          pan,
          TaxpayerType::HUF,
          residentialStatus
      ) {
}

string HUFTaxpayer::getCategoryName() const {
    return taxpayerTypeToString(getType());
}

// ============================================================
// Shared conversion helpers
// ============================================================

string taxpayerTypeToString(TaxpayerType type) {
    switch (type) {
        case TaxpayerType::Individual:
            return "Individual";

        case TaxpayerType::HUF:
            return "HUF (Hindu Undivided Family)";

        default:
            throw invalid_argument(
                "Invalid taxpayer type."
            );
    }
}

string residentialStatusToString(ResidentialStatus status) {
    switch (status) {
        case ResidentialStatus::Resident:
            return "Resident";

        case ResidentialStatus::NonResident:
            return "Non-Resident";

        case ResidentialStatus::ResidentButNotOrdinarilyResident:
            return "Resident but Not Ordinarily Resident (RNOR)";

        default:
            throw invalid_argument(
                "Invalid residential status."
            );
    }
}