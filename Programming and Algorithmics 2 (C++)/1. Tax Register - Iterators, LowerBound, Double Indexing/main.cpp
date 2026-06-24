#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <compare>
#include <functional>
#include <stdexcept>
#include <algorithm>
#endif /* __PROGTEST__ */

struct CitizenRec
{
    std::string name;
    std::string addr;
    std::string accountId;

    int sumIncome;
    int sumExpense;

    CitizenRec(const std::string_view& _name, const std::string_view& _addr, 
               const std::string_view& _accountId) 
                : name(_name), addr(_addr), accountId(_accountId), 
                sumIncome(0), sumExpense(0) {}
    
    static bool CompareByName(const CitizenRec* r, const std::pair<std::string_view, std::string_view>& key)
    {
        if (r->name < key.first)
            return true;
        if (r->name > key.first)
            return false;
        return r->addr < key.second;
    }
    static bool CompareByAccount(const CitizenRec* r, const std::string_view& key)
    {
        return r->accountId < key;
    }
};

class CIterator
{
private:

    std::vector<const CitizenRec*> DB;
    size_t idx;

public:
    
    CIterator(const std::vector<const CitizenRec*>& _DB)
    : DB(_DB), idx(0) {}

    bool atEnd() const
    {
        return idx >= DB.size();
    }
    void next()
    {
        if (!atEnd())
            idx++;
    }
    const std::string& name() const
    {
        return DB[idx]->name;
    }
    const std::string& addr() const
    {
        return DB[idx]->addr;
    }
    const std::string& account () const
    {
        return DB[idx]->accountId;
    }
};

class CTaxRegister
{
private:

    std::vector<CitizenRec*> DB_name;
    std::vector<CitizenRec*> DB_account;

    /* helpers */
    using nameIterator = std::vector<CitizenRec*>::const_iterator;
    using accountIterator = std::vector<CitizenRec*>::const_iterator;

    nameIterator FindByName(const std::pair<std::string_view, std::string_view>& key) const
    {
        return std::lower_bound(DB_name.begin(), DB_name.end(), key, 
                                CitizenRec::CompareByName);
    }

    accountIterator FindByAccount(const std::string_view& acc) const
    {
        return std::lower_bound(DB_account.begin(), DB_account.end(), acc, 
                                CitizenRec::CompareByAccount);
    }

public:

    CTaxRegister() = default;

    ~CTaxRegister()
    {
        for (auto& rec : DB_name)
            delete rec;
    }
    
    bool birth(const std::string& name, const std::string& addr, const std::string& account)
    {
        /* check if there's (name, address) match */
        std::pair<std::string_view, std::string_view> key (name, addr);
        auto itName = FindByName(key);
        if (itName != DB_name.end() && ((*itName)->name == name && (*itName)->addr == addr))
            return false;
        
        /* check if there's accountId match */
        auto itAcc = FindByAccount(account);
        if (itAcc != DB_account.end() && (*itAcc)->accountId == account)
            return false;
        
        /* add into database(s) */
        CitizenRec* rec = new CitizenRec(name, addr, account);
        DB_name.insert(itName, rec);
        DB_account.insert(itAcc, rec);
        return true;
    }

    bool death(const std::string& name, const std::string& addr)
    {
        /* check if there's (name, address) match */
        std::pair<std::string_view, std::string_view> key(name, addr);
        auto itName = FindByName(key);
        if (itName == DB_name.end() || ((*itName)->name != name || (*itName)->addr != addr))
            return false;

        /* find iterator in DB_account based on pointer found */
        CitizenRec* temp = (*itName);
        auto itAcc = FindByAccount(temp->accountId);
        if (itAcc == DB_account.end() || (*itAcc) != temp)
            return false;
        
        DB_name.erase(itName);
        DB_account.erase(itAcc);
        delete temp;

        return true;
    }

    bool income(const std::string& account, int amount)
    {
        /* check if there's accountId match */
        auto it = FindByAccount(account);
        if (it == DB_account.end() || (*it)->accountId != account)
            return false;
        (*it)->sumIncome += amount;
        return true;
    }

    bool income(const std::string& name, const std::string& addr, int amount)
    {
        /* check if there's (name, address) match */
        std::pair<std::string_view, std::string_view> key (name, addr);
        auto it = FindByName(key);
        if (it == DB_name.end() || ((*it)->name != name || (*it)->addr != addr))
            return false;
        
        (*it)->sumIncome += amount;
        return true;
    }

    bool expense(const std::string& account, int amount)
    {
        /* check if there's accountId match */
        auto it = FindByAccount(account);
        if (it == DB_account.end() || (*it)->accountId != account)
            return false;
        (*it)->sumExpense += amount;
        return true;
    }


    bool expense(const std::string& name, const std::string& addr, int amount)
    {
        /* check if there's (name, address) match */
        std::pair<std::string_view, std::string_view> key (name, addr);
        auto it = FindByName(key);
        if (it == DB_name.end() || ((*it)->name != name || (*it)->addr != addr))
            return false;

        (*it)->sumExpense += amount;
        return true;
    }

    bool audit(const std::string& name, const std::string& addr, std::string& account, 
               int& sumIncome, int& sumExpense) const
    {
        /* check if there's (name, address) match */
        std::pair<std::string_view, std::string_view> key (name, addr);
        auto it = FindByName(key);
        if (it == DB_name.end() || ((*it)->name != name || (*it)->addr != addr))
            return false;
        
        account = (*it)->accountId;
        sumIncome = (*it)->sumIncome;
        sumExpense = (*it)->sumExpense;
        return true;
    }
    
    CIterator listByName() const
    {
        std::vector<const CitizenRec*> DB_iterator;
        for (const auto& rec : DB_name)
            DB_iterator.push_back(rec);
        return CIterator(DB_iterator);
    }
};

#ifndef __PROGTEST__
int main()
{
  std::string acct;
  int sumIncome, sumExpense;
  CTaxRegister b0;
  assert ( b0 . birth ( "John Smith", "Oak Road 23", "123/456/789" ) );
  assert ( b0 . birth ( "Jane Hacker", "Main Street 17", "Xuj5#94" ) );
  assert ( b0 . birth ( "Peter Hacker", "Main Street 17", "634oddT" ) );
  assert ( b0 . birth ( "John Smith", "Main Street 17", "Z343rwZ" ) );
  assert ( b0 . income ( "Xuj5#94", 1000 ) );
  assert ( b0 . income ( "634oddT", 2000 ) );
  assert ( b0 . income ( "123/456/789", 3000 ) );
  assert ( b0 . income ( "634oddT", 4000 ) );
  assert ( b0 . income ( "Peter Hacker", "Main Street 17", 2000 ) );
  assert ( b0 . expense ( "Jane Hacker", "Main Street 17", 2000 ) );
  assert ( b0 . expense ( "John Smith", "Main Street 17", 500 ) );
  assert ( b0 . expense ( "Jane Hacker", "Main Street 17", 1000 ) );
  assert ( b0 . expense ( "Xuj5#94", 1300 ) );
  assert ( b0 . audit ( "John Smith", "Oak Road 23", acct, sumIncome, sumExpense ) );
  assert ( acct == "123/456/789" );
  assert ( sumIncome == 3000 );
  assert ( sumExpense == 0 );
  assert ( b0 . audit ( "Jane Hacker", "Main Street 17", acct, sumIncome, sumExpense ) );
  assert ( acct == "Xuj5#94" );
  assert ( sumIncome == 1000 );
  assert ( sumExpense == 4300 );
  assert ( b0 . audit ( "Peter Hacker", "Main Street 17", acct, sumIncome, sumExpense ) );
  assert ( acct == "634oddT" );
  assert ( sumIncome == 8000 );
  assert ( sumExpense == 0 );
  assert ( b0 . audit ( "John Smith", "Main Street 17", acct, sumIncome, sumExpense ) );
  assert ( acct == "Z343rwZ" );
  assert ( sumIncome == 0 );
  assert ( sumExpense == 500 );
  CIterator it = b0 . listByName ();
  assert ( ! it . atEnd ()
           && it . name () == "Jane Hacker"
           && it . addr () == "Main Street 17"
           && it . account () == "Xuj5#94" );
  it . next ();
  assert ( ! it . atEnd ()
           && it . name () == "John Smith"
           && it . addr () == "Main Street 17"
           && it . account () == "Z343rwZ" );
  it . next ();
  assert ( ! it . atEnd ()
           && it . name () == "John Smith"
           && it . addr () == "Oak Road 23"
           && it . account () == "123/456/789" );
  it . next ();
  assert ( ! it . atEnd ()
           && it . name () == "Peter Hacker"
           && it . addr () == "Main Street 17"
           && it . account () == "634oddT" );
  it . next ();
  assert ( it . atEnd () );

  assert ( b0 . death ( "John Smith", "Main Street 17" ) );

  CTaxRegister b1;
  assert ( b1 . birth ( "John Smith", "Oak Road 23", "123/456/789" ) );
  assert ( b1 . birth ( "Jane Hacker", "Main Street 17", "Xuj5#94" ) );
  assert ( !b1 . income ( "634oddT", 4000 ) );
  assert ( !b1 . expense ( "John Smith", "Main Street 18", 500 ) );
  assert ( !b1 . audit ( "John Nowak", "Second Street 23", acct, sumIncome, sumExpense ) );
  assert ( !b1 . death ( "Peter Nowak", "5-th Avenue" ) );
  assert ( !b1 . birth ( "Jane Hacker", "Main Street 17", "4et689A" ) );
  assert ( !b1 . birth ( "Joe Hacker", "Elm Street 23", "Xuj5#94" ) );
  assert ( b1 . death ( "Jane Hacker", "Main Street 17" ) );
  assert ( b1 . birth ( "Joe Hacker", "Elm Street 23", "Xuj5#94" ) );
  assert ( b1 . audit ( "Joe Hacker", "Elm Street 23", acct, sumIncome, sumExpense ) );
  assert ( acct == "Xuj5#94" );
  assert ( sumIncome == 0 );
  assert ( sumExpense == 0 );
  assert ( !b1 . birth ( "Joe Hacker", "Elm Street 23", "AAj5#94" ) );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
