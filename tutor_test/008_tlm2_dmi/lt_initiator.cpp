#ifndef LT_INITIATOR_H
#define LT_INITIATOR_H

#include <stdlib.h>
#include <iostream>
using namespace std;

#include <systemc.h>
#include <tlm.h>
using namespace sc_core;

class lt_initiator
 : public sc_module,
   public tlm::tlm_bw_transport_if<>
{
private:
	void write(unsigned int, unsigned int&);
	void read(unsigned int, unsigned int*);
	void burst_write(unsigned int, unsigned int*, const unsigned int);
	void burst_read(unsigned int, unsigned int*, const unsigned int);
	
	void process();

	tlm::tlm_dmi m_dmi;
	bool         m_dmi_allowed;

public:
	// TLM Initiator Socket
	tlm::tlm_initiator_socket<32> initiator_socket;

	tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_time&);
	void invalidate_direct_mem_ptr(sc_dt::uint64 a, sc_dt::uint64 b);

	SC_CTOR( lt_initiator )
		: initiator_socket("initiator_socket")
	{
		SC_THREAD( process );
		initiator_socket.bind( *this );
		m_dmi_allowed = false;
	}
};

inline void lt_initiator::process() {
#if 0
	unsigned int data = 0x0000 ;

	for(unsigned int i=0; i<10; i++){
		data = (0xA53 << i);
		write((i*4), data);
		printf("--- Write, Address:0x%2x, Data:0x%8x --- \n", i*4, data);
	}
	for(unsigned int i=0; i<10; i++){
		read((i*4), &data);
		printf("--- Read , Address:0x%2x, Data:0x%8x --- \n", i*4, data);
	}
#endif
	const unsigned int loop = 0x1000;
	unsigned int data[loop];
	unsigned int rdata[loop];

	for(unsigned int i=0; i<loop; i++){
		data[i]  = (0xA53 << i);
		rdata[i] = 0;
		printf("--- Write, Address:0x%2x, Data:0x%8x --- \n", i*4, data[i]);
	}
	burst_write(0, data, loop);
	burst_read(0, rdata, loop);
	for(unsigned int i=0; i<loop; i++){
		printf("--- Read , Address:0x%2x, Data:0x%8x --- \n", i*4, rdata[i]);
	}
	printf("--- Simlulation end --- \n");
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

	initiator_socket->b_transport(*trans, delay);
	wait(delay);
	if(trans->is_response_error()) {
		SC_REPORT_ERROR("WRITE:", trans->get_response_string().c_str());
	}
	delete trans;

}

inline void lt_initiator::burst_write(unsigned int address, unsigned int* data, const unsigned int size) {
	
	tlm::tlm_generic_payload* trans;
	trans = new tlm::tlm_generic_payload;
	
	trans->set_command(tlm::TLM_WRITE_COMMAND);
	trans->set_address(static_cast<sc_dt::uint64>(address));
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	
	sc_time delay = SC_ZERO_TIME;

	if((m_dmi.is_write_allowed() || m_dmi.is_read_write_allowed()) &&
		((address >= m_dmi.get_start_address() &&
		 (address + size) <= m_dmi.get_end_address()))){
		m_dmi_allowed = true;
	} else {
		initiator_socket->b_transport(*trans, delay);
		wait(delay);
		if(trans->is_response_error()) {
			SC_REPORT_ERROR("WRITE:", trans->get_response_string().c_str());
		}
		// DMI Check
		if(trans->is_dmi_allowed()){
			m_dmi_allowed = initiator_socket->get_direct_mem_ptr(*trans, m_dmi);
		}
	}
	// DMI
	if(m_dmi_allowed) {
		printf("--- DMI --- \n");
		unsigned int* dmi_data_ptr = (unsigned int*) m_dmi.get_dmi_ptr();
		for(unsigned int i=0; i<size; i++){
			dmi_data_ptr[i] = data[i];
		}
		wait(m_dmi.get_write_latency() * size);
	} else {
		for(unsigned int i=0; i<size; i++){
			trans->set_address(static_cast<sc_dt::uint64>((address+i*4)));
			trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data[i]));
			delay = SC_ZERO_TIME;
			initiator_socket->b_transport(*trans, delay);
			wait(delay);
			if(trans->is_response_error()) {
				SC_REPORT_ERROR("WRITE:", trans->get_response_string().c_str());
			}
		}
	}
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
	initiator_socket->b_transport(*trans, delay);
	wait(delay);
	if(trans->is_response_error()) {
		SC_REPORT_ERROR("READ :", trans->get_response_string().c_str());
	}

	memcpy( data, &rdata, 4);

	delete trans;

}

inline void lt_initiator::burst_read(unsigned int address, unsigned int* data, const unsigned int size) {
	
	tlm::tlm_generic_payload* trans;
	trans = new tlm::tlm_generic_payload;
	
	trans->set_command(tlm::TLM_READ_COMMAND);
	trans->set_address(static_cast<sc_dt::uint64>(address));
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	
	sc_time delay = SC_ZERO_TIME;

	if((m_dmi.is_write_allowed() || m_dmi.is_read_write_allowed()) &&
		((address >= m_dmi.get_start_address() &&
		 (address + size) <= m_dmi.get_end_address()))){
		m_dmi_allowed = true;
	} else {
		initiator_socket->b_transport(*trans, delay);
		wait(delay);
		if(trans->is_response_error()) {
			SC_REPORT_ERROR("WRITE:", trans->get_response_string().c_str());
		}
		// DMI Check
		if(trans->is_dmi_allowed()){
			m_dmi_allowed = initiator_socket->get_direct_mem_ptr(*trans, m_dmi);
		}
	}
	// DMI
	if(m_dmi_allowed) {
		printf("--- DMI --- \n");
		unsigned int* dmi_data_ptr = (unsigned int*) m_dmi.get_dmi_ptr();
		for(unsigned int i=0; i<size; i++){
			data[i] = dmi_data_ptr[i];
		}
		wait(m_dmi.get_read_latency() * size);
	} else {
		for(unsigned int i=0; i<size; i++){
			trans->set_address(static_cast<sc_dt::uint64>((address+i*4)));
			trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data[i]));
			delay = SC_ZERO_TIME;
			initiator_socket->b_transport(*trans, delay);
			wait(delay);
			if(trans->is_response_error()) {
				SC_REPORT_ERROR("READ :", trans->get_response_string().c_str());
			}
		}
	}
	delete trans;

}
inline tlm::tlm_sync_enum lt_initiator::nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay){
  trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
  return tlm::TLM_COMPLETED;
}

inline void lt_initiator::invalidate_direct_mem_ptr(sc_dt::uint64 a, sc_dt::uint64 b){

}

#endif

