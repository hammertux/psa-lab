#include "systemc.h"
#include "stim.h"

void stim::do_stim() {
  MyType val1(100,true);
  MyType val2(200,false);
  
  wait();
  A.write( val1 );
  B.write( val2 );
  wait();
  A.write( val2 );
  B.write( val1 );
  wait();

  sc_stop();
}
