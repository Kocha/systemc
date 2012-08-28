#ifndef LT_INITIATOR_H
#define LT_INITIATOR_H

#include <stdlib.h>
#include <iostream>
#include <iomanip>
using namespace std;

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/tlm_quantumkeeper.h>
using namespace sc_core;

SC_MODULE( lt_initiator ),tlm::tlm_bw_transport_if<>
{
	void write(unsigned int, unsigned int&);
	void read(unsigned int, unsigned int*);
	
	void process();

#ifdef QUANTUM
	tlm_utils::tlm_quantumkeeper m_quantumkeeper;
#endif

public:
	tlm::tlm_initiator_socket<32> initiator_socket;

	tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_time&);
	void invalidate_direct_mem_ptr(sc_dt::uint64 a, sc_dt::uint64 b);

	SC_CTOR( lt_initiator )
		: initiator_socket("initiator_socket")
	{
		SC_THREAD( process );
		initiator_socket.bind( *this );
#ifdef QUANTUM
		m_quantumkeeper.set_global_quantum(sc_time(100,SC_US));
		m_quantumkeeper.reset();
#endif
	}
};

inline void lt_initiator::process() {
	unsigned int data = 0x0000 ;

	for(unsigned int i=0; i<50000; i++){
		data = i*5;
		write((i*4), data);
#if DEBUG
#if (!SIMPLE_MSG)
		std::cout << "--- sc_time : " << sc_time_stamp() << ", "
			<< "current_time : " << m_quantumkeeper.get_current_time() << ", "
			<< "local_time : "   << m_quantumkeeper.get_local_time() << ", "
			<< "Write, Address:0x" << std::hex << setw(4) << i*4 << ", "
			<< "Data:0x"           << std::hex << setw(8) << data
			<< " ---" << std::endl;
#else
		std::cout << "--- sc_time : " << sc_time_stamp() << ", "
			<< "Write, Address:0x" << std::hex << setw(4) << i*4 << ", "
			<< "Data:0x"           << std::hex << setw(8) << data
			<< " ---" << std::endl;
#endif
#endif
	}
	for(unsigned int i=0; i<50000; i++){
		read((i*4), &data);
#if DEBUG
#if (!SIMPLE_MSG)
		std::cout << "--- sc_time : " << sc_time_stamp() << ", "
			<< "current_time : " << m_quantumkeeper.get_current_time() << ", "
			<< "local_time : "   << m_quantumkeeper.get_local_time() << ", "
			<< "Read , Address:0x" << std::hex << setw(4) << i*4 << ", "
			<< "Data:0x"           << std::hex << setw(8) << data
			<< " ---" << std::endl;
#else
		std::cout << "--- sc_time : " << sc_time_stamp() << ", "
			<< "Read , Address:0x" << std::hex << setw(4) << i*4 << ", "
			<< "Data:0x"           << std::hex << setw(8) << data
			<< " ---" << std::endl;
#endif
#endif
	}
#if DEBUG
	printf("--- Simlulation end --- \n");
#endif
	sc_stop(); // stop the simulation

}

inline void lt_initiator::write(unsigned int address, unsigned int& data) {
	
	tlm::tlm_generic_payload* trans;
	trans = new tlm::tlm_generic_payload;
	
	trans->set_command(tlm::TLM_WRITE_COMMAND);
	trans->set_address(static_cast<sc_dt::uint64>(address));
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	
	sc_time delay = SC_ZERO_TIME;
#ifdef QUANTUM
	delay = m_quantumkeeper.get_local_time();
#endif

	initiator_socket->b_transport(*trans, delay);
	if(trans->get_response_status() != tlm::TLM_OK_RESPONSE) {
		switch(trans->get_response_status()){
			case tlm::TLM_ADDRESS_ERROR_RESPONSE     : printf("--- Write Interface Error(Address) --- \n"    ); sc_stop(); break;
			case tlm::TLM_COMMAND_ERROR_RESPONSE     : printf("--- Write Interface Error(Command) --- \n"    ); sc_stop(); break;
			case tlm::TLM_BURST_ERROR_RESPONSE       : printf("--- Write Interface Error(Length) --- \n"     ); sc_stop(); break;
			case tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE : printf("--- Write Interface Error(Byte Enable) --- \n"); sc_stop(); break;
			case tlm::TLM_GENERIC_ERROR_RESPONSE     : printf("--- Write Interface Error(Else) --- \n"       ); sc_stop(); break;
			default: break;
		}
	}
	
#ifdef QUANTUM
	m_quantumkeeper.set(delay);
	if (m_quantumkeeper.need_sync()) {
#if DEBUG
#if (!SIMPLE_MSG)
		std::cout << "Syncing..." << std::endl;
#endif
#endif
		m_quantumkeeper.sync();
	}
#else
	wait(delay);
#endif
	delete trans;

}

inline void lt_initiator::read(unsigned int address, unsigned int* data) {
	
	tlm::tlm_generic_payload* trans;
	trans = new tlm::tlm_generic_payload;
	
	unsigned int rdata;
	
	trans->set_command(tlm::TLM_READ_COMMAND);
	trans->set_address(static_cast<sc_dt::uint64>(address));
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&rdata));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	
	sc_time delay = SC_ZERO_TIME;
#ifdef QUANTUM
	delay = m_quantumkeeper.get_local_time();
#endif

	initiator_socket->b_transport(*trans, delay);

	if(trans->get_response_status() != tlm::TLM_OK_RESPONSE) {
		switch(trans->get_response_status()){
			case tlm::TLM_ADDRESS_ERROR_RESPONSE     : printf("--- Read Interface Error(Address) --- \n"    ); sc_stop(); break;
			case tlm::TLM_COMMAND_ERROR_RESPONSE     : printf("--- Read Interface Error(Command) --- \n"    ); sc_stop(); break;
			case tlm::TLM_BURST_ERROR_RESPONSE       : printf("--- Read Interface Error(Length) --- \n"     ); sc_stop(); break;
			case tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE : printf("--- Read Interface Error(Byte Enable) --- \n"); sc_stop(); break;
			case tlm::TLM_GENERIC_ERROR_RESPONSE     : printf("--- Read Interface Error(Else) --- \n"       ); sc_stop(); break;
			default: break;
		}
	}

	memcpy( data, &rdata, 4);

#ifdef QUANTUM
	m_quantumkeeper.set(delay);
	if (m_quantumkeeper.need_sync()) {
#if DEBUG
#if (!SIMPLE_MSG)
		std::cout << "Syncing..." << std::endl;
#endif
#endif
		m_quantumkeeper.sync();
	}
#else
	wait(delay);
#endif

	delete trans;

}

inline tlm::tlm_sync_enum lt_initiator::nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay){
  trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
  return tlm::TLM_COMPLETED;
}

inline void lt_initiator::invalidate_direct_mem_ptr(sc_dt::uint64 a, sc_dt::uint64 b){
	// none
}

#endif
