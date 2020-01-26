#include "mytype.h"
#include "systemc.h"
#include "stim.h"
#include "mon.h"

int sc_main(int argc, char* argv[]) {
  // signals to wire up the device under test
  sc_signal<MyType> ASig, BSig;

  // declare a clk for the stimulus process
  sc_clock TestClk("TestClock", 10, SC_NS);

  // stimulus instance
  stim Stim1("Stimulus1");
  Stim1.A(ASig);
  Stim1.B(BSig);
  Stim1.Clk(TestClk);

  // monitor instance
  mon Mon1("Monitor1");
  Mon1.A(ASig);
  Mon1.B(BSig);
  Mon1.Clk(TestClk);

  sc_trace_file *wf = sc_create_vcd_trace_file("sigtrace");
  sc_trace(wf, TestClk, "clock");
  sc_trace(wf, ASig, "A");
  sc_trace(wf, BSig, "B");

  sc_start();

  // 64 C8
  sc_close_vcd_trace_file(wf);

  return (0);

}
