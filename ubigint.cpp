// $Id: ubigint.cpp,v 1.12 2016-04-04 13:07:41-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

#include "util.h"
#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that)
{
   if(that==0)
      ubig_value.push_back(0);
   for(unsigned long temp = that; temp>0; temp/=10)
      ubig_value.push_back(temp%10);
   //DEBUGF ('~', this << " -> " << uvalue)
}

ubigint::ubigint (const string& that): ubig_value(0) {
   for (string::size_type i = that.size(); i>0; --i) {
      if (not isdigit (that[i-1])) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      ubig_value.push_back(that[i-1] - '0');
      //uvalue = uvalue * 10 + digit - '0';
   }
}

ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint result;
   ubigint addthat;
   if(ubig_value > that.ubig_value)
   {
      result.ubig_value = ubig_value;
      addthat.ubig_value = that.ubig_value;
   }
   else
   {
      result.ubig_value = that.ubig_value;
      addthat.ubig_value = ubig_value;
   }
   int overflow = 0;
   for(unsigned int i=0; i<addthat.ubig_value.size(); ++i)
   {
      result.ubig_value[i] += addthat.ubig_value[i] + overflow;
      overflow = 0;
      
      if(result.ubig_value[i] > 9)
      {
         result.ubig_value[i] -= 10;
         overflow = 1;
      }
   }
   if(overflow)
      result.ubig_value.push_back(1);
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result;
   result.ubig_value = ubig_value;
   int overflow = 0;
   for(unsigned int i=0; i<that.ubig_value.size(); ++i)
   {
      int temp = result.ubig_value[i] - (that.ubig_value[i] + overflow);
      
      overflow = 0;
      
      if(temp < 0)
      {
         temp += 10;
         overflow = 1;
      }
      result.ubig_value[i] = temp;
   }
   for(unsigned int i=that.ubig_value.size(); i<ubig_value.size() && overflow; ++i)
   {
      int temp = result.ubig_value[i] - overflow;
      overflow = 0;
      
      if(temp < 0)
      {
         temp += 10;
         overflow = 1;
      }
      result.ubig_value[i] = temp;
   }
   while(result.ubig_value.size() > 1 && result.ubig_value[result.ubig_value.size()-1] == 0)
      result.ubig_value.pop_back();
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result;
   int overflow = 0;
   int d = 0;
   for(unsigned int i=0; i<ubig_value.size(); ++i)
   {
      overflow = 0;
      for(unsigned int j=0; j<that.ubig_value.size(); ++j)
      {
         while(result.ubig_value.size() <= i+j)
            result.ubig_value.push_back(0);
         d = result.ubig_value[i+j] + ubig_value[i]*that.ubig_value[j] + overflow;
         result.ubig_value[i+j] = d % 10;
         overflow = d / 10;
      }
      while(result.ubig_value.size() <= i+that.ubig_value.size())
         result.ubig_value.push_back(0);
      result.ubig_value[i+that.ubig_value.size()] = overflow;
   }
   while(result.ubig_value.size() > 1 && result.ubig_value[result.ubig_value.size()-1] == 0)
      result.ubig_value.pop_back();
   return result;
}

void ubigint::multiply_by_2() {
   if(ubig_value.size() > 1 && ubig_value[ubig_value.size()-1] == 0) cout << "PROBLEM*2\n";
   int overflow = 0;
   for(unsigned int i=0; i<ubig_value.size(); ++i)
   {
      ubig_value[i] *= 2;
      ubig_value[i] += overflow;
      overflow = 0;
      
      if(ubig_value[i] > 9)
      {
         ubig_value[i] -= 10;
         overflow = 1;
      }
   }
   if(overflow)
      ubig_value.push_back(1);
}

void ubigint::divide_by_2() {
   if(ubig_value.size() > 1 && ubig_value[ubig_value.size()-1] == 0) cout << "PROBLEM/2\n";
   for(unsigned int i=0; i<ubig_value.size(); ++i)
   {
      ubig_value[i] /= 2;
      if(i+1 < ubig_value.size() && ubig_value[i+1]%2==1)
         ubig_value[i] += 5;
   }
   while(ubig_value.size() > 1 && ubig_value[ubig_value.size()-1] == 0)
      ubig_value.pop_back();
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, ubigint divisor) {
   // Note: divisor is modified so pass by value (copy).
   ubigint zero {0};
   if (divisor == zero) throw ydc_exn ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   cout << "8quo:" << quotient << "  rem:" << remainder << " pow:" << power_of_2 << " div:" << divisor << endl;
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
      return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   if(ubig_value.size() < that.ubig_value.size())
      return false;
      
   if(ubig_value.size() > that.ubig_value.size())
      return false;
      
   for(int i=ubig_value.size()-1; i>=0; --i)
   {
      if(ubig_value[i] != that.ubig_value[i])
      {
         return false;
      }
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   if(ubig_value.size() < that.ubig_value.size())
      return true;
      
   if(ubig_value.size() > that.ubig_value.size())
      return false;
   
   for(int i=ubig_value.size()-1; i>=0; --i)
   {
      if(ubig_value[i] < that.ubig_value[i])
      {
         cout << "true\n";
         return true;
      }
   }     
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   string result = "";
   for(int i=that.ubig_value.size()-1; i>=0; --i)
   {
      result += that.ubig_value[i] + '0';
   }
   return out << result;
}






