#ifndef MYTYPE_H
#define MYTYPE_H
#include "systemc.h"
#include <string>
#include <iomanip>

class MyType {
  private:
    unsigned info;
    bool flag;
  public:

    // constructor
    MyType (unsigned _info = 0, bool _flag = false) {
      info = _info;
      flag = _flag;
    }

    inline bool operator == (const MyType & rhs) const {
      return (rhs.info == info && rhs.flag == flag );
    }

    inline MyType& operator = (const MyType& rhs) {
      info = rhs.info;
      flag = rhs.flag;
      return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf, const MyType & v,
    const std::string& NAME ) {
      sc_trace(tf,v.info, NAME + ".info");
      sc_trace(tf,v.flag, NAME + ".flag");
    }

    inline friend ostream& operator << ( ostream& os,  MyType const & v ) {
      os << "(" << v.info << "," << std::boolalpha << v.flag << ")";
      return os;
    }

};
#endif
