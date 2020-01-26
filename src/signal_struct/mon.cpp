#include "systemc.h"
#include "mon.h"

void mon::monitor() {
  cout << "A\t\t";
  cout << "B" << endl;

  while (true) {
    wait();                             // wait for 1 clock cycle
    cout << A.read() << "\t";
    cout << B.read() << endl;
  }
}
