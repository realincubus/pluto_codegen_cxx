
#include "codegen_cilk.hpp"
#include <iostream>
#include "assert.h"

void CodeGenCilk::pprint_for_loop_name( ) {
  dst << "cilk_for ("; 
}

