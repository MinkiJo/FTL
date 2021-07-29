#include "../../include/container.h"
#include "../../interface/interface.h"


typedef struct align_gc_buffer{
	uint8_t idx;
	char value[PAGESIZE];
	KEYT key[L2PGAP];
}align_gc_buffer;


typedef struct gc_value{
	uint32_t ppa;
	value_set *value;
	bool isdone;
}gc_value;

typedef struct align_merge_buffer{
	uint8_t idx;
	char value[_PPS * PAGESIZE];
	KEYT key[_LPPS];
}merge_buffer;

void hybrid_merge(uint32_t lbnum);
void do_switch(uint32_t lbnum);
void do_merge(uint32_t lbnum);
gc_value* send_req(uint32_t ppa, uint8_t type, value_set *value);
void *hybrid_merge_end_req(algo_req *input); 
