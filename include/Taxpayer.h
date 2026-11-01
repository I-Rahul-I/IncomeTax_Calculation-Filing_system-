#ifndef TAXPAYER_H
#define TAXPAYER_H

#include <string>

using namespace std;

enum class TaxpayerType { Individual, HUF };
enum class ResidentialStatus { Resident, NonResident, ResidentButNotOrdinarilyResident };
enum class AgeCategory { Below60, SeniorCitizen, SuperSeniorCitizen };

class Taxpayer {
private:
    string name;
    string pan;
    TaxpayerType type;
    ResidentialStatus residentialStatus;
protected:
    Taxpayer(const string& name, const string& pan, TaxpayerType type, ResidentialStatus status);
public:
    virtual ~Taxpayer() = default;
    const string& getName() const;
    const string& getPAN() const;
    TaxpayerType getType() const;
    ResidentialStatus getResidentialStatus() const;
    void setName(const string& name);
    void setPAN(const string& pan);
    void setResidentialStatus(ResidentialStatus status);
    static string getFinancialYear();
    static string getAssessmentYear();
    virtual string getCategoryName() const = 0;
};
class IndividualTaxpayer : public Taxpayer {
private:
    int age;
public:
    IndividualTaxpayer(const string&, const string&, ResidentialStatus, int age);
    int getAge() const;
    void setAge(int age);
    AgeCategory getAgeCategory() const;
    string getAgeCategoryName() const;
    string getCategoryName() const override;
};
class HUFTaxpayer : public Taxpayer {
public:
    HUFTaxpayer(const string&, const string&, ResidentialStatus);
    string getCategoryName() const override;
};
class CompanyTaxpayer : public Taxpayer {
public:
    CompanyTaxpayer(const string&, const string&, ResidentialStatus);
    string getCategoryName() const override;
};
class AOPTaxpayer : public Taxpayer {
public:
    AOPTaxpayer(const string&, const string&, ResidentialStatus);
    string getCategoryName() const override;
};
class BOITaxpayer : public Taxpayer {
public:
    BOITaxpayer(const string&, const string&, ResidentialStatus);
    string getCategoryName() const override;
};
class AJPTaxpayer : public Taxpayer {
public:
    AJPTaxpayer(const string&, const string&, ResidentialStatus);
    string getCategoryName() const override;
};
string taxpayerTypeToString(TaxpayerType);
string residentialStatusToString(ResidentialStatus);
#endif
