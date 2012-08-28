#ifndef TOP_H
#define TOP_H

#include "lt_initiator.cpp"
#include "lt_target.cpp"

SC_MODULE( TOP ) {

	lt_initiator ult_initiator;
	lt_target    ult_target;

	SC_CTOR( TOP ) 
		: ult_initiator("ult_initiator"),
		  ult_target("ult_target")
	{
		// bind
		ult_initiator.initiator_socket.bind(ult_target.target_socket);
	}
};

#endif

