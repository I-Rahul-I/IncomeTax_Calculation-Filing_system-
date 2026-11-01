# Build and Run

This application is a session-based annual income-tax computation tool.

It uses only data entered during the current session. It does not retain
prior-year data, permanent taxpayer records, or uploaded documents.

## Requirements

- C++17 or later compiler
- CMake 3.16 or later
- The common configuration file described in `docs/CONFIGURATION.md`

## Project layout

```text
include/
  Taxpayer.h
  TaxRules.h
  Income.h
  IncomeComputationResult.h
  SalaryIncome.h
  HousePropertyIncome.h
  BusinessIncome.h
  CapitalGainsIncome.h
  OtherSourcesIncome.h
  ExemptIncome.h
  Deductions.h
  Payments.h
  TaxCalculator.h
  TaxComputationResult.h
  ReportGenerator.h
  ApplicationSession.h

src/
  Taxpayer.cpp
  TaxRules.cpp
  Income.cpp
  SalaryIncome.cpp
  HousePropertyIncome.cpp
  BusinessIncome.cpp
  CapitalGainsIncome.cpp
  OtherSourcesIncome.cpp
  ExemptIncome.cpp
  Deductions.cpp
  Payments.cpp
  TaxCalculator.cpp
  ReportGenerator.cpp
  ApplicationSession.cpp
  main.cpp

docs/
  CONFIGURATION.md
  BUILD_AND_RUN.md
```

## Build with CMake

Create a `CMakeLists.txt` file in the project root:

```cmake
cmake_minimum_required(VERSION 3.16)

project(IncomeTaxCalculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    income_tax_calculator
    src/Taxpayer.cpp
    src/TaxRules.cpp
    src/Income.cpp
    src/SalaryIncome.cpp
    src/HousePropertyIncome.cpp
    src/BusinessIncome.cpp
    src/CapitalGainsIncome.cpp
    src/OtherSourcesIncome.cpp
    src/ExemptIncome.cpp
    src/Deductions.cpp
    src/Payments.cpp
    src/TaxCalculator.cpp
    src/ReportGenerator.cpp
    src/ApplicationSession.cpp
    src/main.cpp
)

target_include_directories(
    income_tax_calculator
    PRIVATE
    include
)
```

Build the application:

```bash
cmake -S . -B build
cmake --build build
```

On Linux or macOS, the executable is normally created at:

```text
build/income_tax_calculator
```

On Windows, it is normally created under a configuration-specific folder, such as:

```text
build/Debug/income_tax_calculator.exe
```

## Run the application

Run the executable with the path to the common configuration file:

```bash
./build/income_tax_calculator config/tax_rules_2025_26.conf
```

On Windows:

```powershell
.\build\Debug\income_tax_calculator.exe config\tax_rules_2025_26.conf
```

The application performs a small built-in demonstration:

1. Loads the configuration file.
2. Selects an Individual taxpayer under the New Regime.
3. Adds salary income.
4. Adds savings-account interest under Other Sources.
5. Adds a Section 80C claim.
6. Adds a TDS credit.
7. Calculates tax.
8. Prints a plain-text computation report.

## Configuration file

The application requires a configuration file containing the rule codes
used by the income, deduction, special-income, surcharge, rebate, cess,
and payment modules.

At minimum, the demonstration requires entries for:

```text
SUPPORT_INDIVIDUAL
SUPPORT_TDS_TCS
SUPPORT_ADVANCE_TAX
SUPPORT_SELF_ASSESSMENT_TAX

SLAB entries for Individual, Below60, New Regime

DEDUCTION_801
DEDUCTION_80C_80CCC_80CCD1_LIMIT

FAMILY_PENSION_DEDUCTION_PERCENT
FAMILY_PENSION_DEDUCTION_MAX

REBATE_INCOME_LIMIT
REBATE_MAXIMUM

SURCHARGE rules for Individual
SURCHARGE_INCLUDES_SPECIAL_TAX
SURCHARGE_MARGINAL_RELIEF

HEALTH_AND_EDUCATION_CESS_RATE
```

The exact configuration format is defined in `docs/CONFIGURATION.md`.

## Important limitations

- The application does not determine legal eligibility for exemptions,
  deductions, or payment credits.
- It does not validate supporting documents or verify third-party data.
- Business income must be entered as the user-computed taxable profit.
- Capital-gains entries must already be classified under the correct
  capital-gains category.
- Online-game and virtual-digital-asset amounts must be entered as the
  user-determined taxable amounts.
- Agricultural income is reported separately and requires a separate
  rate-purpose check before treating the result as final.
- The report is a computation summary, not tax advice.


## Regime comparison reports

The application collects the current-year details once, then calculates under both
regimes using the corresponding configuration files. It writes these files in the
project root:

- `report_old_regime.txt`
- `report_new_regime.txt`

Each report ends with a prominent final-result section identifying the balance tax
payable or estimated refund after entered credits. The result is an educational
computation and must be verified against current law and source documents.
