# User Workflow

The application supports one annual income-tax computation at a time.

All entered data belongs only to the current session. Closing the
application discards the data. The application does not retain
prior-year information, permanent taxpayer profiles, or uploaded documents.

## Workflow overview

1. Load the configuration file.
2. Select the taxpayer category and regime.
3. Enter income under the applicable heads.
4. Enter Chapter VI-A deduction claims.
5. Enter TDS, TCS, advance tax, and self-assessment tax.
6. Run the calculation.
7. Review the computation report.
8. Start a new session or close the application.

## Step 1: Load configuration

The user must load the common configuration file before entering data.

The configuration supplies:

- Slab rates and thresholds
- Rebate settings
- Surcharge rules
- Cess rate
- Deduction limits
- Special-income rates
- Family-pension adjustment settings
- Enabled taxpayer categories and payment types

The application must not proceed if a required rule is missing.

## Step 2: Select taxpayer

The current model supports:

- Individual
- HUF

The user selects:

- Taxpayer category
- Applicable regime
- Age group, where relevant

The application does not ask for or store:

- Name
- PAN
- Address
- Contact details
- Bank-account details
- Passwords
- Prior-year return data

## Step 3: Enter income

The user adds one income head at a time. Only one object per income head
is allowed.

### Salary

Enter:

- Gross salary
- Exempt allowances, only if eligible
- Professional tax, only if eligible
- Employer NPS contribution under Section 80CCD(2), only if eligible

The user must not enter the same receipt in more than one field.

### House property

Enter:

- Gross annual value or rent received
- Municipal taxes paid
- Interest on borrowed capital
- Pre-construction interest, only where eligible

The application calculates the annual value and applies the configured
standard deduction. It does not determine property ownership, co-ownership,
letting status, or legal eligibility.

### Business or profession

Enter the computed taxable profit or loss.

The application does not calculate:

- Depreciation
- Stock valuation
- Partner remuneration or interest
- Presumptive income
- Books-of-account requirements
- Audit requirements

### Capital gains

The capital-gains module accepts one transaction per capital-gains head.
Enter the asset type, term, treatment, dates, actual consideration, costs,
transfer expenses, and any independently verified eligible exemption.

For land/building, an optional stamp-duty value is collected and a simplified
Section 50C substitution is applied when it exceeds actual consideration by
more than 10%. This route does not model agreement-date versus registration-date
exceptions or a valuation-officer reference. Section 112A grandfathered cost is
supported when the user supplies the 31-Jan-2018 FMV. Specified debt mutual funds
acquired on/after 1-Apr-2023 use the Section 50AA deemed-short-term/normal-rate
route. These routes still require user verification of facts and eligibility.

Capital-loss set-off and carry-forward are not completed in this module;
losses are retained as pending adjustments and are not offset against other
income. User-entered exemption claims are subtracted from positive gain but the
software does not verify the exemption's legal eligibility or qualifying
investment. The module does not determine the correct holding period
automatically for every asset type.

### Other sources

Enter ordinary receipts separately:

- Savings-account interest
- Fixed-deposit interest
- Recurring-deposit interest
- Other deposit interest
- Bond interest
- Dividend income
- Family pension
- Other ordinary income

Enter special-income categories separately:

- Lottery and similar winnings
- Online-game net winnings
- Virtual-digital-asset income

For online games and virtual digital assets, enter the taxable amount
determined under the applicable rules. Do not enter gross withdrawals,
deposits, or sale value as taxable income.

### Exempt income

Enter only the amount claimed to be exempt.

The application reports exempt income separately. It does not add it to
Gross Total Income.

Agricultural income is retained separately because it may require a
separate rate-purpose check.

## Step 4: Enter deductions

Enter each Chapter VI-A claim separately.

The user enters the claimed amount. The application applies configured
category limits.

The central calculator later applies:

- The combined 80C, 80CCC, and 80CCD(1) ceiling
- The overall Chapter VI-A GTI cap

The application does not determine whether a deduction is legally available.

## Step 5: Enter payments and credits

Enter:

- TDS credit
- TCS credit
- Advance tax paid
- Self-assessment tax paid

These are credits against the final liability. They are not income and
must not be entered as deductions.

The application does not verify credit eligibility or year-wise availability.

## Step 6: Calculate

The central calculator performs the following order:

1. Evaluates every income head.
2. Aggregates normal-rate taxable income into Gross Total Income.
3. Applies Chapter VI-A category limits.
4. Applies the combined 80C, 80CCC, and 80CCD(1) ceiling.
5. Applies the Chapter VI-A GTI cap.
6. Calculates total income and rounds it to the nearest rupee.
7. Calculates slab tax.
8. Calculates special-rate tax separately.
9. Applies rebate.
10. Applies surcharge.
11. Applies health and education cess.
12. Subtracts TDS, TCS, advance tax, and self-assessment tax.
13. Shows either balance tax payable or refund.

## Step 7: Review report

The report shows:

- Income-head details
- Gross Total Income
- Deductions and adjustments
- Total income
- Slab tax
- Special-rate tax
- Rebate
- Surcharge
- Cess
- Payment credits
- Balance payable or refund
- Important notes and limitations

The report is a computation summary. It is not tax advice, a legal
eligibility determination, or a filed return.

## Data-entry safeguards

The user must ensure that:

- Each receipt is entered only once.
- Taxable and exempt portions are not mixed in one amount.
- TDS/TCS is not entered as income or deduction.
- Capital-gains categories are correctly selected.
- Online-game and VDA amounts are taxable amounts, not gross transaction values.
- Agricultural income is entered separately from other exempt income.
- Family pension is entered once as an annual combined amount.

## Session reset

The user may reset the session at any time.

Resetting clears:

- Taxpayer selection
- All income-head entries
- Deduction claims
- Payment credits
- The latest calculation result

It does not archive, export, upload, or retain any data.


## Annual aggregation and presumptive routes (revision)

- Enter capital-gain transactions separately; the annual calculator aggregates
  them and applies the supported STCL/LTCL set-off sequence before tax.
- For a business/profession, select normal accounts, Section 44AD, or Section
  44ADA. The presumptive flow asks for cash receipts and any higher declared
  profit; do not also enter expenses/depreciation for that same presumptive
  activity.
- Special-rate income contributes to total income but is not re-taxed through
  normal slab rates. Section 112A's annual threshold is applied after aggregation.
