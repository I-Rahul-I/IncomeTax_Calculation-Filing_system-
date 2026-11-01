# Configuration Format

The application uses one plain-text configuration file.

Each non-empty, non-comment line has the form:

```text
KEY = VALUE
```

Comments begin with `#`.

Example:

```text
# Individual taxpayers are supported.
SUPPORT_INDIVIDUAL = YES
```

Values are interpreted as:

- `YES` / `TRUE` / `1` for boolean true
- `NO` / `FALSE` / `0` for boolean false
- Numeric values use a period as the decimal separator
- Multi-part entries use pipe (`|`) separators

## Session and taxpayer support

```text
SUPPORT_INDIVIDUAL = YES
SUPPORT_HUF = YES
SUPPORT_TDS_TCS = YES
SUPPORT_ADVANCE_TAX = YES
SUPPORT_SELF_ASSESSMENT_TAX = YES
```

## Slabs

Slabs use this format:

```text
SLAB|TAXPAYER_TYPE|AGE_GROUP|REGIME|FROM|TO|RATE|DESCRIPTION
```

Fields:

- `TAXPAYER_TYPE`: `INDIVIDUAL` or `HUF`
- `AGE_GROUP`: `BELOW_60`, `SENIOR_60_79`, or `SUPER_SENIOR_80_PLUS`
- `REGIME`: `NEW` or `OLD`
- `FROM`: lower income bound, inclusive
- `TO`: upper income bound, inclusive; use `-1` for no upper limit
- `RATE`: percentage rate
- `DESCRIPTION`: report description

Example:

```text
SLAB|INDIVIDUAL|BELOW_60|NEW|0|400000|0|Nil slab
SLAB|INDIVIDUAL|BELOW_60|NEW|400001|800000|5|Five percent slab
SLAB|INDIVIDUAL|BELOW_60|NEW|800001|-1|10|Ten percent slab
```

## Rebate

```text
REBATE_INCOME_LIMIT = 700000
REBATE_MAXIMUM = 25000
```

## Cess

```text
HEALTH_AND_EDUCATION_CESS_RATE = 4
```

## Surcharge

Surcharge rules use this format:

```text
SURCHARGE|TAXPAYER_TYPE|THRESHOLD|RATE|DESCRIPTION
```

Example:

```text
SURCHARGE|INDIVIDUAL|5000000|10|Income above 50 lakh
SURCHARGE|INDIVIDUAL|10000000|15|Income above 1 crore
SURCHARGE|INDIVIDUAL|20000000|25|Income above 2 crore
SURCHARGE|INDIVIDUAL|50000000|37|Income above 5 crore
```

Surcharge controls:

```text
SURCHARGE_INCLUDES_SPECIAL_TAX = NO
SURCHARGE_MARGINAL_RELIEF = YES
```

## Family pension

```text
FAMILY_PENSION_DEDUCTION_PERCENT = 33.33
FAMILY_PENSION_DEDUCTION_MAX = 25000
```

## Deductions

Deduction rules use this format:

```text
DEDUCTION|CODE|MAX_AMOUNT|MAX_PERCENT|DESCRIPTION
```

Fields:

- `CODE`: numeric deduction code
- `MAX_AMOUNT`: absolute ceiling; use `-1` for no absolute ceiling
- `MAX_PERCENT`: percentage ceiling of the claimed amount; use `0` for no percentage ceiling
- `DESCRIPTION`: report description

The application maps deduction categories to these codes:

| Category  | Code |
| --------- | ---: |
| 80C       |  801 |
| 80CCC     |  802 |
| 80CCD(1)  |  803 |
| 80CCD(1B) |  804 |
| 80CCD(2)  |  805 |
| 80CCG     |  806 |
| 80D       |  807 |
| 80DD      |  808 |
| 80DDB     |  809 |
| 80E       |  810 |
| 80EE      |  811 |
| 80EEA     |  812 |
| 80EEB     |  813 |
| 80G       |  814 |
| 80GGB     |  815 |
| 80GGC     |  816 |
| 80QQB     |  817 |
| 80RRB     |  818 |
| 80TTA     |  819 |
| 80TTB     |  820 |
| 80U       |  821 |

Example:

```text
DEDUCTION|801|150000|0|Section 80C deduction
DEDUCTION|804|50000|0|Additional NPS deduction
DEDUCTION|807|25000|0|Medical insurance premium
DEDUCTION|819|10000|0|Savings-account interest deduction
DEDUCTION|820|50000|0|Interest income deduction for eligible senior citizens
```

Combined 80C, 80CCC, and 80CCD(1) ceiling:

```text
DEDUCTION_80C_80CCC_80CCD1_LIMIT = 150000
```

## Special-income rules

Special-income rules use this format:

```text
SPECIAL|CODE|RATE|DESCRIPTION
```

Required codes:

```text
SPECIAL|115BB|30|Lottery, crossword puzzle, race including horse race, card game and other game of any sort, gambling or betting
SPECIAL|115BBJ|30|Online games net winnings
SPECIAL|115BBH|30|Virtual digital asset income
```

## Minimum demonstration configuration

```text
SUPPORT_INDIVIDUAL = YES
SUPPORT_HUF = YES
SUPPORT_TDS_TCS = YES
SUPPORT_ADVANCE_TAX = YES
SUPPORT_SELF_ASSESSMENT_TAX = YES

SLAB|INDIVIDUAL|BELOW_60|NEW|0|400000|0|Nil slab
SLAB|INDIVIDUAL|BELOW_60|NEW|400001|800000|5|Five percent slab
SLAB|INDIVIDUAL|BELOW_60|NEW|800001|1200000|10|Ten percent slab
SLAB|INDIVIDUAL|BELOW_60|NEW|1200001|-1|15|Fifteen percent slab

REBATE_INCOME_LIMIT = 700000
REBATE_MAXIMUM = 25000

HEALTH_AND_EDUCATION_CESS_RATE = 4

SURCHARGE|INDIVIDUAL|5000000|10|Income above 50 lakh
SURCHARGE|INDIVIDUAL|10000000|15|Income above 1 crore
SURCHARGE|INDIVIDUAL|20000000|25|Income above 2 crore
SURCHARGE|INDIVIDUAL|50000000|37|Income above 5 crore

SURCHARGE_INCLUDES_SPECIAL_TAX = NO
SURCHARGE_MARGINAL_RELIEF = YES

FAMILY_PENSION_DEDUCTION_PERCENT = 33.33
FAMILY_PENSION_DEDUCTION_MAX = 25000

DEDUCTION|801|150000|0|Section 80C deduction
DEDUCTION|802|150000|0|Section 80CCC deduction
DEDUCTION|803|150000|0|Section 80CCD(1) deduction
DEDUCTION|804|50000|0|Section 80CCD(1B) deduction
DEDUCTION|807|25000|0|Section 80D deduction
DEDUCTION|819|10000|0|Section 80TTA deduction
DEDUCTION|820|50000|0|Section 80TTB deduction

DEDUCTION_80C_80CCC_80CCD1_LIMIT = 150000

SPECIAL|115BB|30|Specified special income
SPECIAL|115BBJ|30|Online games net winnings
SPECIAL|115BBH|30|Virtual digital asset income
```

## Configuration rules

- Every rule used by an enabled module must be present.
- A missing required rule must cause a clear error.
- Slabs must be entered in ascending income order.
- A slab with `TO = -1` must be the final slab for its taxpayer, age group, and regime.
- Deduction ceilings of `-1` mean no absolute ceiling.
- Deduction percentage ceilings of `0` mean no percentage-based ceiling.
- Special-income rules must exist for every special-income category entered by the user.
- The configuration file must not contain taxpayer names, PANs, addresses, or other personal identity data.
