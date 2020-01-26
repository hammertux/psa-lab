#ifndef MON_H
#define MON_H
#include "mytype.h"
#include "systemc.h"

SC_MODULE(mon) {

  sc_in<MyType> A, B;
  sc_in<bool> Clk;

  void monitor();

  SC_CTOR(mon) {
    SC_THREAD(monitor);
    sensitive << Clk.neg();
  }
};
#endif
