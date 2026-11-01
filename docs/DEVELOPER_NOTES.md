# Developer Notes

This project is a session-based annual income-tax computation application.

It is designed to calculate tax from user-entered annual data and a
configuration-driven rule set. It does not retain prior-year data,
permanent taxpayer records, or uploaded documents.

## Core design rules

### Session-only data

- One `ApplicationSession` represents one annual computation.
- Closing the application discards all session data.
- No prior-year balances, taxpayer profiles, documents, or historical
  calculations are retained.
- Resetting the session clears all entered data and the latest result.

### Configuration-driven calculations

- No tax rate, threshold, ceiling, rebate limit, surcharge rate, cess rate,
  or deduction limit may be hard-coded in calculation logic.
- All such values must come from the common configuration file.
- A missing required configuration rule must produce a clear error.

### Separation of concerns

- Income modules calculate head-level taxable income.
- The deduction module evaluates category-level claims.
- The central calculator applies cross-module rules.
- The report generator formats an already calculated result.
- The report generator must never recalculate or modify amounts.

## Central calculation ownership

Only `TaxCalculator` may apply:

- Gross Total Income aggregation
- Combined 80C, 80CCC, and 80CCD(1) ceiling
- Overall Chapter VI-A GTI cap
- Total-income rounding
- Slab tax
- Special-rate tax
- Rebate
- Surcharge
- Cess
- Payment credits
- Final balance or refund

Individual income modules must not apply these rules independently.

## Income-head rules

### Salary

- Salary income is entered as annual amounts.
- Family pension is not part of salary income.
- Employer NPS under Section 80CCD(2) remains identifiable for the
  deduction module.
- The module does not determine exemption eligibility.

### House property

- The module calculates annual value and applies the configured
  standard deduction.
- It does not determine ownership, co-ownership, letting status,
  or legal eligibility.
- Pre-construction interest is entered separately and remains subject to
  configuration and eligibility checks.

### Business or profession

- The user enters computed taxable profit or loss.
- The application does not calculate depreciation, stock valuation,
  partner remuneration, partner interest, presumptive income, or audit status.

### Capital gains

- The user selects the capital-gains category and enters taxable gain or loss.
- The application does not calculate consideration, cost, transfer expenses,
  indexation, exemptions, or holding period.
- Category-specific treatment must be configuration-driven.

### Other sources

- Ordinary receipts and special-income receipts are kept separate.
- Interest categories remain separately available for deduction calculations.
- Family pension is evaluated once for the annual combined amount.
- Dividend income remains identifiable for later component-specific handling.
- Online-game and VDA amounts are user-declared taxable amounts.
- The application does not infer online-game net winnings or VDA income
  from bank credits or transaction values.

### Exempt income

- Exempt income is report-only.
- It is never added to Gross Total Income.
- Agricultural income is retained separately for rate-purpose checks.
- The module does not determine legal exemption eligibility.

## Deduction rules

- Each deduction category maps to a numeric configuration code.
- The deduction module applies category-level absolute and percentage limits.
- It does not apply the GTI cap.
- It does not merge 80C, 80CCC, and 80CCD(1).
- The central calculator applies their combined ceiling and the GTI cap.
- 80TTA and 80TTB must use interest totals from the Other Sources module.
- The application must prevent double-counting interest deductions.

## Payment rules

- TDS, TCS, advance tax, and self-assessment tax are separate credit types.
- They are subtracted only after total tax liability, including cess.
- Any excess credit becomes a refund amount.
- The application does not verify credit eligibility or year-wise availability.
- The application does not calculate interest for late payment or deferment.

## Current limitations

The following areas are intentionally not complete and must not be
represented as fully calculated:

- Agricultural-income rate-purpose calculation
- Capital-gains transaction-level computation
- Business depreciation and related adjustments
- Presumptive taxation
- Partner remuneration and interest
- VDA transaction-level computation
- Online-game net-winnings computation
- Set-off and carry-forward of losses
- AMT and AMTC
- Clubbing of income
- Income splitting or attribution
- Interest for late payment or deferment
- Interest for late filing
- Fee for late filing
- Document validation
- Return preparation or filing
- Legal eligibility determination

## Required future work

Before production use, implement and test:

1. A complete agricultural-income rate-purpose mechanism.
2. Configuration-driven capital-gains category treatment.
3. Explicit capital-gains transaction inputs and calculations.
4. Business-income depreciation and adjustment rules.
5. Set-off and carry-forward rules.
6. AMT and AMTC where applicable.
7. Clubbing-of-income rules where applicable.
8. A fully specified surcharge marginal-relief formula.
9. Explicit validation that 80TTA and 80TTB do not exceed eligible interest.
10. Component-specific dividend tax handling.
11. Comprehensive unit tests for every configuration path.
12. Integration tests for every income-head combination.
13. Error-path tests for missing, invalid, and conflicting configuration.
14. Rounding tests at every calculation stage.
15. Report tests for all supported taxpayer categories, regimes, and age groups.

## Testing requirements

Every calculation rule must have tests for:

- Zero income
- Income below the basic exemption threshold
- Income exactly at each slab boundary
- Income just above each slab boundary
- Maximum deduction claims
- Claims exceeding configured limits
- GTI lower than total deductions
- Rebate eligibility and ineligibility
- Each surcharge threshold
- Special-rate income with and without slab income
- TDS/TCS credits lower than, equal to, and higher than liability
- Invalid dates
- Missing configuration rules
- Duplicate income heads
- Duplicate receipts and credits

## Coding standards

- Use C++17 or later.
- Validate every user-entered numeric amount for finiteness and non-negativity.
- Use explicit enumeration validation in every public entry point.
- Do not use floating-point values for identifiers or rule codes.
- Keep user-facing messages clear and non-technical where possible.
- Do not include taxpayer identity data in calculation results or reports.
- Do not add persistence without explicitly revising the session-only design.
