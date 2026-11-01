#include "InputHandler.h"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace std;

// ============================================================
// Constructor
// ============================================================

InputHandler::InputHandler(
    istream& input,
    ostream& output
)
    : input(input),
      output(output) {
}

// ============================================================
// Internal helpers
// ============================================================

string InputHandler::trim(const string& value) {
    const string whitespace = " \t\n\r\f\v";

    const size_t first = value.find_first_not_of(whitespace);

    if (first == string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(whitespace);

    return value.substr(first, last - first + 1);
}

string InputHandler::toUpperCase(const string& value) {
    string result = value;

    for (char& character : result) {
        if (character >= 'a' && character <= 'z') {
            character = static_cast<char>(
                character - 'a' + 'A'
            );
        }
    }

    return result;
}

bool InputHandler::isValidPANFormat(const string& pan) {
    if (pan.length() != 10) {
        return false;
    }

    for (size_t i = 0; i < 5; ++i) {
        if (pan[i] < 'A' || pan[i] > 'Z') {
            return false;
        }
    }

    for (size_t i = 5; i < 9; ++i) {
        if (pan[i] < '0' || pan[i] > '9') {
            return false;
        }
    }

    return pan[9] >= 'A' && pan[9] <= 'Z';
}

string InputHandler::readLine(const string& prompt) {
    // Keep the original prompt wording intact, but present it as a clear,
    // consistent console input card. The prompt text itself is not rewritten.
    output << "\n  +--------------------------------------------------+\n"
           << "  |  " << prompt << "\n"
           << "  +--------------------------------------------------+\n"
           << "  > " << flush;

    string value;

    if (!getline(input, value)) {
        throw runtime_error(
            "Input ended before all required details were entered."
        );
    }

    return value;
}

// ============================================================
// Welcome screen
// ============================================================

void InputHandler::displayWelcome() {
    output
        << "\n"
        << "====================================================\n"
        << "       INCOME TAX COMPUTATION AND COMPARISON\n"
        << "====================================================\n"
        << "Financial Year : "
        << Taxpayer::getFinancialYear() << "\n"
        << "Assessment Year: "
        << Taxpayer::getAssessmentYear() << "\n\n"
        << "Enter the requested details for the current session.\n"
        << "Applicable tax options depend on taxpayer category.\n"
        << "Separate old-regime and new-regime text reports will be generated after computation.\n\n"
        << "Academic application: this does not submit an ITR.\n"
        << "Use CA-verified rules before relying on results.\n"
        << "====================================================\n";
}

// ============================================================
// Reusable input helpers
// ============================================================

string InputHandler::readText(
    const string& prompt,
    bool allowEmpty
) {
    while (true) {
        const string value = trim(readLine(prompt));

        if (allowEmpty || !value.empty()) {
            return value;
        }

        output << "This field cannot be empty. Please try again.\n";
    }
}

int InputHandler::readInteger(
    const string& prompt,
    int minimum,
    int maximum
) {
    if (minimum > maximum) {
        throw invalid_argument(
            "Integer input minimum cannot exceed maximum."
        );
    }

    while (true) {
        const string text = trim(readLine(prompt));

        istringstream parser(text);

        int value;
        char extra;

        // Reading an additional non-whitespace character rejects
        // inputs such as "25abc", "2.5", or "10 20".
        if (
            (parser >> value) &&
            !(parser >> extra) &&
            value >= minimum &&
            value <= maximum
        ) {
            return value;
        }

        output
            << "Enter a whole number from "
            << minimum << " to " << maximum << ".\n";
    }
}

double InputHandler::readAmount(const string& prompt) {
    while (true) {
        const string text = trim(readLine(prompt));

        // Allow plain decimal notation only.
        // Reject commas, currency symbols, signs and exponents.
        bool validFormat = !text.empty();
        bool foundDigit = false;
        bool foundDecimalPoint = false;

        for (char character : text) {
            if (character >= '0' && character <= '9') {
                foundDigit = true;
            } else if (
                character == '.' &&
                !foundDecimalPoint
            ) {
                foundDecimalPoint = true;
            } else {
                validFormat = false;
                break;
            }
        }

        if (!validFormat || !foundDigit) {
            output
                << "Enter a non-negative amount without commas or "
                << "currency symbols, for example 150000.50.\n";

            continue;
        }

        istringstream parser(text);

        double amount;
        char extra;

        if (
            (parser >> amount) &&
            !(parser >> extra) &&
            isfinite(amount) &&
            amount >= 0.0
        ) {
            return amount;
        }

        output
            << "The amount is invalid or too large. "
            << "Please enter it again.\n";
    }
}

bool InputHandler::readYesNo(const string& prompt) {
    while (true) {
        const string answer = toUpperCase(
            trim(readLine(prompt))
        );

        if (answer == "Y" || answer == "YES") {
            return true;
        }

        if (answer == "N" || answer == "NO") {
            return false;
        }

        output << "Please enter Y/YES or N/NO.\n";
    }
}

string InputHandler::readPAN() {
    while (true) {
        const string pan = toUpperCase(
            readText(
                "Enter PAN (press Enter to skip): ",
                true
            )
        );

        if (pan.empty() || isValidPANFormat(pan)) {
            return pan;
        }

        output
            << "Invalid PAN format. Expected five letters, "
            << "four digits, and one letter.\n"
            << "Example format: ABCDE1234F\n";
    }
}

// ============================================================
// Taxpayer-specific input
// ============================================================

TaxpayerType InputHandler::readTaxpayerType() {
    output
        << "\nSelect taxpayer category:\n"
        << "1. Individual\n"
        << "2. HUF (Hindu Undivided Family)\n"
        << "3. Exit\n";

    const int choice = readInteger(
        "Enter your choice (1-3): ",
        1,
        6
    );

    switch (choice) {
        case 1: return TaxpayerType::Individual;
        case 2: return TaxpayerType::HUF;
        case 3: throw invalid_argument("Exit selected.");
        default: throw invalid_argument("Invalid taxpayer category.");
    }
}

ResidentialStatus InputHandler::readResidentialStatus(
    TaxpayerType taxpayerType
) {
    bool allowRNOR = false;

    switch (taxpayerType) {
        case TaxpayerType::Individual:
        case TaxpayerType::HUF:
            allowRNOR = true;
            break;

        default:
            throw invalid_argument(
                "Cannot collect residential status for "
                "an invalid taxpayer category."
            );
    }

    output
        << "\nSelect residential status:\n"
        << "1. Resident\n"
        << "2. Non-Resident\n";

    if (allowRNOR) {
        output
            << "3. Resident but Not Ordinarily Resident (RNOR)\n";
    }

    const int maximumChoice = allowRNOR ? 3 : 2;

    const int choice = readInteger(
        allowRNOR
            ? "Enter your choice (1-3): "
            : "Enter your choice (1-2): ",
        1,
        maximumChoice
    );

    switch (choice) {
        case 1: return ResidentialStatus::Resident;
        case 2: return ResidentialStatus::NonResident;
        case 3: return ResidentialStatus::ResidentButNotOrdinarilyResident;
        default: throw invalid_argument("Invalid residential status.");
    }
}

int InputHandler::readIndividualAge() {
    output
        << "\nEnter completed age as at 31 March 2026,\n"
        << "not your age on the day you run this program.\n";

    return readInteger(
        "Age: ",
        0,
        numeric_limits<int>::max()
    );
}

unique_ptr<Taxpayer> InputHandler::collectTaxpayerDetails() {
    output
        << "\n----------------------------------------------------\n"
        << "                 TAXPAYER DETAILS\n"
        << "----------------------------------------------------\n";

    const TaxpayerType type = readTaxpayerType();

    const string name = readText(
        type == TaxpayerType::Individual
            ? "\nEnter full name: "
            : "\nEnter entity name: "
    );

    const string pan = readPAN();

    const ResidentialStatus status =
        readResidentialStatus(type);

    switch (type) {
        case TaxpayerType::Individual: {
            const int age = readIndividualAge();

            return make_unique<IndividualTaxpayer>(
                name,
                pan,
                status,
                age
            );
        }

        case TaxpayerType::HUF:
            return make_unique<HUFTaxpayer>(
                name,
                pan,
                status
            );

        default:
            throw invalid_argument(
                "Cannot create an invalid taxpayer category."
            );
    }
}

void InputHandler::displayTaxpayerSummary(
    const Taxpayer& taxpayer
) {
    output
        << "\n====================================================\n"
        << "                  TAXPAYER SUMMARY\n"
        << "====================================================\n"
        << "Name              : " << taxpayer.getName() << "\n"
        << "PAN               : "
        << (
            taxpayer.getPAN().empty()
                ? "Not provided"
                : taxpayer.getPAN()
        )
        << "\n"
        << "Taxpayer category : "
        << taxpayer.getCategoryName() << "\n"
        << "Residential status: "
        << residentialStatusToString(
               taxpayer.getResidentialStatus()
           )
        << "\n"
        << "Financial Year    : "
        << Taxpayer::getFinancialYear() << "\n"
        << "Assessment Year   : "
        << Taxpayer::getAssessmentYear() << "\n";

    const IndividualTaxpayer* individual =
        dynamic_cast<const IndividualTaxpayer*>(&taxpayer);

    if (individual != nullptr) {
        output
            << "Age at FY end     : "
            << individual->getAge() << "\n"
            << "Age category      : "
            << individual->getAgeCategoryName() << "\n";
    }

    output
        << "====================================================\n";
}