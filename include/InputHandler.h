#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <iostream>
#include <memory>
#include <string>

#include "Taxpayer.h"

using namespace std;

class InputHandler {
private:
    istream& input;
    ostream& output;

    // Read one complete line.
    // The implementation will throw an exception if input ends
    // unexpectedly, rather than repeatedly prompting forever.
    string readLine(const string& prompt);

    // Remove leading and trailing whitespace.
    static string trim(const string& value);

    // Convert ASCII lowercase letters to uppercase.
    static string toUpperCase(const string& value);

    // Check the structural PAN pattern:
    // five letters, four digits, one letter.
    //
    // This does not verify PAN registration or ownership.
    static bool isValidPANFormat(const string& pan);

public:
    // By default, use the console.
    // Streams can also be supplied explicitly.
    explicit InputHandler(
        istream& input = cin,
        ostream& output = cout
    );

    // Display the project title, supported year, and disclaimer.
    void displayWelcome();

    // ========================================================
    // Reusable input helpers
    // ========================================================

    // Read trimmed text.
    // When allowEmpty is false, blank input is rejected.
    string readText(
        const string& prompt,
        bool allowEmpty = false
    );

    // Read a complete integer within an inclusive range.
    // Rejects entries such as "25abc" or "2.5".
    int readInteger(
        const string& prompt,
        int minimum,
        int maximum
    );

    // Read a finite, non-negative monetary amount.
    // The user enters a plain number without currency symbols
    // or commas, for example: 150000.50.
    double readAmount(const string& prompt);

    // Accept Y / YES / N / NO, case-insensitively.
    bool readYesNo(const string& prompt);

    // Read an optional PAN.
    // Blank input means the taxpayer has skipped this field.
    string readPAN();

    // ========================================================
    // Taxpayer-specific input
    // ========================================================

    TaxpayerType readTaxpayerType();

    // Display only the residential-status selections supported
    // for the chosen taxpayer category.
    ResidentialStatus readResidentialStatus(
        TaxpayerType taxpayerType
    );

    // Read completed age as at 31 March 2026.
    // Called only for an Individual taxpayer.
    int readIndividualAge();

    // Collect basic details and create the corresponding
    // concrete taxpayer object.
    //
    // Ownership of the object is returned to the caller.
    unique_ptr<Taxpayer> collectTaxpayerDetails();

    // Display the basic information collected.
    // This does not perform any tax calculation.
    void displayTaxpayerSummary(
        const Taxpayer& taxpayer
    );
};

#endif