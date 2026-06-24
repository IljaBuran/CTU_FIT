#ifndef __PROGTEST__
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <memory>
#include <compare>
#include <functional>
#include <optional>
#include <cmath>
#include <unordered_map>

class CTimeStamp
{
private:
    
    int year, month, day, hour, minute;
    double seconds;

public:
  
    CTimeStamp(int year_, int month_, int day_,
               int hour_, int minute_, double sec_) 
               : year(year_), month(month_), day(day_), 
                 hour(hour_), minute(minute_), seconds(sec_) {}
    
    int compare(const CTimeStamp& x) const
    {
        auto t1 = std::tie(year, month, day, hour, minute, seconds);
        auto t2 = std::tie(x.year, x.month, x.day, x.hour, x.minute, x.seconds);

        if (t1 < t2)
            return -1;
        else if (t1 > t2)
            return 1;
        return 0;
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    const CTimeStamp& x)
    {
        int secondsInt = static_cast<int>(x.seconds);
        int milisecondsInt = static_cast<int>(std::round((x.seconds - secondsInt) * 1000));
    
        return (os << std::setw(4) << std::setfill('0') << x.year << '-'
                   << std::setw(2) << x.month << '-'
                   << std::setw(2) << x.day << ' '
                   << std::setw(2) << x.hour << ':'
                   << std::setw(2) << x.minute << ':'
                   << std::setw(2) << secondsInt << '.'
                   << std::setw(3) << milisecondsInt);
    }
};

class CMail
{
private:
    
    CTimeStamp _timeStamp;
    std::string _from;
    std::string _to;
    std::optional<std::string> _subject;

public:
    
    CMail(const CTimeStamp& timeStamp_,
          const std::string& from_,
          const std::string& to_,
          const std::optional<std::string>& subject_)
          : _timeStamp(timeStamp_), _from(from_), _to(to_), _subject(subject_) {}

    int compareByTime(const CTimeStamp& x) const
    {
        return _timeStamp.compare(x);
    }

    int compareByTime(const CMail& x) const
    {
        return _timeStamp.compare(x._timeStamp);
    }

    const std::string& from() const { return _from; }

    const std::string& to() const { return _to; }

    const std::optional<std::string>& subject() const { return _subject; }

    const CTimeStamp& timeStamp() const { return _timeStamp; }

    friend std::ostream& operator<<(std::ostream& os,
                                    const CMail& x)
    {
        os << x._timeStamp << ' ' << x._from << " -> " << x._to;
        if (x._subject.has_value())
            os << ", subject: " << x._subject.value();
        return os;
    }
};
// your code will be compiled in a separate namespace
namespace MysteriousNamespace 
{
#endif /* __PROGTEST__ */
//----------------------------------------------------------------------------------------
class CMailLog
{
private:

    /* custom comparator for std::map to keep database sorted */
    struct Cmp
    {
        inline bool operator()(const CTimeStamp& a, const CTimeStamp& b) const
        {
            return (a.compare(b) < 0);
        }
    };

    /* sorted mails */
    std::map<CTimeStamp, std::vector<CMail>, Cmp> database;

public:
    
    /* tries to parse messages from stream, connect them and insert into database */
    int parseLog(std::istream &in)
    {
        int res = 0;

        /* temporary struct to be used with temporary std::unordered map */
        struct PartialMailInfo
        {
            std::string from;
            std::optional<std::string> subject;
        };

        /* temporary container to store partial emails */
        std::unordered_map<std::string, PartialMailInfo> temp;
        std::string logLine;

        /* read line by line */
        while (std::getline(in, logLine))
        {
            if (logLine.empty())
                continue;
            
            /* declarations */
            std::istringstream iss(logLine);
            int day, year, hour, minute;
            double seconds;
            std::string monthStr, timeStr, dns, mailId, message;
            
            /* read from iss and assign values to corresponding vars */
            if (!(iss >> monthStr >> day >> year >> timeStr >> dns >> mailId))
                continue;
            
            /* get rid of ':' at the end of mailID */
            if (mailId.back() != ':')
                continue;
            
            mailId.pop_back();
            
            if (!IsMailIdValid(mailId))
                continue;
            
            /* get rid of leading zeros and extract message */
            while (iss.peek() == ' ')
                iss.get();
            std::getline(iss, message);
            
            /* read the message, cuts the prefix and processes */
            if (message.starts_with("from="))
                temp[mailId].from = message.substr(5);
            else if (message.starts_with("subject="))
                temp[mailId].subject = message.substr(8);
            else if (message.starts_with("to="))
            {
                message = message.substr(3);

                /* parse time and create CTimeStamp based on it */
                if (!ParseTime(timeStr, hour, minute, seconds))
                    continue;
                CTimeStamp timeStamp(year, ConvMonth(monthStr), day, hour, minute, seconds);

                /* create new CMail instance and insert into database */
                CMail newMail(timeStamp, temp[mailId].from, message, temp[mailId].subject);
                database[timeStamp].push_back(newMail);
                res++;
            }
        }
        return res;
    }

    std::list<CMail> listMail(const CTimeStamp& from,
                              const CTimeStamp& to) const
    {
        std::list<CMail> res;
        
        /* find corresponding interval based on parameters */
        auto it = database.lower_bound(from);
        auto itEnd = database.upper_bound(to);

        /* iterate through and insert mails into list */
        for ( ; it != itEnd; it++)
            for (const auto& mail : it->second)    
                res.push_back(mail);
        
        return res;
    }

    std::set<std::string> activeUsers(const CTimeStamp& from,
                                      const CTimeStamp& to) const
    {
        std::set<std::string> res;

        /* find corresponding interval based on parameters */
        auto it = database.lower_bound(from);
        auto itEnd = database.upper_bound(to);

        /* copy users for all emails into set */
        for ( ; it != itEnd; it++)
            for (const auto& mail : it->second)
            {
                res.insert(mail.from());
                res.insert(mail.to());
            }

        return res;
    }

private:

/* Helpers */

    /* Straightforward function returns number of a month */
    int ConvMonth(const std::string_view& str) const
    {
        if (str == "Jan") return 1;
        if (str == "Feb") return 2;
        if (str == "Mar") return 3;
        if (str == "Apr") return 4;
        if (str == "May") return 5;
        if (str == "Jun") return 6;
        if (str == "Jul") return 7;
        if (str == "Aug") return 8;
        if (str == "Sep") return 9;
        if (str == "Oct") return 10;
        if (str == "Nov") return 11;
        if (str == "Dec") return 12;
        else return -1;
    }

    bool IsMailIdValid(const std::string_view& str)
    {
        for (char c : str)
            if (!std::isalnum(c))
                return false;
        return true;
    }

    /* Converts time values and assigns to corresponding parameters */
    bool ParseTime(const std::string& timeStr_, int& hour_, 
                   int& minute_, 
                   double& seconds_)
    {
        std::istringstream iss(timeStr_);
        std::string hour, minute, seconds;
        
        if (!std::getline(iss, hour, ':'))
            return false;
        if (!std::getline(iss, minute, ':'))
            return false;
        if (!std::getline(iss, seconds))
            return false;
        try
        {
            hour_ = std::stoi(hour);
            minute_ = std::stoi(minute);
            seconds_ = std::stod(seconds);
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }
};
//----------------------------------------------------------------------------------------
#ifndef __PROGTEST__
} // namespace
std::string printMail(const std::list<CMail>& all)
{
    std::ostringstream oss;
    for (const auto& mail : all)
        oss << mail << "\n";
    return oss.str();
}
int main()
{
    MysteriousNamespace::CMailLog m;
    std::list<CMail> mailList;
    std::set<std::string> users;
    std::istringstream iss;

    iss . clear ();
    iss . str (
        "Mar 29 2025 12:35:32.233 relay.fit.cvut.cz ADFger72343D: from=user1@fit.cvut.cz\n"
        "Mar 29 2025 12:37:16.234 relay.fit.cvut.cz JlMSRW4232Df: from=person3@fit.cvut.cz\n"
        "Mar 29 2025 12:55:13.023 relay.fit.cvut.cz JlMSRW4232Df: subject=New progtest homework!\n"
        "Mar 29 2025 13:38:45.043 relay.fit.cvut.cz Kbced342sdgA: from=office13@fit.cvut.cz\n"
        "Mar 29 2025 13:36:13.023 relay.fit.cvut.cz JlMSRW4232Df: to=user76@fit.cvut.cz\n"
        "Mar 29 2025 13:55:31.456 relay.fit.cvut.cz KhdfEjkl247D: from=PR-department@fit.cvut.cz\n"
        "Mar 29 2025 14:18:12.654 relay.fit.cvut.cz Kbced342sdgA: to=boss13@fit.cvut.cz\n"
        "Mar 29 2025 14:48:32.563 relay.fit.cvut.cz KhdfEjkl247D: subject=Business partner\n"
        "Mar 29 2025 14:58:32.000 relay.fit.cvut.cz KhdfEjkl247D: to=HR-department@fit.cvut.cz\n"
        "Mar 29 2025 14:25:23.233 relay.fit.cvut.cz ADFger72343D: mail undeliverable\n"
        "Mar 29 2025 15:02:34.231 relay.fit.cvut.cz KhdfEjkl247D: to=CIO@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=CEO@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=dean@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=vice-dean@fit.cvut.cz\n"
        "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=archive@fit.cvut.cz\n"
        "Mar 29 2025 23:59:59.001 relay.fit.cvut.cz KhdfEjkl247D: to=archive@fit.cvut.cz\n" );
    assert ( m . parseLog ( iss ) == 9 );
    mailList = m . listMail ( CTimeStamp ( 2025, 3, 28, 0, 0, 0 ),
                                CTimeStamp ( 2025, 3, 29, 23, 59, 59 ) );
    #ifdef DEBUG
    std::cout << printMail(mailList);
    #endif /* DEBUG */
    assert ( printMail ( mailList ) == R"###(2025-03-29 13:36:13.023 person3@fit.cvut.cz -> user76@fit.cvut.cz, subject: New progtest homework!
2025-03-29 14:18:12.654 office13@fit.cvut.cz -> boss13@fit.cvut.cz
2025-03-29 14:58:32.000 PR-department@fit.cvut.cz -> HR-department@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> CEO@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> dean@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> vice-dean@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> archive@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.231 PR-department@fit.cvut.cz -> CIO@fit.cvut.cz, subject: Business partner
)###" );
    mailList = m . listMail ( CTimeStamp ( 2025, 3, 28, 0, 0, 0 ),
                                CTimeStamp ( 2025, 3, 29, 14, 58, 32 ) );
    assert ( printMail ( mailList ) == R"###(2025-03-29 13:36:13.023 person3@fit.cvut.cz -> user76@fit.cvut.cz, subject: New progtest homework!
2025-03-29 14:18:12.654 office13@fit.cvut.cz -> boss13@fit.cvut.cz
2025-03-29 14:58:32.000 PR-department@fit.cvut.cz -> HR-department@fit.cvut.cz, subject: Business partner
)###" );
    mailList = m . listMail ( CTimeStamp ( 2025, 3, 30, 0, 0, 0 ),
                                CTimeStamp ( 2025, 3, 30, 23, 59, 59 ) );
    assert ( printMail ( mailList ) == "" );
    users = m . activeUsers ( CTimeStamp ( 2025, 3, 28, 0, 0, 0 ),
                                CTimeStamp ( 2025, 3, 29, 23, 59, 59 ) );
    assert ( users == std::set<std::string>( { "CEO@fit.cvut.cz", "CIO@fit.cvut.cz", "HR-department@fit.cvut.cz", "PR-department@fit.cvut.cz", "archive@fit.cvut.cz", "boss13@fit.cvut.cz", "dean@fit.cvut.cz", "office13@fit.cvut.cz", "person3@fit.cvut.cz", "user76@fit.cvut.cz", "vice-dean@fit.cvut.cz" } ) );
    users = m . activeUsers ( CTimeStamp ( 2025, 3, 28, 0, 0, 0 ),
                                CTimeStamp ( 2025, 3, 29, 13, 59, 59 ) );
    assert ( users == std::set<std::string>( { "person3@fit.cvut.cz", "user76@fit.cvut.cz" } ) );
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
