# Release Checklist

Use this checklist before releasing the income-tax computation application.

An item is complete only when it is implemented, tested, and reviewed.

## 1. Architecture

- [ ] The application uses one session for one annual computation.
- [ ] Closing the application discards all session data.
- [ ] No prior-year data is retained.
- [ ] No permanent taxpayer profile is retained.
- [ ] No document upload or document storage exists.
- [ ] Resetting the session clears taxpayer, income, deductions, payments, and result.
- [ ] Income-head objects are owned by the application session.
- [ ] Only one object per income head is permitted.
- [ ] The report generator does not recalculate or alter calculated amounts.

## 2. Configuration

- [ ] The application loads one common configuration file.
- [ ] All tax rates are configuration-driven.
- [ ] All thresholds are configuration-driven.
- [ ] All deduction ceilings are configuration-driven.
- [ ] All special-income rates are configuration-driven.
- [ ] Surcharge rules are configuration-driven.
- [ ] Rebate rules are configuration-driven.
- [ ] Cess rate is configuration-driven.
- [ ] Family-pension adjustment settings are configuration-driven.
- [ ] Missing required configuration rules produce clear errors.
- [ ] Invalid configuration values produce clear errors.
- [ ] Unknown configuration keys are rejected or explicitly warned about.
- [ ] The configuration file contains no personal taxpayer data.

## 3. Taxpayer support

- [ ] Individual taxpayers are supported only when enabled.
- [ ] HUF taxpayers are supported only when enabled.
- [ ] Unsupported taxpayer categories are rejected.
- [ ] Taxpayer category, regime, and age group are validated.
- [ ] No name, PAN, address, contact detail, bank detail, or password is collected.
- [ ] No prior-year return data is collected.

## 4. Income modules

### Salary

- [ ] Gross salary is validated.
- [ ] Exempt allowances are entered separately.
- [ ] Professional tax is entered separately.
- [ ] Employer NPS under 80CCD(2) is identifiable.
- [ ] Family pension is not accepted as salary income.
- [ ] Salary income is not double-counted with other heads.

### House property

- [ ] Annual value is calculated from entered inputs.
- [ ] Municipal taxes are handled according to configuration.
- [ ] Interest on borrowed capital is handled according to configuration.
- [ ] Standard deduction is configuration-driven.
- [ ] Pre-construction interest is separately identifiable.
- [ ] Ownership and co-ownership eligibility are not automatically assumed.

### Business or profession

- [ ] Taxable profit or loss is user-entered.
- [ ] Depreciation is not independently calculated.
- [ ] Stock valuation is not independently calculated.
- [ ] Partner remuneration and interest are not independently calculated.
- [ ] Presumptive income is not independently calculated.
- [ ] Audit and books-of-account requirements are not automatically determined.

### Capital gains

- [ ] Capital-gains category is explicitly selected.
- [ ] Taxable gain or loss is user-entered.
- [ ] Category-specific treatment is configuration-driven.
- [ ] Consideration, cost, expenses, indexation, and exemptions are not inferred.
- [ ] Holding-period classification is not automatically assumed.

### Other sources

- [ ] Ordinary and special-income entries are separate.
- [ ] Interest categories are separately retained.
- [ ] Dividend income is separately retained.
- [ ] Family pension is entered as one annual combined amount.
- [ ] Family-pension deduction is applied only once.
- [ ] Lottery and similar winnings require configuration rule 115BB.
- [ ] Online-game income requires configuration rule 115BBJ.
- [ ] VDA income requires configuration rule 115BBH.
- [ ] Online-game and VDA taxable amounts are not inferred from gross receipts.
- [ ] TDS/TCS is not subtracted from gross income entries.

### Exempt income

- [ ] Exempt income is reported separately.
- [ ] Exempt income is not included in GTI.
- [ ] Agricultural income is separately identifiable.
- [ ] Agricultural-income rate-purpose treatment is not automatically applied.
- [ ] Zero-value exempt entries are rejected.
- [ ] Taxable portions are not entered as exempt income.

## 5. Deductions

- [ ] Every deduction category maps to a configured rule code.
- [ ] Claimed and allowed amounts are reported separately.
- [ ] Category-level absolute ceilings are applied.
- [ ] Category-level percentage ceilings are applied.
- [ ] The combined 80C, 80CCC, and 80CCD(1) ceiling is applied by the calculator.
- [ ] The Chapter VI-A GTI cap is applied by the calculator.
- [ ] Deductions do not exceed GTI.
- [ ] 80TTA and 80TTB use Other Sources interest totals.
- [ ] Interest deductions are not double-counted.
- [ ] Deduction eligibility is not automatically determined.

## 6. Payments

- [ ] TDS, TCS, advance tax, and self-assessment tax are separate.
- [ ] Payment dates are validated.
- [ ] Credits are not entered as income or deductions.
- [ ] Credits are subtracted only after cess.
- [ ] Excess credits produce a refund amount.
- [ ] Credit eligibility is not automatically determined.
- [ ] Interest for late payment or deferment is not calculated.

## 7. Calculation

- [ ] All income heads are evaluated before aggregation.
- [ ] GTI includes only normal-rate taxable income.
- [ ] Exempt income is excluded from GTI.
- [ ] Total income is rounded to the nearest rupee.
- [ ] Slab tax is calculated only from slab income.
- [ ] Special-rate tax is calculated separately.
- [ ] Rebate is applied according to configuration.
- [ ] Surcharge is applied according to configuration.
- [ ] Surcharge marginal relief is fully specified and tested.
- [ ] Cess is applied after rebate and surcharge.
- [ ] Payment credits are applied after cess.
- [ ] The final result shows either balance payable or refund.
- [ ] No calculation stage produces a negative tax liability.

## 8. Reports

- [ ] Reports show income-head details.
- [ ] Reports show GTI.
- [ ] Reports show deductions and adjustments.
- [ ] Reports show total income.
- [ ] Reports show slab tax, special-rate tax, rebate, surcharge, and cess.
- [ ] Reports show payment credits.
- [ ] Reports show balance payable or refund.
- [ ] Reports include calculation limitations.
- [ ] Reports include a clear non-advice disclaimer.
- [ ] Reports contain no taxpayer identity data.
- [ ] Plain-text and Markdown formats produce consistent amounts.

## 9. Testing

- [ ] Unit tests exist for every public validation rule.
- [ ] Unit tests exist for every configuration-driven calculation.
- [ ] Tests cover zero, boundary, and excess-income cases.
- [ ] Tests cover each slab boundary.
- [ ] Tests cover deduction limits and GTI cap.
- [ ] Tests cover rebate eligibility and ineligibility.
- [ ] Tests cover every surcharge threshold.
- [ ] Tests cover special-income categories.
- [ ] Tests cover payment credits below, equal to, and above liability.
- [ ] Tests cover invalid dates and invalid amounts.
- [ ] Tests cover missing configuration rules.
- [ ] Tests cover duplicate income heads.
- [ ] Tests cover duplicate receipts and credits.
- [ ] Tests cover rounding at every calculation stage.
- [ ] Tests cover all supported taxpayer categories, regimes, and age groups.

## 10. Release restrictions

Do not release the application unless:

- [ ] Every unsupported calculation is clearly disclosed.
- [ ] The application does not claim to determine legal eligibility.
- [ ] The application does not claim to prepare or file a return.
- [ ] The application does not provide tax advice.
- [ ] The application does not retain data beyond the active session.
- [ ] The application does not upload or transmit user data.
- [ ] The application does not hard-code tax values.
- [ ] The application does not silently ignore missing rules.
- [ ] The application does not silently apply unsupported assumptions.
