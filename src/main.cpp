#include <fstream>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "ApplicationSession.h"
#include "InputHandler.h"
#include "SalaryIncome.h"
#include "OtherSourcesIncome.h"
#include "ExemptIncome.h"
#include "Payments.h"
#include "TaxComputationResult.h"
#include "TaxRules.h"
#include "Deductions.h"
#include "HousePropertyIncome.h"
#include "BusinessIncome.h"
#include "CapitalGainIncome.h"

using namespace std;

namespace {
void collectSalary(InputHandler& in, ApplicationSession& session) {
    auto salary = make_unique<SalaryIncome>("Annual salary");
    cout << "\nEnter annual salary components (₹; enter 0 if not applicable).\n";
    salary->setBasicSalary(in.readAmount("Basic salary: "));
    salary->setDearnessAllowance(in.readAmount("Dearness allowance: "));
    salary->setBonus(in.readAmount("Bonus: "));
    salary->setCommission(in.readAmount("Commission: "));
    salary->setHouseRentAllowance(in.readAmount("HRA received: "));
    salary->setLeaveTravelAllowance(in.readAmount("Leave travel allowance: "));
    salary->setOtherTaxableAllowances(in.readAmount("Other taxable allowances: "));
    salary->setTaxablePerquisites(in.readAmount("Taxable perquisites: "));
    salary->setLTAExemptionClaimed(in.readAmount("LTA exemption claimed (eligible amount): "));
    salary->setProfessionalTaxPaid(in.readAmount("Professional tax paid: "));
    session.addIncomeHead(move(salary));
}

void collectHouseProperty(InputHandler& in, ApplicationSession& session) {
    auto property = make_unique<HousePropertyIncome>();
    cout << "\nHouse property details (whole property amounts; ownership share is applied by the calculator).\n"
              << "1 Self-occupied\n2 Let-out\n3 Deemed let-out\n";
    property->setPropertyUse(static_cast<PropertyUse>(in.readInteger("Property use (1-3): ", 1, 3)));
    property->setOwnershipPercent(in.readAmount("Ownership percentage (0-100): "));
    property->setGrossAnnualValue(in.readAmount("Gross annual value / rent (₹): "));
    property->setMunicipalTaxesPaid(in.readAmount("Eligible municipal taxes paid (₹): "));
    property->setCurrentYearInterest(in.readAmount("Current-year eligible loan interest (₹): "));
    property->setPreConstructionInterestInstallment(in.readAmount("Eligible pre-construction interest instalment (₹): "));
    cout << "Self-occupied interest category:\n1 Higher-limit eligible (confirm conditions)\n2 Other eligible loan\n";
    property->setSelfOccupiedInterestCategory(static_cast<SelfOccupiedInterestCategory>(in.readInteger("Choose (1-2): ", 1, 2)));
    session.addIncomeHead(move(property));
}

void collectBusiness(InputHandler& in, ApplicationSession& session) {
    auto business = make_unique<BusinessIncome>();
    cout << "\nBusiness/professional income details.\n1 Business\n2 Profession\n";
    business->setActivityType(static_cast<BusinessActivityType>(in.readInteger("Choose (1-2): ", 1, 2)));
    business->setActivityName(in.readText("Activity name: "));
    business->setNatureOfActivity(in.readText("Nature of activity: "));
    business->setGrossReceipts(in.readAmount("Gross receipts (₹): "));
    cout << "Computation method:\n1 Normal accounts\n2 Section 44AD presumptive business\n3 Section 44ADA presumptive profession\n";
    const int method = in.readInteger("Choose (1-3): ", 1, 3);
    business->setComputationMethod(static_cast<BusinessComputationMethod>(method));
    if (method != 1) {
        business->setCashReceipts(in.readAmount("Cash receipts included in gross receipts (₹): "));
        business->setClaimedPresumptiveProfit(in.readAmount("Higher profit voluntarily declared (₹; 0 if not): "));
        session.addIncomeHead(move(business));
        return;
    }
    business->setOtherBusinessIncome(in.readAmount("Other business income (₹): "));
    bool addExpense = true;
    while (addExpense) {
        BusinessExpense expense;
        expense.description = in.readText("Expense description: ");
        expense.amount = in.readAmount("Expense amount (₹): ");
        expense.disallowedAmount = in.readAmount("Portion disallowed (₹): ");
        expense.explanation = in.readText("Reason/note (optional): ", true);
        business->addExpense(expense);
        addExpense = in.readYesNo("Add another expense entry? (y/n): ");
    }
    business->setBookDepreciation(in.readAmount("Book depreciation (₹): "));
    business->setTaxDepreciation(in.readAmount("Eligible tax depreciation (₹): "));
    session.addIncomeHead(move(business));
}

void collectCapitalGain(InputHandler& in, ApplicationSession& session) {
    auto gain = make_unique<CapitalGainIncome>();
    cout << "\nCapital-gain details (one transaction entry).\n"
              << "Asset: 1 Listed equity 2 Equity MF 3 Business trust 4 Land/building 5 Gold 6 Debt MF 7 Unlisted share 8 Other\n";
    gain->setAssetType(static_cast<CapitalAssetType>(in.readInteger("Asset type (1-8): ", 1, 8)));
    cout << "Term: 1 Short-term 2 Long-term\n";
    gain->setGainTerm(static_cast<CapitalGainTerm>(in.readInteger("Choose (1-2): ", 1, 2)));
    cout << "Tax treatment: 1 Normal rate 2 Section 111A 3 Section 112 4 Section 112A\n";
    gain->setTaxTreatment(static_cast<CapitalGainTaxTreatment>(in.readInteger("Choose (1-4): ", 1, 4)));
    gain->setAssetDescription(in.readText("Asset description: "));
    gain->setAcquisitionDate(in.readText("Acquisition date YYYY-MM-DD: "));
    gain->setTransferDate(in.readText("Transfer date YYYY-MM-DD: "));
    gain->setSaleConsideration(in.readAmount("Sale consideration (₹): "));
    if (gain->getTaxTreatment() == CapitalGainTaxTreatment::Section112A &&
        gain->getAcquisitionDate() < "2018-02-01") {
        gain->setGrandfatheredFMV(in.readAmount(
            "Fair market value as of 2018-01-31 for Section 112A (₹): "
        ));
    }
    if (gain->getAssetType() == CapitalAssetType::LandOrBuilding) {
        gain->setStampDutyValue(in.readAmount(
            "Stamp-duty value for Section 50C (₹; enter 0 if not applicable/unknown): "
        ));
    }
    gain->setTransferExpenses(in.readAmount("Transfer expenses (₹): "));
    gain->setAcquisitionCost(in.readAmount("Acquisition cost (₹): "));
    gain->setImprovementCost(in.readAmount("Improvement cost (₹): "));
    const double exemption = in.readAmount("Eligible exemption amount (₹; 0 if none): ");
    const string section = exemption > 0 ? in.readText("Exemption section/reference: ") : "";
    gain->setExemptionClaim(exemption, section);
    const bool isSecurity =
        gain->getAssetType() == CapitalAssetType::ListedEquityShare ||
        gain->getAssetType() == CapitalAssetType::EquityOrientedMutualFund ||
        gain->getAssetType() == CapitalAssetType::BusinessTrustUnit;
    if (isSecurity) {
        gain->setSTTPaidOnAcquisition(in.readYesNo("STT paid on acquisition? (Y/N): "));
        gain->setSTTPaidOnTransfer(in.readYesNo("STT paid on transfer? (Y/N): "));
    }
    session.addIncomeHead(move(gain));
}

void collectOtherSources(InputHandler& in, ApplicationSession& session) {
    auto other = make_unique<OtherSourcesIncome>();
    cout << "\nEnter annual income from other sources.\n";
    bool addOrdinary = true;
    while (addOrdinary) {
        cout << "Ordinary income category:\n"
                  << "1 Savings interest\n2 Fixed deposit interest\n"
                  << "3 Recurring deposit interest\n4 Other deposit interest\n"
                  << "5 Bond interest\n6 Dividend\n7 Family pension\n8 Other ordinary income\n";
        const int type = in.readInteger("Choose (1-8): ", 1, 8);
        OtherSourceEntry entry;
        entry.type = static_cast<OtherSourceType>(type);
        entry.description = in.readText("Description: ");
        entry.amount = in.readAmount("Taxable amount: ");
        other->addOrdinaryEntry(entry);
        addOrdinary = in.readYesNo("Add another ordinary income entry? (y/n): ");
    }
    bool addSpecial = true;
    while (addSpecial) {
        cout << "Special entry category:\n1 Lottery/similar winnings\n"
                  << "2 Online-game net winnings\n3 Virtual digital asset income\n";
        const int type = in.readInteger("Choose (1-3): ", 1, 3);
        SpecialOtherSourceEntry entry;
        entry.type = static_cast<SpecialOtherSourceType>(type);
        entry.description = in.readText("Description: ");
        entry.taxableAmount = in.readAmount("Separately determined taxable amount: ");
        entry.relevantDate = in.readText("Relevant date YYYY-MM-DD (blank if not applicable): ", true);
        other->addSpecialEntry(entry);
        addSpecial = in.readYesNo("Add another special-rate entry? (y/n): ");
    }
    session.addIncomeHead(move(other));
}

void collectExemptIncome(InputHandler& in, ApplicationSession& session) {
    auto exempt = make_unique<ExemptIncome>();
    bool addExempt = true;
    while (addExempt) {
        cout << "Exempt-income category:\n1 Agricultural income\n2 Scholarship\n"
                  << "3 PPF interest\n4 Eligible insurance proceeds\n5 Other declared exempt income\n";
        const int type = in.readInteger("Choose (1-5): ", 1, 5);
        ExemptIncomeEntry entry;
        entry.type = static_cast<ExemptIncomeType>(type);
        entry.description = in.readText("Description: ");
        entry.amount = in.readAmount("Declared exempt amount: ");
        entry.exemptionReference = in.readText("Section/reference (optional): ", true);
        entry.remarks = in.readText("Remarks (optional): ", true);
        exempt->addEntry(entry);
        addExempt = in.readYesNo("Add another exempt-income entry? (y/n): ");
    }
    session.addIncomeHead(move(exempt));
}


void collectDeductions(InputHandler& in, ApplicationSession& session) {
    cout << "\nEnter Chapter VI-A deduction claims. "
              << "The engine applies regime eligibility and configured limits.\n"
              << "If you have no deduction claims, choose 0 to skip.\n";
    bool addDeduction = true;
    while (addDeduction) {
        cout << "Deduction category:\n"
                  << "1 80C\n2 80CCC\n3 80CCD(1)\n4 80CCD(1B)\n5 80CCD(2)\n"
                  << "6 80CCG\n7 80D\n8 80DD\n9 80DDB\n10 80E\n"
                  << "11 80EE\n12 80EEA\n13 80EEB\n14 80G\n15 80GGB\n"
                  << "16 80GGC\n17 80QQB\n18 80RRB\n19 80TTA\n20 80TTB\n21 80U\n"
                  << "0 No deductions / finish deduction entry\n";
        const int type = in.readInteger("Choose (0-21): ", 0, 21);
        if (type == 0) {
            break;
        }

        DeductionClaim claim;
        claim.type = static_cast<DeductionType>(type);
        claim.description = Deductions::getDeductionTypeName(claim.type);
        if (claim.type == DeductionType::Section80DDB) {
            cout << "For Section 80DDB, enter eligible medical expenses before reimbursement.\n";
            claim.claimedAmount = in.readAmount("Specified-disease medical expenses (Rs): ");
            claim.patientAge = in.readInteger("Patient age (years): ", 1, 120);
            claim.reimbursementAmount = in.readAmount("Medical expenses reimbursed by insurance/employer (Rs): ");
            if (claim.reimbursementAmount > claim.claimedAmount) {
                throw invalid_argument("Reimbursement cannot exceed medical expenses.");
            }
        } else {
            claim.claimedAmount = in.readAmount("Amount claimed: ");
        }
        claim.remarks = in.readText("Eligibility/supporting note (optional): ", true);
        session.getDeductions().addClaim(claim);
        addDeduction = in.readYesNo("Add another deduction claim? (y/n): ");
    }
}
void collectPayments(InputHandler& in, ApplicationSession& session) {
    cout << "\nEnter tax payments/credits (not income).\n";
    bool addAnother = true;
    while (addAnother) {
        cout << "\nSelect payment/credit type:\n"
                  << "1 TDS\n2 TCS\n3 Advance tax\n4 Self-assessment tax\n"
                  << "0 Finish payment/credit entry\n";
        const int type = in.readInteger("Choose (0-4): ", 0, 4);
        if (type == 0) break;

        PaymentEntry entry;
        entry.type = static_cast<PaymentType>(type);
        entry.description = in.readText("Description: ");
        entry.amount = in.readAmount("Credit/payment amount: ");
        entry.paymentDate = in.readText("Payment date YYYY-MM-DD (optional): ", true);
        entry.reference = in.readText("Reference (optional): ", true);
        session.getPayments().addEntry(entry);

        addAnother = in.readYesNo("Add another payment/credit entry? (y/n): ");
    }
}
}

int main() {
    try {
        InputHandler input;
        input.displayWelcome();

        ApplicationSession session;
        session.loadRules("config/common_tax_rules_AY_2026_27.txt",
                          "config/new_regime_AY_2026_2027.txt");

        auto taxpayer = input.collectTaxpayerDetails();
        session.setTaxpayer(move(taxpayer));
        input.displayTaxpayerSummary(session.getTaxpayer());

        const bool isHUF = session.getTaxpayer().getType() == TaxpayerType::HUF;
        cout << "\nIncome and credits are entered once; the application will calculate both regimes.\n";
        if (isHUF) {
            cout << "HUF workflow: salary and house-property heads are not collected in this application.\n";
        }
        bool finished = false;
        while (!finished) {
            if (isHUF) {
                cout << "\nAdd HUF income details (choose 0 when finished):\n"
                          << "1 Business/profession\n2 Capital gains\n"
                          << "3 Other sources\n4 Exempt income (reported separately)\n"
                          << "0 Finish income entry\n";
                const int choice = input.readInteger("Choose (0-4): ", 0, 4);
                switch (choice) {
                    case 1:
                        if (session.hasIncomeHead(IncomeHead::BusinessProfession))
                            cout << "Business/profession head already entered; skipping duplicate.\n";
                        else collectBusiness(input, session);
                        break;
                    case 2:
                        if (session.hasIncomeHead(IncomeHead::CapitalGains))
                            cout << "Capital-gains head already entered; skipping duplicate.\n";
                        else collectCapitalGain(input, session);
                        break;
                    case 3:
                        if (session.hasIncomeHead(IncomeHead::OtherSources))
                            cout << "Other-sources head already entered; skipping duplicate.\n";
                        else collectOtherSources(input, session);
                        break;
                    case 4:
                        if (session.hasIncomeHead(IncomeHead::ExemptIncome))
                            cout << "Exempt-income head already entered; skipping duplicate.\n";
                        else collectExemptIncome(input, session);
                        break;
                    case 0: finished = true; break;
                }
            } else {
                cout << "\nAdd income details (choose 0 when finished):\n"
                          << "1 Salary\n2 House property\n3 Business/profession\n"
                          << "4 Capital gains\n5 Other sources\n6 Exempt income (reported separately)\n"
                          << "0 Finish income entry\n";
                const int choice = input.readInteger("Choose (0-6): ", 0, 6);
                switch (choice) {
                    case 1:
                        if (session.hasIncomeHead(IncomeHead::Salary))
                            cout << "Salary head already entered; skipping duplicate.\n";
                        else collectSalary(input, session);
                        break;
                    case 2:
                        if (session.hasIncomeHead(IncomeHead::HouseProperty))
                            cout << "House-property head already entered; skipping duplicate.\n";
                        else collectHouseProperty(input, session);
                        break;
                    case 3:
                        if (session.hasIncomeHead(IncomeHead::BusinessProfession))
                            cout << "Business/profession head already entered; skipping duplicate.\n";
                        else collectBusiness(input, session);
                        break;
                    case 4:
                        if (session.hasIncomeHead(IncomeHead::CapitalGains))
                            cout << "Capital-gains head already entered; skipping duplicate.\n";
                        else collectCapitalGain(input, session);
                        break;
                    case 5:
                        if (session.hasIncomeHead(IncomeHead::OtherSources))
                            cout << "Other-sources head already entered; skipping duplicate.\n";
                        else collectOtherSources(input, session);
                        break;
                    case 6:
                        if (session.hasIncomeHead(IncomeHead::ExemptIncome))
                            cout << "Exempt-income head already entered; skipping duplicate.\n";
                        else collectExemptIncome(input, session);
                        break;
                    case 0: finished = true; break;
                }
            }
        }

        if (session.getIncomeHeadCount() == 0) {
            cout << "No taxable income head was entered. Adding a zero-value fallback income head.\n";
            if (isHUF) {
                auto other = make_unique<OtherSourcesIncome>("Other sources");
                session.addIncomeHead(move(other));
            } else {
                auto salary = make_unique<SalaryIncome>("Annual salary");
                session.addIncomeHead(move(salary));
            }
        }

        collectDeductions(input, session);
        collectPayments(input, session);

        // Compute the new regime first, then reload old-regime rules while
        // retaining the exact same taxpayer, income, deductions, and credits.
        session.loadRules("config/common_tax_rules_AY_2026_27.txt",
                          "config/new_regime_AY_2026_2027.txt",
                          RegimeType::New);
        const TaxComputationResult newResult = session.calculate();
        const string newReport = session.generateReport(ReportFormat::PlainText);

        session.loadRules("config/common_tax_rules_AY_2026_27.txt",
                          "config/old_regime_AY_2026_27.txt",
                          RegimeType::Old);
        const TaxComputationResult oldResult = session.calculate();
        const string oldReport = session.generateReport(ReportFormat::PlainText);

        ofstream newFile("report_new_regime.txt");
        if (!newFile) throw runtime_error("Could not create report_new_regime.txt");
        newFile << newReport;
        newFile.close();

        ofstream oldFile("report_old_regime.txt");
        if (!oldFile) throw runtime_error("Could not create report_old_regime.txt");
        oldFile << oldReport;
        oldFile.close();

        cout << "\n================ REGIME COMPARISON ================\n";
        cout << "New regime balance tax payable: Rs. " << newResult.balanceTaxPayable
                  << " | estimated refund: Rs. " << newResult.refund << "\n";
        cout << "Old regime balance tax payable: Rs. " << oldResult.balanceTaxPayable
                  << " | estimated refund: Rs. " << oldResult.refund << "\n";
        if (newResult.balanceTaxPayable < oldResult.balanceTaxPayable) {
            cout << "Lower calculated balance: New Regime\n";
        } else if (oldResult.balanceTaxPayable < newResult.balanceTaxPayable) {
            cout << "Lower calculated balance: Old Regime\n";
        } else {
            cout << "Both regimes have the same calculated balance.\n";
        }
        cout << "\nReports saved: report_old_regime.txt and report_new_regime.txt\n";
        cout << "Note: These reports reflect only the income heads and deductions supported and entered in this application.\n";
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
