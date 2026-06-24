#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <span>
#include <algorithm>
#include <memory>
#include <compare>
#include <complex>
#endif /* __PROGTEST__ */


namespace polyManip
{
    // // keep this dummy version if you do not implement a real manipulator
    // std::ios_base & ( * poly_var ( const std::string & name ) ) ( std::ios_base & x )
    // {
    //     return [] ( std::ios_base & ios ) -> std::ios_base & { return ios; };
    // }
    
    static int polyVarNameIndex = std::ios::xalloc();

    struct CustomVar
    {
        std::string var;
        CustomVar(const std::string& str) : var(str) {}
    };

    std::ostream& operator<<(std::ostream& os, const CustomVar& pv)
    {
        void* & p = os.pword(polyVarNameIndex);
        if (p)
            delete static_cast<std::string*>(p);

        p = new std::string(pv.var);
        return os;
    }

    CustomVar poly_var( const std::string & name )
    {
        return CustomVar(name);
    }
}

/* 
    CPolynomial Class:
        Represents a polynomial, 
        stores non-zero polynomials in pairs <exponent, coeficient> in ascending order,
        methods: multiplications, comparisons
*/
class CPolynomial
{
private:

std::vector<std::pair<unsigned, double>> polynom;

    /* 
        sets/updates the coefficient of a given exponent
        if coef parameters is 0, it removes the exponent from the polynom
    */
    void setCoef(unsigned exp, double coef)
    {
        std::pair<unsigned, double> key(exp, coef);
        auto it = std::lower_bound(polynom.begin(), polynom.end(), key, compareFn);
        
        if (coef == 0.0)
        {
            if (it != polynom.end() && it->first == exp)
                polynom.erase(it);
        }
        else
        {
            if (it == polynom.end() || it->first != exp)
                polynom.insert(it, key);
            else
                it->second = coef;
        }
    }

    /* returns coeficient for given exponent, 0 if not found */
    double getCoef(unsigned exp) const
    {
        std::pair<unsigned, double> key(exp, 0.);
        auto it = std::lower_bound(polynom.begin(), polynom.end(), key, compareFn);

        if (it == polynom.end() || it->first != exp)
            return 0.;
        else
            return it->second;
    }

    /* comparison function to feed std::lower_bound to keep data sorted*/
    static bool compareFn (const std::pair<unsigned, double> & a, const std::pair<unsigned, double> & b)
    {
        return a.first < b.first;
    }

    /* 
        helper class to allower operator[] get and set coefficients,
        I implemented this so there is no need to call some Normalize function in case of calling a[2] = 0
     */
    class Proxy
    {
    private:

        CPolynomial* poly;
        unsigned exp;

    public:

        /* constructor */
        Proxy(CPolynomial* p, unsigned e) : poly(p), exp(e) {}

        /* convert the proxy to double by reading the coef from the polynomial */
        operator double() const
        {
            return poly->getCoef(exp);
        }

        /* allowing assigning a double to exponent's coefficient */
        Proxy& operator=(double val)
        {
            poly->setCoef(exp, val);
            return *this;
        }

        /* operator+= support */
        Proxy& operator+=(double val)
        {
            double current = poly->getCoef(exp);
            poly->setCoef(exp, current + val);
            return *this;
        }
    };

public:
    
    /* default  */    
    CPolynomial() = default;

    /* 
        operator*(double d) - scalar multiplication, muls all coeficients by d
        returns new instance of CPolynomial
    */
    CPolynomial operator*(double d) const
    {
        CPolynomial res;
        if (d == 0)
        {
            return res;
        }

        for (auto p : polynom)
        {
            double temp = p.second * d;
            if (temp != 0.)    
                res.setCoef(p.first, temp);
        }
        return res;
    }

    /* 
        operator*(const CPolynomial& other) - polynomial multiplication
        returns new instance of CPolynomial
    */
    CPolynomial operator*(const CPolynomial& other) const
    {
        CPolynomial res;
        for (auto p1 : polynom)
            for (auto p2 : other.polynom)
            {
                unsigned exp = p1.first + p2.first;
                res.setCoef(exp, res.getCoef(exp) + p1.second * p2.second);
            }
        return res;
    }
    
    /* 
        operator*=(double d) - scalar multiplication, muls all coeficients by d 
        assigns value to current instance of CPolynomial
    */
    CPolynomial operator*=(double d)
    {
        return *this = *this * d;
    }
   
    /* 
        operator*=(const CPolynomial& other) - polynomial multiplication
        assigns value to current instance of CPolynomial
    */
    CPolynomial operator*=(const CPolynomial& other)
    {
        return *this = *this * other;
    }

    /* equality check, checks if all coeficients of exponents are equal */
    bool operator==(const CPolynomial& other) const
    {
        if (polynom.size() != other.polynom.size() || degree() != other.degree())
            return false;
        for (size_t i = 0; i < polynom.size(); i++)
            if (polynom[i] != other.polynom[i])
                return false;
        return true;
    }
    
    /* negation of operator== */
    bool operator!=(const CPolynomial& other) const
    {
        return !(*this == other);
    }
    
    /* read-only access to a coeficient of exponent idx */
    double operator[](unsigned idx) const
    {
        return getCoef(idx);
    }
    
    /* write-enabled access, returns Proxy - that can set or +- the coef */
    Proxy operator[](unsigned idx)
    {
        return Proxy(this, idx);
    }
    
    /* evaluates polynom based on input d */
    double operator()(double d) const
    {
        double res = 0;
        for (auto p : polynom)
            res += p.second * std::pow(d, p.first);
        return res;
    }

    /* returns true if polynomial is equal to 0 (all coeficients are 0) */
    bool operator!() const
    {
        for (auto p : polynom)
            if (p.second != 0.)
                return false;
        return true;
    }

    /* returns true if polynomial is non-zero */
    explicit operator bool() const
    {
        return !((*this).operator!());
    }

    /* returns degree of polynomial */
    unsigned degree() const
    {
        if (!(*this))
            return 0;
        return polynom.back().first;
    }
    
    /* operator<< to pass polynomial into ostream in descending order */
    friend std::ostream& operator<<(std::ostream& os, const CPolynomial& cp);

    /* compares polynom with vector equivalent */
    friend bool dumpMatch(const CPolynomial& x, const std::vector<double>& ref);

    /* debug */
    void print() const
    {
        for (auto p : polynom)
            std::cout << p.first << " : " << p.second << '\n';
    }
};
/* operator<< to pass polynomial into ostream in descending order */
std::ostream& operator<<(std::ostream& os, const CPolynomial& cp)
{
    if (!cp)
        return os << "0";

    void* p = os.pword(polyManip::polyVarNameIndex);
    std::string var = (p ? *static_cast<std::string*>(p) : "x");

    bool first = true;
    for (int i = static_cast<int>(cp.polynom.size()) - 1; i >= 0; i--)
    {
        unsigned exp = cp.polynom[i].first;
        double coef = cp.polynom[i].second;
        double abscoef = std::fabs(coef);

        if (first)
        {
            if (coef < 0.)
                os << "- ";
        }
        else
        {
            os << (coef < 0 ? " - " : " + ");
        }

        if (exp == 0)
        {
            os << abscoef;
        }
        else
        {
            if (abscoef != 1.)
                os << abscoef << "*";
            os << var << "^" << exp;
        }
        first = false;
    }
    return os;
}

/* needed to call poly_var w/o namespace prefix */
using namespace polyManip; 

#ifndef __PROGTEST__

/* approximity check of 2 double values */
bool smallDiff(double a, double b)
{
    return std::fabs(a - b) < 1e-6;
}

/* compares polynom with vector equivalent */
bool dumpMatch(const CPolynomial& x, const std::vector<double>& ref)
{
    std::vector<double> temp;
    if (!x)
    {
        temp.push_back(0.);
    }
    else
    {
        unsigned track = 0;
        for (size_t i = 0; i < x.polynom.size(); track++)
        {
            if (x.polynom[i].first != track)
                temp.push_back(0.);
            else
            {
                temp.push_back(x.polynom[i].second);
                i++;
            }
        }
    }
    return temp == ref;
}

/* basic tests */
int main ()
{
    CPolynomial a, b, c;
    std::ostringstream out, tmp;
        
    a[0] = -10;
    a[1] = 3.5;
    a[3] = 1;
    assert ( smallDiff ( a ( 2 ), 5 ) );
    out . str ("");
    out << a;
    assert ( out . str () == "x^3 + 3.5*x^1 - 10" );
    c = a * -2;
    assert ( c . degree () == 3
            && dumpMatch ( c, std::vector<double>{ 20.0, -7.0, -0.0, -2.0 } ) );

    out . str ("");
    out << c;
    assert ( out . str () == "- 2*x^3 - 7*x^1 + 20" );
    out . str ("");
    out << b;
    assert ( out . str () == "0" );
    b[5] = -1;
    b[2] = 3;
    out . str ("");
    out << b;
    assert ( out . str () == "- x^5 + 3*x^2" );
    c = a * b;
    assert ( c . degree () == 8);
    assert( dumpMatch ( c, std::vector<double>{ -0.0, -0.0, -30.0, 10.5, -0.0, 13.0, -3.5, 0.0, -1.0 } ) );

    out . str ("");

    out << c;
    assert ( out . str () == "- x^8 - 3.5*x^6 + 13*x^5 + 10.5*x^3 - 30*x^2" );
    a *= 5;
    assert ( a . degree () == 3
            && dumpMatch ( a, std::vector<double>{ -50.0, 17.5, 0.0, 5.0 } ) );

    a *= b;
    assert ( a . degree () == 8
            && dumpMatch ( a, std::vector<double>{ 0.0, 0.0, -150.0, 52.5, -0.0, 65.0, -17.5, -0.0, -5.0 } ) );

    assert ( a != b );
    b[5] = 0;
    assert ( static_cast<bool> ( b ) );
    assert ( ! ! b );
    b[2] = 0;
    assert ( !(a == b) );
    a *= 0;
    assert ( a . degree () == 0
            && dumpMatch ( a, std::vector<double>{ 0.0 } ) );

    assert ( a == b );
    assert ( ! static_cast<bool> ( b ) );
    assert ( ! b );

    // bonus - manipulators

    out . str ("");
    out << poly_var ( "y" ) << c;
    assert ( out . str () == "- y^8 - 3.5*y^6 + 13*y^5 + 10.5*y^3 - 30*y^2" );
    out . str ("");
    tmp << poly_var ( "abc" );
    out . copyfmt ( tmp );
    out << c;
    assert ( out . str () == "- abc^8 - 3.5*abc^6 + 13*abc^5 + 10.5*abc^3 - 30*abc^2" );
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
