# Calculation Route Audit (AY 2026-27 / FY 2025-26)

This revision expands annual aggregation and calculation paths beyond the prior
single-transaction capital-gains implementation. It is a calculation aid; a
professional return-preparation engine must still verify statutory eligibility,
source documents, filing deadlines, and fact-dependent exceptions.

## Implemented / corrected in this revision

- **Multiple capital-gain transaction entries:** capital gains may be entered as
  separate transactions and aggregated for the annual calculation.
- **Capital-loss set-off within the year:** STCL is set off first against STCG,
  then LTCG; LTCL is set off only against remaining LTCG. Unabsorbed amounts are
  surfaced as carry-forward candidates rather than deducted from ordinary income.
- **Correct total-income / tax bases:** special-rate income is included in GTI and
  total income, but excluded from slab-tax calculation so it is not double-taxed
  at normal slab rates. Chapter VI-A deductions are capped against normal-rate
  income in this model.
- **Section 112A annual threshold:** the ₹1.25 lakh threshold is applied once to
  the aggregate annual gain after permitted capital loss set-off. The resident
  Individual/HUF unused basic-exemption adjustment is applied where relevant.
- **Section 50C:** stamp-duty value substitution uses the configured 10% tolerance
  for land/building transactions.
- **Section 112A grandfathered acquisition cost:** `max(actual cost, min(31-Jan-2018
  FMV, sale consideration))` is applied for eligible pre-1-Feb-2018 acquisitions.
- **Section 50AA:** specified debt mutual funds acquired on/after 1-Apr-2023 are
  validated as deemed short-term with normal-rate treatment.
- **Sections 44AD / 44ADA:** presumptive income paths are available in the business
  entry flow. 44AD uses 6% for qualifying non-cash receipts and 8% for cash
  receipts; 44ADA uses the 50% floor. Eligibility/receipt thresholds are checked
  against entered values.
- **Current-year inter-head loss set-off:** business loss is not set off against
  salary; house-property inter-head set-off is capped at ₹2 lakh in the old regime
  and not applied under the new regime. Residual losses are identified as
  carry-forward candidates.
- **Section 87A marginal relief:** configured new-regime marginal relief is applied
  above the rebate threshold for eligible resident Individuals.
- **Surcharge on special-rate tax:** special-rate tax is included in the surcharge
  computation, with the configured surcharge cap applied to specified capital-gain
  tax.
- **Build and regression tests:** C++17 build plus route-focused tests for Section
  50C, Section 112A grandfathering/annual threshold and capital-loss set-off, and
  presumptive 44AD/44ADA calculations.

## Still requires case-specific evidence / separate functionality

The following are not silently guessed or treated as universally eligible:

- Persistent prior-year loss ledgers, statutory carry-forward duration/return-filing
  tests, and unabsorbed depreciation brought forward across assessment years.
- All STT acquisition/transfer exceptions and every asset-specific holding-period
  rule (including special business-trust or corporate-action cases).
- Section 50C agreement-date vs registration-date alternatives and valuation-officer
  reference process.
- Property transitional indexed-vs-unindexed comparison for qualifying pre-23-Jul-2024
  acquisitions; asset-block depreciation schedules and business accounting adjustments.
- Full statutory eligibility/limits and reinvestment validation for Sections 54/54B/
  54EC/54F/54GB and other exemption claims. The existing exemption amount is user-entered.
- Detailed Section 80D/80G/NPS/other deduction eligibility beyond configured ceilings;
  complete presumptive-tax audits, books/audit consequences, and all section-specific
  exceptions.
- Complete Sections 234A/234B/234C interest calculation, which needs return due date,
  payment timing, special-income timing exceptions, and assessed-tax-specific data.

## Build

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/*.cpp -o income_tax_app
```


## Chapter VI-A deduction routes added in this revision

Every deduction category exposed in the console menu now has an explicit rule key in both AY 2026-27 regime files, preventing the prior `Missing deduction rule` exception. Newly represented categories include 80CCG, 80DDB, 80EE, 80EEA, 80EEB, 80GGB, 80GGC, 80QQB, and 80RRB. The rule files mark deductions not available under the new regime or not applicable to the application's Individual/HUF taxpayer support as disallowed rather than leaving them absent.

Section 80DDB now collects patient age and reimbursements, subtracts reimbursed amounts, and applies the configured ₹40,000 ceiling or ₹1,00,000 senior-patient ceiling. The user-entered amount must be for specified-disease treatment and the remaining statutory conditions must be met; those eligibility facts are self-declared, not independently verified by the application. Other new routes apply their configured section ceilings and taxpayer/regime eligibility checks; detailed document/loan/grant/political-party or royalty qualification checks are not independently validated by the software.
