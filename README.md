# 💰 Income Tax Computation System

<p align="center">

**A Modular C++ Console-Based Income Tax Calculator**

An object-oriented application that simplifies annual income tax computation through modular income handling, configurable tax rules, automated calculations, and Old vs New Regime comparison.

![C++](https://img.shields.io/badge/C++-17-blue?logo=cplusplus)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey)
![Application](https://img.shields.io/badge/Type-Console%20Application-0D9488)
![Architecture](https://img.shields.io/badge/Design-Object--Oriented-purple)

</p>

---

## 📌 Overview

The **Income Tax Computation System** is a C++ application developed using Object-Oriented Programming principles to organize and automate the annual income tax calculation process.

The system collects taxpayer information, processes different sources of income, evaluates deduction claims, accounts for taxes already paid, and generates detailed computation reports.

One of its key features is the ability to calculate tax under both the **Old Tax Regime and New Tax Regime using the same set of inputs**, making it easier to compare the resulting tax liabilities.

The application uses a modular architecture in which each income category, deduction, payment, and calculation component has a separate responsibility.

## ✨ Key Features

### 1. Multiple Income Sources

Supports different categories of income:

- Salary Income
- House Property Income
- Business and Professional Income
- Capital Gains
- Income from Other Sources
- Exempt Income

### 2. Dual Tax Regime Computation

- Calculates tax under the Old Tax Regime.
- Calculates tax under the New Tax Regime.
- Uses identical taxpayer and financial inputs for both calculations.
- Generates separate reports for each regime.
- Displays the balance tax payable or estimated refund.

### 3. Deduction Management

Supports configured Chapter VI-A deduction claims, including:

- Section 80C
- Section 80CCC
- Section 80CCD
- Section 80D
- Other implemented deduction categories.

The system applies configured deduction limits and applicable overall caps.

### 4. Tax Payment and Credit Management

Accounts for payments and credits already made:

- Tax Deducted at Source (TDS)
- Tax Collected at Source (TCS)
- Advance Tax
- Self-Assessment Tax

These amounts are adjusted against the final tax liability.

### 5. Configurable Tax Rules

Tax slabs, deduction limits, rebates, surcharge rules, and cess settings are maintained in external configuration files rather than being embedded directly into the calculation logic.

### 6. Detailed Report Generation

Automatically generates two text reports:

- `report_new_regime.txt`
- `report_old_regime.txt`

Each report contains income details, deductions, tax calculations, credits, and the final payable or refund amount.

---

## 🛠️ Technology Stack

| Technology               | Purpose                                       |
| ------------------------ | --------------------------------------------- |
| C++                      | Core application development                  |
| C++17                    | Language standard                             |
| OOP                      | Modular application architecture              |
| STL                      | Strings, vectors, collections and utilities   |
| File Handling            | Reading configurations and generating reports |
| Exception Handling       | Handling runtime errors                       |
| Assertions               | Focused calculation testing                   |
| Text Configuration Files | Managing tax rules and regime settings        |

---

## 🏗️ System Architecture

The application follows a modular architecture with clearly separated responsibilities.

```text
                  CONSOLE USER
                       |
                       v
                 INPUT HANDLER
                       |
                       v
                   TAXPAYER
                       |
          +------------+------------+
          |            |            |
          v            v            v
       INCOME       DEDUCTIONS    PAYMENTS
          |            |            |
          v            v            v
    Income Modules  Deduction     Tax Credits
                    Evaluation
          |            |            |
          +------------+------------+
                       |
                       v
                 TAX RULES
                       |
                       v
                TAX CALCULATOR
                       |
                       v
             TAX COMPUTATION RESULT
                       |
                       v
               REPORT GENERATOR
                       |
              +--------+--------+
              |                 |
              v                 v
         OLD REGIME         NEW REGIME
           REPORT             REPORT
```

## 🧠 Object-Oriented Programming Concepts

The project demonstrates the practical application of the following OOP concepts:

| Concept        | Implementation                                                              |
| -------------- | --------------------------------------------------------------------------- |
| Encapsulation  | Income, taxpayer, deduction and payment data are managed through classes.   |
| Abstraction    | Common `Income` interface provides a shared contract for income categories. |
| Inheritance    | Specialized income classes inherit from the common income abstraction.      |
| Polymorphism   | Different income objects are processed through the common interface.        |
| Constructors   | Objects are initialized with their required data.                           |
| Modular Design | Separate header and source files organize different responsibilities.       |

### Major Classes

- `Taxpayer` – Stores taxpayer category and relevant profile information.
- `Income` – Common abstraction for different income categories.
- `SalaryIncome` – Handles salary-related calculations.
- `HousePropertyIncome` – Handles house property income.
- `BusinessIncome` – Handles business and professional income.
- `CapitalGainIncome` – Handles capital gains calculations.
- `OtherSourcesIncome` – Handles other taxable income.
- `ExemptIncome` – Handles exempt income entries.
- `Deductions` – Evaluates deduction claims.
- `Payments` – Manages tax payments and credits.
- `TaxRules` – Loads and manages tax configurations.
- `TaxCalculator` – Performs the central tax computation.
- `TaxComputationResult` – Stores the final calculation results.
- `ReportGenerator` – Generates detailed computation reports.
- `ApplicationSession` – Coordinates the current application session.

---

## 📂 Project Structure

```text
income-tax-project/
│
├── config/
│   ├── common_tax_rules_AY_2026_27.txt
│   ├── new_regime_AY_2026_2027.txt
│   └── old_regime_AY_2026_27.txt
│
├── include/
│   ├── ApplicationSession.h
│   ├── BusinessIncome.h
│   ├── CapitalGainIncome.h
│   ├── Deductions.h
│   ├── ExemptIncome.h
│   ├── HousePropertyIncome.h
│   ├── Income.h
│   ├── InputHandler.h
│   ├── OtherSourcesIncome.h
│   ├── Payments.h
│   ├── ReportGenerator.h
│   ├── SalaryIncome.h
│   ├── TaxCalculator.h
│   ├── TaxComputationResult.h
│   ├── TaxRules.h
│   └── Taxpayer.h
│
├── src/
│   ├── ApplicationSession.cpp
│   ├── BusinessIncome.cpp
│   ├── CapitalGainIncome.cpp
│   ├── Deductions.cpp
│   ├── ExemptIncome.cpp
│   ├── HousePropertyIncome.cpp
│   ├── InputHandler.cpp
│   ├── main.cpp
│   ├── OtherSourcesIncome.cpp
│   ├── Payments.cpp
│   ├── ReportGenerator.cpp
│   ├── SalaryIncome.cpp
│   ├── TaxCalculator.cpp
│   ├── TaxRules.cpp
│   └── Taxpayer.cpp
│
├── tests/
│   ├── annual_tax_route_tests.cpp
│   ├── capital_route_tests.cpp
│   ├── deduction_route_tests.cpp
│   ├── presumptive_route_tests.cpp
│   └── README.md
│
├── docs/
│   ├── BUILD_AND_RUN.md
│   ├── CONFIGURATION.md
│   ├── DEVELOPER_NOTES.md
│   ├── RELEASE_CHECKLIST.md
│   ├── ROUTE_AUDIT_AND_COVERAGE.md
│   └── USER_WORKFLOW.md
│
├── report_new_regime.txt
├── report_old_regime.txt
└── .gitignore
```

---

## ⚙️ Installation and Execution

### Prerequisites

- Windows / Linux
- GCC or MinGW-w64 compiler
- C++17 or later

Verify your compiler:

```bash
g++ --version
```

### Step 1: Clone the Repository

```bash
git clone YOUR_REPOSITORY_URL
```

Navigate to the project directory:

```bash
cd income-tax-project
```

### Step 2: Compile the Project

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o income_tax.exe
```

### Step 3: Run the Application

**Windows (PowerShell):**

```powershell
.\income_tax.exe
```

**Linux:**

```bash
./income_tax.exe
```

**Important:** Run the executable from the project root directory because the application loads its configuration files using relative paths.

### Step 4: View Generated Reports

After successful execution, the application generates:

```text
report_old_regime.txt
report_new_regime.txt
```

Open these files using any text editor to inspect the detailed computation.

---

## 🧪 Testing

The project includes focused calculation-route tests for important tax computation scenarios.

| Test File                     | Testing Focus                                     |
| ----------------------------- | ------------------------------------------------- |
| `capital_route_tests.cpp`     | Capital gains and Section 50C / 112A calculations |
| `deduction_route_tests.cpp`   | Deduction calculations and configured limits      |
| `presumptive_route_tests.cpp` | Sections 44AD and 44ADA                           |
| `annual_tax_route_tests.cpp`  | Annual tax aggregation and capital-loss set-off   |

The test instructions and compilation commands are available in `tests/README.md`.

---

## 📊 Sample Output

The following is an illustrative demonstration using:

- Salary Income: ₹12,00,000
- Section 80C Claim: ₹1,50,000
- TDS Credit: ₹50,000
- Individual Taxpayer
- Age: 30 Years

### Regime Comparison

```text
=============================================
          TAX REGIME COMPARISON
=============================================

NEW REGIME
---------------------------------------------
Balance Tax Payable : Rs. 0
Estimated Refund   : Rs. 50000

OLD REGIME
---------------------------------------------
Balance Tax Payable : Rs. 67000
Estimated Refund   : Rs. 0

=============================================
Reports saved:
report_old_regime.txt
report_new_regime.txt
=============================================
```

_Sample output from the project's configured demonstration inputs. Actual results depend on user inputs and the loaded tax rules._

---

## ⚠️ Limitations and Disclaimer

This project is developed for academic and educational purposes.

- Tax calculations depend on user-provided information.
- The application does not independently verify supporting financial documents.
- Legal eligibility for deductions and exemptions is not independently established.
- Some statutory provisions and exceptional tax scenarios are simplified or incomplete.
- The application does not connect to government tax portals.
- It does not file income tax returns.
- Taxpayer information is session-based and is not retained as a permanent database.
- Generated reports should be independently verified before being used for actual tax decisions.

**This application is a tax computation tool, not professional tax advice or an official income tax filing platform.**

---

## 🎯 Project Objectives

The primary objectives of this project are:

1. Apply Object-Oriented Programming concepts to a practical financial application.
2. Develop reusable and modular income computation components.
3. Automate the calculation of income tax using configurable rules.
4. Implement comparison between the Old and New Tax Regimes.
5. Generate structured and readable tax computation reports.
6. Improve code maintainability through separation of responsibilities.

---

## 👨‍💻 Academic Project

**Project:** Income Tax Computation System

**Course:** Object-Oriented Programming System Project

**Language:** C++

**Institution:** IIIT Bhubaneswar

---

<p align="center">

### 💻 Built with C++ | Powered by OOP

**Thank You for Visiting!**

</p>
