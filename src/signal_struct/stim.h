#include "mytype.h"
#include "systemc.h"

SC_MODULE(stim) {
  sc_out<MyType> A,B;
  sc_in<bool> Clk;

  void do_stim();

  SC_CTOR(stim) {
    SC_THREAD(do_stim);
    sensitive << Clk.pos();
  }

};
