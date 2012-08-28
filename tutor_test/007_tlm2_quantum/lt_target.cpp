#ifndef LT_TARGET_H
#define LT_TARGET_H

#include <stdlib.h>
#include <vector>
#include <iostream>
using namespace std;

#include <systemc.h>
#include <tlm.h>
using namespace sc_core;

class lt_target
 : public sc_module,
   public tlm::tlm_fw_transport_if<>
{
private:
	vector<unsigned int> mem;
	unsigned int         mem_size;

public:
	lt_target( sc_module_name name);
	~lt_target();

	void b_transport(tlm::tlm_generic_payload&, sc_time&);
	tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_time&);
	unsigned int transport_dbg(tlm::tlm_generic_payload&); // 
	bool get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&); // 

	tlm::tlm_target_socket<32> target_socket;

	sc_time wr_latency;
	sc_time rd_latency;

};

SC_HAS_PROCESS(lt_target);
inline lt_target::lt_target(sc_module_name name)
 : sc_module(name),
	target_socket("target_socket")
{
	target_socket.bind( *this );
	// Memory Initialize
	mem_size = 0xf000;
	mem.resize(mem_size);
	for(unsigned int i=0; i<mem_size; i++) { mem[i] = 0; }
	wr_latency = sc_time(10,SC_NS);
	rd_latency = sc_time(10,SC_NS);
}

inline lt_target::~lt_target(){
	// none
}

inline void lt_target::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay){

	sc_dt::uint64  adr  = trans.get_address() / 4;
	unsigned char* ptr  = trans.get_data_ptr();
	unsigned int   len  = trans.get_data_length();
	unsigned char* byt  = trans.get_byte_enable_ptr();
	unsigned int   blen = trans.get_byte_enable_length();
	unsigned int   wid  = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(mem_size) ) {
		trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
		return;
	}
	if ( wid < len ) {
		trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
		return;
	}
	
	if (trans.is_read()) {
		delay += rd_latency;
		if ( byt != 0 ) {
			for ( unsigned int i = 0; i < len; i++ )
				if ( byt[i % blen] == TLM_BYTE_ENABLED ){
					ptr[i] = (mem[adr+i/4] >> ((i&3)*8)) & 0xFF;
				}
		} else {
			memcpy(ptr, &mem[adr], len);
		}
	} else if (trans.is_write()) {
		delay += wr_latency; 
		if ( byt != 0 ) {
			for ( unsigned int i = 0; i < len; i++ )
				if ( byt[i % blen] == TLM_BYTE_ENABLED ) {
					unsigned char temp[4];
					memcpy (temp, &mem[adr+i/4], 4);
					memcpy (&temp[i&3], &ptr[i], 1);
					memcpy (&mem[adr+i/4], temp, 4);
				}
		} else {
			memcpy( &mem[adr], ptr, len);
		}
	}
	
	trans.set_dmi_allowed(false);
	
	trans.set_response_status(tlm::TLM_OK_RESPONSE );

}

inline tlm::tlm_sync_enum lt_target::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay){
  return tlm::TLM_COMPLETED;
}

inline unsigned int lt_target::transport_dbg(tlm::tlm_generic_payload& trans){
  return 0;
}

inline bool lt_target::get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi){
  return false;
}

#endif
