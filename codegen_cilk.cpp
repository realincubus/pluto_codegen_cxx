
#include "codegen_cilk.hpp"
#include <iostream>
#include "assert.h"

void CodeGenCilk::pprint_for_loop_name( ) {
  header_includes.insert("cilk/cilk.h");
  dst << "cilk_for ("; 
}

