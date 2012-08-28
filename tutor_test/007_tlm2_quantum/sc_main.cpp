#include <stdlib.h>
#include <iostream>

using namespace std;

#include <systemc.h>
#include "top.h"

int sc_main( int argc, char* argv[] ) {
	TOP utop("utop");
#if DEBUG
	cout << "--- Simulation Start ---" << endl;
#endif
	sc_start();
	return 0;
}
