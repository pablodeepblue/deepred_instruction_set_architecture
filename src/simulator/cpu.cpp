#include "cpu.h"
#include <cstdio>
#include <memory.h>

// Data in each pipe stage
// executable execute_buffer = {
// 	.in2=0,
// 	.in1=0,
// 	.dest=0,
// 	.shamt=0,
// 	.op=NOP,
// };
//
// mupdate memory_buffer = {
// 	.dat=0,
//     .ops=MEMPAS,
// 	.reg=XZR,
// 	.mem=0,
// };
// rupdate writes_buffer = rupdate{
// 	.dat=0,
// 	.reg=XZR,
// };
//
// bool discard = false;

//each func in in charge of clearing thier buffer;
//this is to avoid multiwrites

inline void CPU::reset(){
	cycle_count = 0;
	hlt = 0;
	reg[XPC] = 0;
	reg[XSP] = MAXMEMORY;
	reg_buff = 0;
	flags = 0; //XXXXNZVC
	fetch_buffer=NOP_WORD;
	decode_buffer=NOP_WORD;
	execute_buffer.op=NOP;
	memory_buffer.ops = MEMPAS;
	memory_buffer.reg = XZR;
	writes_buffer.reg=XZR;
}


void CPU::reset_all(){
	cycle_count = 0;
	hlt = 0;
	reg_buff = 0;
	flags = 0; //XXXXNZVC
	discard = 0;
	fetch_buffer= NOP_WORD;
	decode_buffer= NOP_WORD;
	execute_buffer = executable();
	memory_buffer = mupdate();
	writes_buffer = rupdate();
	for (int i=0; i < 16; i++) {
		reg[i] = 0;
	}
	reg[XSP] = MAXMEMORY;
	pipemode = 1;
	ppu.delete_sdl();
	state();
}


inline void CPU::regF(int reg_){
	if (reg_ == XZR) return;
	reg_buff |= 1 << reg_;
}

inline void CPU::regU(int reg_){
	if (reg_ == XZR) return;
	reg_buff &= ~(1<<reg_);
}

inline int CPU::regC(int reg_){
    if(reg_ == XZR) return 0;
    return reg_buff & (1 << reg_);
}


inline int CPU::flagC(int fn){
	return ( flags & fn ? 1 : 0 );
}

void CPU::branch(mupdate& mup, executable& ex, int clear){
	//printf("BRANCH TO %d %d %d\n", ex.in1, ex.in2, ex.in1 + ex.in2);
	mup.dat = ex.in1 + ex.in2;
	mup.reg = XPC;
	ex.op = NOP;
	//printf("Clearing buff [BRANCH]\n");
	decode_buffer = NOP_WORD;
	fetch_buffer  = NOP_WORD;
	discard = true;
	if (clear) flags = 0;
}


inline uint8_t  CPU::NFLA(uint64_t result){
	return ( result & MSB ? NFL : 0 );
}


inline uint8_t CPU::VFLA(uint32_t a, uint32_t b, uint64_t result){
	return ( ( (a & MSB) == (b & MSB) ) && ( (a & MSB) != (result & MSB) ) ? VFL : 0);
}


inline uint8_t CPU::ZFLA(int64_t result){
	return ((result & MAX_UINT32) == 0 ? ZFL : 0);
}


inline uint8_t  CPU::CFLA(int64_t result){
	return (result & 0x100000000 ? CFL : 0);
}


inline word CPU::address_to_tag(const word address){
    return address >> 5;
}


inline word CPU::address_to_index(const word address){
    return (address>>2) & 0b111;
}


CPU::CPU(Memory *memory, PPU ppu_)
	: mem(memory), ppu(ppu_) {
	reg[XSP] = MAXMEMORY;

	pipemode = 1;
	reset();
}


Cache_lv1& CPU::getDataMemory(){
    return mem.getDataMemory();
}


Cache_lv1& CPU::getInstructMemory(){
    return mem.getInstructMemory();
}


void CPU::state(){
	printf("PC: %08x ", reg[XPC]);
	printf("ST: %08x ", reg[XSP]);
	printf("PIPE: %s ", (pipemode ? "True" : "False"));
	printf("HALT: %s ", (hlt ? "True" : "False"));
	printf("REGBUFF: %08X\n", reg_buff);
	printf("Negt FLAG: %X ", flagC(NFL));
	printf("oVer FLAG: %X ", flagC(VFL));
	printf("ZERO FLAG: %X ", flagC(ZFL));
	printf("Carr FLAG: %X\n", flagC(CFL));
	for (int i = 0; i < 16; i++)
		printf("REG[%i] ~ %08X ", i, reg[i]);
	printf("\n");
	printf("Fetch: %08X\n", fetch_buffer);
	printf("Decode: %08X\n", decode_buffer);
	printf("Execute: [%02X] [%01X] [%08X] [%08X] [%08X]\n",
		execute_buffer.op, execute_buffer.shamt,
		execute_buffer.dest, execute_buffer.in1, execute_buffer.in2
	);
	printf("Memory [%01X] [%02u] [%08X] [%08X]\n",
		memory_buffer.ops, memory_buffer.reg, memory_buffer.mem, memory_buffer.dat);
	printf("Write Back [%02u] [%08X]\n", writes_buffer.reg, writes_buffer.dat);
}

uint32_t CPU::F(int _blocked) {
	fetch_buffer = NOP_WORD;
	if(reg[XPC] >= MAXMEMORY){ printf("FORCED STOP (INSTRUCT OUTSIDE MEM)\n"); hlt = 1; return NOP_WORD; } //avoid segmentation error
	if(mem.memI_buffer_address(reg[XPC], FETCH) == cache_wait && mem.memI_buffer_status(FETCH)) {
		if (discard) {
			discard = false;
			return NOP_WORD;
		}
		fetch_buffer = mem.memI_load_buffer(FETCH).data[address_to_index(reg[XPC])];
		if (_blocked != BLOCKED) reg[XPC] += 4;
	}
	return fetch_buffer;
}

executable CPU::D(int _blocked) {
	int blocked = _blocked;
	executable e;
	uint8_t op = (decode_buffer >> 24);
    uint8_t reg2;
    uint8_t reg1;
	switch(op >> 4){
		case NOPFORMAT << 1:
			e.op = NOP;
			break;
		case HFORMAT << 1: //HLT
			e.op = HLT;
			break;
		case RFORMAT << 1: //REG
			// add line regs are up
            reg2 = (decode_buffer >> 16) & 0xF;
            reg1 = (decode_buffer >>  4) & 0xF;
			if (regC(reg1) || regC(reg2)) {
				blocked = BLOCKED;
				break;
			}
			e.in2 = reg[reg2];
			e.in1 = reg[reg1];
			e.dest= (decode_buffer >>  0) & 0xF;
			e.shamt = (uint8_t)((decode_buffer >> 8) & 0x1F);
			e.op =  op;
			if (blocked != BLOCKED) regF(e.dest);
			break;
		case IFORMAT << 1: //IMM / DATA
            reg1 = (decode_buffer >> 4) & 0xF;
			if (regC(reg1)) {
				blocked = BLOCKED;
				break;
			}
			e.in2 = (int32_t)(int16_t)((decode_buffer >>  8) & 0xFFFF);
			e.in1 = reg[reg1];
			e.dest = (decode_buffer >>  0) & 0xF;
			e.op = op;
			if (blocked != BLOCKED) regF(e.dest);
			break;
		case (IFORMAT << 1) + 1:
            reg2 = (decode_buffer >> 4) & 0xF;
			reg1 = (decode_buffer >>  0) & 0xF;
			if(regC(reg2) || regC(reg1))  {
				blocked = BLOCKED;
				break;
			}
			e.in2 = (int32_t)(int16_t)((decode_buffer >>  8) & 0xFFFF);
			e.in1 = reg[reg2];
            e.dest= reg[reg1];
			e.op = op;
			break;
		case BFORMAT << 1: //BRANCH
			if(regC(XPC)) {
				blocked = BLOCKED;
				break;
			}
			e.in2 = decode_buffer & 0x800000 ? ((int32_t)((~decode_buffer) & 0xFFFFFF) + 1) * -1 : decode_buffer & 0xFFFFFF;
			e.in1 = reg[XPC] - (pipemode ? 4 : 0);
			e.op = op;
			if (blocked != BLOCKED) regF(XPC);
			break;
		case IMFORMAT << 1: //MEM OPS
			reg1 = decode_buffer & 0xF;
			if (regC(reg1))  {
				blocked = BLOCKED;
				break;
			}
			e.in2 = (int32_t)(int16_t)((decode_buffer >>  8) & 0xFFFF);
			e.in1 = reg[reg1];
			e.dest = reg1;
			e.shamt =(uint8_t)((decode_buffer >>  4) & 0x11);
			e.op = op;
			if (blocked != BLOCKED) regF(e.dest);
			break;
		case MEFORMAT << 1:
			e.in1 = decode_buffer & 0xFFFFFF;
			e.op = op;
            if (blocked != BLOCKED && op == INP) regF(e.in1);
		default: break;
	}
	if (blocked == BLOCKED) F(blocked);
	else decode_buffer = F(blocked);
	return e;

}

mupdate CPU::E(int _blocked) {
	// execute
	int blocked = _blocked;
	uint64_t result;
	int32_t  cresult;
	mupdate m;
	m.ops = execute_buffer.op;
	//printf("execute [%02x]\n", execute_buffer.op);
	switch(execute_buffer.op){
		case HLT:
			hlt = 1;
			blocked = BLOCKED;
			break;
		case NOP:
			break;
		case SUB: case SUBI:
            //printf("SUB");
            result = (uint64_t)(uint32_t)(execute_buffer.in1) - (uint64_t)(uint32_t)(execute_buffer.in2);
			cresult = (int32_t)result;
			//printf(" (%09lX) (%ld)", result, result);
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | VFLA(execute_buffer.in1, -execute_buffer.in2, result) | ZFLA(result)
				| CFLA(result);
			break;
		case ADD: case ADDI: // add
            //printf("ADD");
			result = (uint64_t)(uint32_t)execute_buffer.in1 + (uint64_t)(uint32_t)execute_buffer.in2;
		    m.dat = (int32_t)result;
			m.reg = execute_buffer.dest;
			//printf("(%09lX) (%ld)", result, result);
			flags = NFLA(result) | VFLA(execute_buffer.in1, execute_buffer.in2, result) | ZFLA(result)
				| CFLA(result);
			break;
		case DIV:
            //printf("DIV");
			result = execute_buffer.in1 / execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case MOD:
            //printf("MOD");
			result = execute_buffer.in1 % execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case MUL:
            //printf("MUL");
			result = execute_buffer.in1 * execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case AND: case ANDI:
            //printf("AND");
			result = execute_buffer.in1 & execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case ORR: case ORRI:
            //printf("ORR");
			result = execute_buffer.in1 | execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
            break;
		case EOR: case EORI:
            //printf("EOR");
			result = execute_buffer.in1 ^ execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
            break;
		case LSL:
            //printf("LSL");
			result = execute_buffer.in1 << execute_buffer.shamt;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result)
				| CFLA(result);
            break;
		case LSR:
            //printf("LSR");
			result = ((uint32_t)execute_buffer.in1) >> execute_buffer.shamt;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result)
				| (execute_buffer.shamt != 0 && 1 << (execute_buffer.shamt-1) & execute_buffer.in1 ? CFL : 0);
            break;
        case BR:
            //printf("BR");
            m.dat = execute_buffer.in2;
            m.reg = XPC;
			decode_buffer = NOP_WORD;
			fetch_buffer  = NOP_WORD;
            break;
        case LDUR:
            //printf("LDUR");
            m.mem = execute_buffer.in1 + (execute_buffer.in2 & 0xFFFF);
            m.reg = execute_buffer.dest;
            break;
        case LDURB:
            //printf("LDURB");
            m.mem = execute_buffer.in1 + execute_buffer.in2;
            m.reg = execute_buffer.dest;
            break;
        case STUR:
            //printf("STUR");
			m.dat = execute_buffer.dest;
			//printf("STUR %08X %08X\n", execute_buffer.in1, execute_buffer.in2);
            m.mem = execute_buffer.in1 + execute_buffer.in2;
            break;
        case STURB:
            //printf("STURB");
			//M[in1 + in2] = dest
			m.dat = execute_buffer.dest;
            m.mem = execute_buffer.in1 + execute_buffer.in2;
            break;
        case B_:
			//printf("B");
            branch(m, execute_buffer, 0);
			break;
        case BEQ:
			//printf("BEQ");
            if(flagC(ZFL))
				branch(m, execute_buffer, 1);
			else {
				regU(XPC);
			}
			break;
        case BNE:
			//printf("BNE");
            if(!flagC(ZFL))
				branch(m, execute_buffer, 1);
			else regU(XPC);
			break;
        case BLT:
			//printf("BLT");
            if( flagC(NFL) != flagC(VFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
			break;
        case BLE:
			//printf("BLE");
            if( flagC(ZFL) || (flagC(NFL) != flagC(VFL)) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
			break;
		case BGT:
			//printf("BGT");
            if( !flagC(ZFL) && (flagC(NFL) == flagC(VFL)) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BGE:
			//printf("BGE");
            if ( flagC(NFL) == flagC(VFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BLO:
			//printf("BLO");
            if ( flagC(CFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BLS:
			//printf("BLS");
            if ( flagC(CFL) || flagC(ZFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BHI:
			//printf("BHI");
            if ( !flagC(CFL) && !flagC(ZFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BHS:
			//printf("BHS");
            if ( !flagC(CFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
		case MOVZ:
            //printf("MOVZ");
            m.dat = (execute_buffer.in2 & 0xFFFF) << (execute_buffer.shamt*16);
			m.reg = execute_buffer.dest;
			break;
        case MOVK:{
            //printf("MOVK");
            uint32_t mask = 0xFFFF << ((execute_buffer.shamt&0b1)*16);
            m.dat = (execute_buffer.in1 & ~mask) | ((execute_buffer.in2 & 0xFFFF) << (execute_buffer.shamt*16));
			m.reg = execute_buffer.dest;
            break;
        }
		case MOVB:{
			uint32_t mask = 0xFF << (execute_buffer.shamt*8);
			m.dat = (execute_buffer.in1 & ~mask) | ((execute_buffer.in2 & 0xFF) << (execute_buffer.shamt*8));
			m.reg = execute_buffer.dest;
            break;
		}
		case PPU_:
			if(_blocked != BLOCKED) ppu.output();
			break;
		case WAI:
			if(!ppu.wait(execute_buffer.in1)) {blocked = BLOCKED; break;};
			break;
        case INP:
            m.dat = (int32_t)input;
            input = -1;
            m.reg = (execute_buffer.in1 & 0xF);
		default: break;
    }
	if (blocked == BLOCKED) D(blocked);
	else execute_buffer = D(blocked);
	return m;
}

rupdate CPU::M() {
	int blocked = IDLE;
	rupdate w;
	w.reg = XZR;
	switch(memory_buffer.ops){
		//If load, call Cache / Memmory
		case MEMLDW:
			if(mem.memD_buffer_address(memory_buffer.mem, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY)){
				w.dat = mem.memD_load_buffer(MEMORY).data[(memory_buffer.mem>>2) & 0b111];
				w.reg = memory_buffer.reg;
				break;
			}
			else blocked = BLOCKED;
			break;
		case MEMLDB:
			if(mem.memD_buffer_address(memory_buffer.mem, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY)){
				int32_t res = (
								mem.memD_load_buffer(MEMORY).data[(memory_buffer.mem>>2) & 0b111]
								& (0xFF << ((memory_buffer.mem & 0b11)*8))
							) >> ((memory_buffer.mem & 0b11)*8);
				w.dat = res;
				w.reg = memory_buffer.reg;
				break;
			}
			else blocked = BLOCKED;
			break;
		// If store, send data, address to cache
		case MEMSTW: //sta
			if(mem.memD_store_word(memory_buffer.mem, memory_buffer.dat, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY)) {
				//printf("SERVING[MAIN]STORED %08X (CLEARING)\n", memory_buffer.mem);
				mem.memD_load_buffer(MEMORY);
				break;
			}
			else blocked = BLOCKED;
			break;
		case MEMSTB: //sta
			if(mem.memD_store_byte_(memory_buffer.mem, memory_buffer.dat & 0xFF, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY)) {
				//printf("[MAIN]STORED %08X\n", memory_buffer.mem);
				mem.memD_load_buffer(MEMORY);
				break;
			}
			else blocked = BLOCKED;
			break;
		case FLU:
			if (mem.memD_flush(MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY)) {;
				mem.memD_load_buffer(MEMORY);
				break;
			}
			else blocked = BLOCKED;
			break;
		default: {
			w.dat = memory_buffer.dat;
			w.reg = memory_buffer.reg;
			break;
		}
	}

	if (blocked == BLOCKED) {
		//printf("MEM BLOCKED");
		E(blocked);
	}
	else memory_buffer = E(blocked);
	return w;
}

void CPU::W() {
	//  If W instr has result
	if (writes_buffer.reg != XZR) {
		// Write to Regs
		reg[writes_buffer.reg] = writes_buffer.dat;
		// Update Pend Regs == Update PC
		regU(writes_buffer.reg);
	}
	if (writes_buffer.reg == XPC) {
		decode_buffer = NOP_WORD;
		fetch_buffer = NOP_WORD;
	}
	writes_buffer = M();
}

void CPU::clock_pipeline() {
	W();
	uint32_t x = ppu.pollevent();
	if (x != (uint32_t)-1)
		input = x;
	cycle_count++;
}

void CPU::clock_nonpipeline() {
	fetch_buffer = NOP_WORD;
	if(reg[XPC] >= MAXMEMORY){ printf("FORCED STOP (INSTRUCT OUTSIDE MEM)\n"); hlt = 1;}
	while(!(mem.memI_buffer_address(reg[XPC], FETCH) == cache_wait && mem.memI_buffer_status(FETCH))) {cycle_count++;}
	fetch_buffer = mem.memI_load_buffer(FETCH).data[address_to_index(reg[XPC])];
	reg[XPC] += 4;
	decode_buffer = fetch_buffer;
	//decode
	cycle_count ++;
	executable e;
	uint8_t op = (decode_buffer >> 24);
    uint8_t reg2;
    uint8_t reg1;
	switch(op >> 4){
		case NOPFORMAT << 1:
			e.op = NOP;
			break;
		case HFORMAT << 1: //HLT
			e.op = HLT;
			break;
		case RFORMAT << 1: //REG
			// add line regs are up
            reg2 = (decode_buffer >> 16) & 0xF;
            reg1 = (decode_buffer >>  4) & 0xF;
			e.in2 = reg[reg2];
			e.in1 = reg[reg1];
			e.dest= (decode_buffer >>  0) & 0xF;
			e.shamt = (uint8_t)((decode_buffer >> 8) & 0x1F);
			e.op =  op;
			break;
		case IFORMAT << 1: //IMM / DATA
            reg1 = (decode_buffer >> 4) & 0xF;
			e.in2 = (int32_t)(int16_t)((decode_buffer >>  8) & 0xFFFF);
			e.in1 = reg[reg1];
			e.dest = (decode_buffer >>  0) & 0xF;
			e.op = op;
			break;
		case (IFORMAT << 1) + 1:
            reg2 = (decode_buffer >> 4) & 0xF;
			reg1 = (decode_buffer >>  0) & 0xF;
			e.in2 = (int32_t)(int16_t)((decode_buffer >>  8) & 0xFFFF);
			e.in1 = reg[reg2];
            e.dest= reg[reg1];
			e.op = op;
			break;
		case BFORMAT << 1: //BRANCH
			e.in2 = decode_buffer & 0x800000 ? ((int32_t)((~decode_buffer) & 0xFFFFFF) + 1) * -1 : decode_buffer & 0xFFFFFF;
			e.in1 = reg[XPC] - 4;
			e.op = op;
			break;
		case IMFORMAT << 1: //MEM OPS
			reg1 = decode_buffer & 0xF;
			e.in2 = (int32_t)(int16_t)((decode_buffer >>  8) & 0xFFFF);
			e.in1 = reg[reg1];
			e.dest = reg1;
			e.shamt =(uint8_t)((decode_buffer >>  4) & 0x11);
			e.op = op;
			break;
		case MEFORMAT << 1:
			e.in1 = decode_buffer & 0xFFFFFF;
			e.op = op;
            if (op == INP) regF(e.in1);
		default: break;
	}
	execute_buffer = e;
	// execute
	cycle_count ++;
	uint64_t result;
	int32_t  cresult;
	mupdate m;
	m.ops = execute_buffer.op;
	switch(execute_buffer.op){
		case HLT:
			hlt = 1;
			break;
		case NOP:
			break;
		case SUB: case SUBI:
            //printf("SUB");
            result = (uint64_t)(uint32_t)(execute_buffer.in1) - (uint64_t)(uint32_t)(execute_buffer.in2);
			cresult = (int32_t)result;
			//printf(" (%09lX) (%ld)", result, result);
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | VFLA(execute_buffer.in1, -execute_buffer.in2, result) | ZFLA(result)
				| CFLA(result);
			break;
		case ADD: case ADDI: // add
            //printf("ADD");
			result = (uint64_t)(uint32_t)execute_buffer.in1 + (uint64_t)(uint32_t)execute_buffer.in2;
		    m.dat = (int32_t)result;
			m.reg = execute_buffer.dest;
			//printf("(%09lX) (%ld)", result, result);
			flags = NFLA(result) | VFLA(execute_buffer.in1, execute_buffer.in2, result) | ZFLA(result)
				| CFLA(result);
			break;
		case DIV:
            //printf("DIV");
			result = execute_buffer.in1 / execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case MOD:
            //printf("MOD");
			result = execute_buffer.in1 % execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case MUL:
            //printf("MUL");
			result = execute_buffer.in1 * execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case AND: case ANDI:
            //printf("AND");
			result = execute_buffer.in1 & execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
			break;
		case ORR: case ORRI:
            //printf("ORR");
			result = execute_buffer.in1 | execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
            break;
		case EOR: case EORI:
            //printf("EOR");
			result = execute_buffer.in1 ^ execute_buffer.in2;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result);
            break;
		case LSL:
            //printf("LSL");
			result = execute_buffer.in1 << execute_buffer.shamt;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result)
				| CFLA(result);
            break;
		case LSR:
            //printf("LSR");
			result = ((uint32_t)execute_buffer.in1) >> execute_buffer.shamt;
			cresult = (int32_t)result;
		    m.dat = cresult;
			m.reg = execute_buffer.dest;
			flags = NFLA(result) | ZFLA(result)
				| (execute_buffer.shamt != 0 && 1 << (execute_buffer.shamt-1) & execute_buffer.in1 ? CFL : 0);
            break;
        case BR:
            //printf("BR");
            m.dat = execute_buffer.in2;
            m.reg = XPC;
			decode_buffer = NOP_WORD;
			fetch_buffer  = NOP_WORD;
            break;
        case LDUR:
            //printf("LDUR");
            m.mem = execute_buffer.in1 + (execute_buffer.in2 & 0xFFFF);
            m.reg = execute_buffer.dest;
            break;
        case LDURB:
            //printf("LDURB");
            m.mem = execute_buffer.in1 + (execute_buffer.in2 & 0xFFFF);
            m.reg = execute_buffer.dest;
            break;
        case STUR:
            //printf("STUR");
			m.dat = execute_buffer.dest;
			//printf("STUR %08X %08X\n", execute_buffer.in1, execute_buffer.in2);
            m.mem = execute_buffer.in1 + (execute_buffer.in2 & 0xFFFF);
            break;
        case STURB:
            //printf("STURB");
			//M[in1 + in2] = dest
			m.dat = execute_buffer.dest;
            m.mem = execute_buffer.in1 + (execute_buffer.in2 & 0xFFFF);
            break;
        case B_:
			//printf("B");
            branch(m, execute_buffer, 0);
			break;
        case BEQ:
			//printf("BEQ");
            if(flagC(ZFL))
				branch(m, execute_buffer, 1);
			else regU(XPC);
			break;
        case BNE:
			//printf("BNE");
            if(!flagC(ZFL))
				branch(m, execute_buffer, 1);
			else regU(XPC);
			break;
        case BLT:
			//printf("BLT");
            if( flagC(NFL) != flagC(VFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
			break;
        case BLE:
			//printf("BLE");
            if( flagC(ZFL) || (flagC(NFL) != flagC(VFL)) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
			break;
		case BGT:
			//printf("BGT");
            if( !flagC(ZFL) && (flagC(NFL) == flagC(VFL)) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BGE:
			//printf("BGE");
            if ( flagC(NFL) == flagC(VFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BLO:
			//printf("BLO");
            if ( flagC(CFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BLS:
			//printf("BLS");
            if ( flagC(CFL) || flagC(ZFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BHI:
			//printf("BHI");
            if ( !flagC(CFL) && !flagC(ZFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
        case BHS:
			//printf("BHS");
            if ( !flagC(CFL) )
				branch(m, execute_buffer, 1);
			else regU(XPC);
            break;
		case MOVZ:
            //printf("MOVZ");
            m.dat = (execute_buffer.in2 & 0xFFFF) << (execute_buffer.shamt*16);
			m.reg = execute_buffer.dest;
			break;
        case MOVK:{
            //printf("MOVK");
            uint32_t mask = 0xFFFF << ((execute_buffer.shamt&0b1)*16);
            m.dat = (execute_buffer.in1 & ~mask) | ((execute_buffer.in2 & 0xFFFF) << (execute_buffer.shamt*16));
			m.reg = execute_buffer.dest;
            break;
        }
		case MOVB:{
			uint32_t mask = 0xFF << (execute_buffer.shamt*8);
			m.dat = (execute_buffer.in1 & ~mask) | ((execute_buffer.in2 & 0xFF) << (execute_buffer.shamt*8));
			m.reg = execute_buffer.dest;
            break;
		}
		case PPU_:
			ppu.output();
			break;
		case WAI:
			while(!ppu.wait(execute_buffer.in1)){};
			break;
        case INP:
            m.dat = (int32_t)input;
            input = -1;
            m.reg = (execute_buffer.in1 & 0xF);
		default: break;
    }
	memory_buffer = m;
	//memory
	cycle_count++;
	rupdate w;
	w.reg = XZR;
	int32_t res;
	switch(memory_buffer.ops){
		//If load, call Cache / Memmory
		case MEMLDW:
			while(!(mem.memD_buffer_address(memory_buffer.mem, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY))){ cycle_count ++; };
			w.dat = mem.memD_load_buffer(MEMORY).data[(memory_buffer.mem>>2) & 0b111];
			w.reg = memory_buffer.reg;
			break;
		case MEMLDB:
			while(!(mem.memD_buffer_address(memory_buffer.mem, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY))){ cycle_count ++; };
			res = (
							mem.memD_load_buffer(MEMORY).data[(memory_buffer.mem>>2) & 0b111]
							& (0xFF << ((memory_buffer.mem & 0b11)*8))
						) >> ((memory_buffer.mem & 0b11)*8);
			w.dat = res;
			w.reg = memory_buffer.reg;
		    break;
		// If store, send data, address to cache
		case MEMSTW: //sta
			while(!(mem.memD_store_word(memory_buffer.mem, memory_buffer.dat, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY))){ cycle_count ++; };
			mem.memD_load_buffer(MEMORY);
			break;
		case MEMSTB: //sta
			while(!(mem.memD_store_byte_(memory_buffer.mem, memory_buffer.dat & 0xFF, MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY))){ cycle_count ++; };
			mem.memD_load_buffer(MEMORY);
			break;
		case FLU:
			while (!(mem.memD_flush(MEMORY) == cache_wait && mem.memD_buffer_status(MEMORY))){ cycle_count ++; }
			mem.memD_load_buffer(MEMORY);
			break;
		default: {
			w.dat = memory_buffer.dat;
			w.reg = memory_buffer.reg;
			break;
		}
	}
	//write
	cycle_count ++;
	writes_buffer = w;
	if (writes_buffer.reg != XZR) {
		// Write to Regs
		reg[writes_buffer.reg] = writes_buffer.dat;
		// Update Pend Regs == Update PC
		regU(writes_buffer.reg);
	}
	uint32_t x = ppu.pollevent();
	if (x != (uint32_t)-1)
		input = x;
	cycle_count++;
}

int CPU::run_nonpipline(){
	while (!hlt){
		clock_nonpipeline();
		//state();
	}
	return cycle_count;
}

int CPU::single_step_nonpipline(){
	clock_nonpipeline();
	//state();
	return cycle_count;
}

int CPU::run_pipline(){
	while (!hlt || writes_buffer.reg != 15 || memory_buffer.ops != HLT){
		clock_pipeline();
	}
	return cycle_count;
}


int CPU::single_step_pipline(){
	clock_pipeline();
	//state();
	return cycle_count;
}


void CPU::open_ui(QApplication &app) {

	qDebug() << "opened";

    QLabel *cacheTitle = new QLabel("Cache Instruction Table");  // Create title label
    cacheTitle->setAlignment(Qt::AlignCenter);  // Center-align the title

	QLabel *cacheDTitle = new QLabel("Cache Data Table");  // Create title label
	cacheDTitle->setAlignment(Qt::AlignCenter);  // Center-align the title

    QLabel *memTitle = new QLabel("Memory Table");  // Create title label
    memTitle->setAlignment(Qt::AlignCenter);  // Center-align the title

	QLabel *pipeTitle = new QLabel("Pipe Stage");  // Create title label
	memTitle->setAlignment(Qt::AlignCenter);  // Center-align the title

	QLabel *regTitle = new QLabel("Register Content");  // Create title label
	memTitle->setAlignment(Qt::AlignCenter);  // Center-align the title

    QWidget window;
    window.setMinimumSize(1400, 800);  // Ensure enough space for side-by-side layout

	// Display Tables
 //    QTableWidget cacheTable(&window);
 //    QTableWidget memTable(&window);
	// QTableWidget pipeTable(&window);

	cacheTable = new QTableWidget(&window);
	memTable = new QTableWidget(&window);
	pipeTable = new QTableWidget(&window);
	regTable = new QTableWidget(&window);
	cacheDTable = new QTableWidget(&window);
	halt_display = new QLabel(&window);

	// Table Pixel Size
    cacheTable->setMinimumSize(250,250);
	cacheDTable->setMinimumSize(250,250);
    memTable->setMinimumSize(200,250);
	// memTable->setMaximumSize(2000, 250);  // Set maximum size
	pipeTable->setMinimumSize(500,50);
	// pipeTable->setMinimumSize(500,250);
	regTable->setMinimumSize(25,150);

	// Table Row/Column size
    cacheTable->setRowCount(INDEX_BITS);
    cacheTable->setColumnCount(LINE_SIZE + 2);

	cacheDTable->setRowCount(INDEX_BITS);
	cacheDTable->setColumnCount(LINE_SIZE + 2);

    memTable->setRowCount(MEMORY_LINE_SIZE);
    memTable->setColumnCount(LINE_SIZE);

	pipeTable->setRowCount(9);
	pipeTable->setColumnCount(5);

	regTable->setRowCount(17);
	regTable->setColumnCount(1);

    // column/Row labels
    cacheTable->setHorizontalHeaderLabels({"V", "D", "0", "4", "8", "C", "10", "14", "18", "1C"});
	cacheDTable->setHorizontalHeaderLabels({"V", "D", "0", "4", "8", "C", "10", "14", "18", "1C"});
    memTable->setHorizontalHeaderLabels({"0", "4", "8", "C", "10", "14", "18", "1C"});
	pipeTable->setHorizontalHeaderLabels({"Fetch", "Decode", "Execute", "Memory", "WriteBack"});
	regTable->setHorizontalHeaderLabels({"Register Content"});
	QStringList pipeLabels = {"Type", "Op", "Dest", "in1", "in2", "Shamt", "Mem", "Data", "Reg"};

    // row labels
    QStringList cacheRowLabels;
    for (int i = 0; i < INDEX_BITS; i++) {
        cacheRowLabels.append(QString("0x") + QString("%1").arg(i * 32, 3, 16, QChar('0')).toUpper());
    }

    cacheTable->setVerticalHeaderLabels(cacheRowLabels);
	cacheDTable->setVerticalHeaderLabels(cacheRowLabels);
	pipeTable->setVerticalHeaderLabels(pipeLabels);

    QStringList memRowLabels;
    for (word i = 0; i < MEMORY_LINE_SIZE; i++) {
        memRowLabels.append(QString("0x") + QString("%1").arg(i * 32, 4, 16, QChar('0')).toUpper());
    }
    memTable->setVerticalHeaderLabels(memRowLabels);

	QStringList regRowLabels;
	for (int i = 0; i < 16; i++) {
		switch(i){
			case XPC:
				regRowLabels.append(QString("XPC (X%1)").arg(i));
				break;
			case XSP:
				regRowLabels.append(QString("XSP (X%1)").arg(i));
				break;
			case XZR:
				regRowLabels.append(QString("XZR (X%1)").arg(i));
				break;
			default:
				regRowLabels.append(QString("X%1").arg(i));
		}
	}
	regRowLabels.append(QString("Reg Buffer"));
	regTable->setVerticalHeaderLabels(regRowLabels);

    // column width
    for (int i = 0; i < LINE_SIZE+2; i++) {
        cacheTable->setColumnWidth(i, 50);
    	cacheDTable->setColumnWidth(i, 50);
    	if (i < LINE_SIZE)
			memTable->setColumnWidth(i, 50);
    }

	cacheTable->setColumnWidth(0, 10);
	cacheDTable->setColumnWidth(0, 10);
	cacheTable->setColumnWidth(1, 10);
	cacheDTable->setColumnWidth(1, 10);

	// Cycle count
	cycle_display = new QLabel(&window);

	// PC value
	pc = new QLabel(&window);

    // Set Data
	update_ui();
	cycle_display->setText("Cycle: 0");
	halt_display->setText("Halted: 0");
	pc->setText("PC: X");



	// Push button
	QPushButton *singleStep = new QPushButton("Single-Step");
	QPushButton *multiStep = new QPushButton("Multi-Step");
	QPushButton *fullRun = new QPushButton("Run The Pipe");
	QCheckBox *noPipe = new QCheckBox("Disable Pipe");
	QCheckBox *noCache= new QCheckBox("Disable Cache");



	// Load file
	memLoad_file = new QLineEdit(&window);
	multiStep_num = new QLineEdit(&window);

	QPushButton *readButton = new QPushButton("Load Mem");
	QPushButton *resetSim = new QPushButton("Reset Sim");


	QVBoxLayout *loadMem = new QVBoxLayout();
	loadMem->setSpacing(0);
	loadMem->addWidget(memLoad_file, 0);
	loadMem->addWidget(readButton, 0, Qt::AlignTop);
	loadMem->addWidget(resetSim, 0, Qt::AlignTop);
	loadMem->addWidget(noPipe);
	loadMem->addWidget(noCache);


	// loadMem->setAlignment(memLoad_file, Qt::AlignTop);  // Align QLineEdit to top
	// loadMem->setAlignment(readButton, Qt::AlignTop);


	QVBoxLayout *cycle = new QVBoxLayout();
	QVBoxLayout *cycle_nested = new QVBoxLayout();

	cycle_nested->setSpacing(0);
	cycle_nested->addWidget(multiStep_num);
	cycle_nested->addWidget(multiStep);

	cycle->addLayout(cycle_nested);
	cycle->addWidget(singleStep);
	cycle->addWidget(fullRun);


	cycle->setAlignment(cycle_nested, Qt::AlignTop);  // Align QLineEdit to top
	cycle->setAlignment(singleStep, Qt::AlignTop);  // Align QLineEdit to top
	cycle->setAlignment(fullRun, Qt::AlignTop);


	int memWidth = 4;
	int regWidth = 2;

	// table titles
	QHBoxLayout *tableTitles = new QHBoxLayout();
	tableTitles->addWidget(cacheTitle, memWidth);
	tableTitles->addWidget(cacheDTitle, memWidth);
	tableTitles->addWidget(regTitle, regWidth);


    // mem/cache display tables
    QHBoxLayout *tableLayout = new QHBoxLayout();
    tableLayout->addWidget(cacheTable, memWidth);
	tableLayout->addWidget(cacheDTable, memWidth);
    // tableLayout->addWidget(memTable, memWidth);
	tableLayout->addWidget(regTable, regWidth);


	QHBoxLayout *pipeButtons = new QHBoxLayout();
	pipeButtons->addLayout(loadMem);
	pipeButtons->addLayout(cycle);


	// Run time Data
	QVBoxLayout *runData = new QVBoxLayout();
	runData->setSpacing(0);
	runData->addWidget(cycle_display, 0,  Qt::AlignCenter);
	runData->addWidget(halt_display, 0,  Qt::AlignCenter);
	runData->addWidget(pc, 0,  Qt::AlignCenter);



	QWidget* whiteBlock = new QWidget();
	// whiteBlock->setStyleSheet("background-color: white;");

	// mem/cache display tables
	QHBoxLayout *buttons = new QHBoxLayout();
	buttons->addWidget(whiteBlock, 3);
	buttons->addLayout(runData, 2);
	buttons->addLayout(pipeButtons, 5);




	// middle row

	// middle row
	QHBoxLayout *memPipeTitle = new QHBoxLayout();
	memPipeTitle->addWidget(memTitle, Qt::AlignCenter);  // Add title above the table
	memPipeTitle->addWidget(pipeTitle, Qt::AlignCenter);

	QHBoxLayout *memPipe = new QHBoxLayout();
	memPipe->addWidget(memTable, Qt::AlignCenter);  // Add title above the table
	memPipe->addWidget(pipeTable, Qt::AlignCenter);  // Add horizontal layout below the title

    // stack layouts
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(tableTitles);  // Add title above the table
    vLayout->addLayout(tableLayout);  // Add horizontal layout below the title
	vLayout->addLayout(memPipeTitle);  // Add title above the table
	vLayout->addLayout(memPipe);  // Add horizontal layout below the title
	vLayout->addLayout(buttons);  //

	connect(worker, &Worker_pipe::update, this, &CPU::update_ui);  // Connecting worker's signal to CPU's slot
	connect(singleStep, &QPushButton::clicked, worker, &Worker_pipe::single_step);
	connect(multiStep, &QPushButton::clicked, worker, &Worker_pipe::multi_step);
	connect(fullRun, &QPushButton::clicked, worker, &Worker_pipe::run_pipe);
	connect(readButton, &QPushButton::clicked, worker, &Worker_pipe::load_mem);
	connect(resetSim, &QPushButton::clicked, worker, &Worker_pipe::reset_sim);
	// connect(fullRun, &QPushButton::clicked, worker, &Worker_pipe::run_pipe);
	connect(noPipe, &QCheckBox::toggled, worker, &Worker_pipe::setNoPipe);
	connect(noCache, &QCheckBox::toggled, worker, &Worker_pipe::setNoCache);



    window.setLayout(vLayout);
    window.show();

	app.exec();
}


char* CPU::getTextFromInput() {
	// QString qText = memLoad_file->text();
	// std::string stdText = qText.toStdString();
	//
	// char* filepath = new char[stdText.length() + 1];
	// std::strcpy(filepath, stdText.c_str());
	//
	// return filepath;

	QString qText = memLoad_file->text();
	std::string stdText = qText.toStdString();

	std::string finalPath = stdText;

	char* filepath = new char[finalPath.length() + 1];
	std::strcpy(filepath, finalPath.c_str());

	return filepath;
}

int CPU::getStep() {
	QString qText = multiStep_num->text();
    int number = qText.toInt();
	return number;
}


void CPU::setWorker(Worker_pipe *workerObj) {
	worker = workerObj;  // Pass the worker object to CPU
}

void CPU::update_ui(){

	QString labelText = QString("Cycle: %1").arg(cycle_count);  // Create the formatted string
	cycle_display->setText(labelText);  // Update the label text

	// QString pcText = QString("PC: %1").arg(reg[13]);  // Create the formatted string

	QString pcText = QString("PC: 0x") + QString("%1").arg(reg[XPC], 4, 16, QChar('0')).toUpper();
	pc->setText(pcText);  // Update the label text

	QString haltlabelText = QString("Halted: %1").arg(hlt);
	halt_display->setText(haltlabelText);

	qDebug() << "2: UI Updated";
	//get storage data
	cache_line* cacheI;
	cache_line* cacheD;
	cacheI = mem.memI_return_storage();
	cacheD = mem.memD_return_storage();
	for (int i = 0; i < INDEX_BITS; i++) {
		cacheTable->setItem(i, 0,
				new QTableWidgetItem(QString::number(cacheI[i].valid, 16).toUpper()));
		cacheTable->setItem(i, 1,
				new QTableWidgetItem(QString::number(cacheI[i].update, 16).toUpper()));
		cacheDTable->setItem(i, 0,
				new QTableWidgetItem(QString::number(cacheD[i].valid, 16).toUpper()));
		cacheDTable->setItem(i, 1,
				new QTableWidgetItem(QString::number(cacheD[i].update, 16).toUpper()));
		for (int j = 2; j < LINE_SIZE+2; j++) {
			cacheTable->setItem(i, j,
					new QTableWidgetItem(QString::number(cacheI[i].data[j-2], 16).toUpper()));
			cacheDTable->setItem(i, j,
					new QTableWidgetItem(QString::number(cacheD[i].data[j-2], 16).toUpper()));
		}
	}

	memory_line* _mem = mem.getMemory().return_storage();

	

	for (int i = 0; i < 16; i++) {
		regTable->setItem(i, 0,
			new QTableWidgetItem(QString::number(reg[i], 16).toUpper()));
			// new QTableWidgetItem(QString::number(reg[i], 16).toUpper()));
	}

	// QString("0x") + QString::number(i*32, 16).toUpper();

	regTable->setItem(16, 0,
		new QTableWidgetItem(QString::number(reg_buff, 2).toUpper()));


    for (word i = 0; i < MEMORY_LINE_SIZE; i++) {
		for (int j = 0; j < LINE_SIZE; j++) {
			memTable->setItem(i, j,
				new QTableWidgetItem(QString::number(_mem[i].data[j], 16).toUpper()));
		}
	}
pipeTable->setItem(0, 0,
		new QTableWidgetItem(QString("%1").arg(fetch_buffer, 8, 16, QChar('0'))));

	pipeTable->setItem(0, 1,
		new QTableWidgetItem(QString("%1").arg(decode_buffer >> 29, 1, 16, QChar('0').toUpper())));
	pipeTable->setItem(1, 1,
		new QTableWidgetItem(QString("%1").arg(decode_buffer >> 24, 2, 16, QChar('0')).toUpper()));

	pipeTable->setItem(0, 2,
		new QTableWidgetItem(QString("%1").arg(execute_buffer.op >> 5, 1, 16, QChar('0')).toUpper()));
	pipeTable->setItem(1, 2,
		new QTableWidgetItem(QString("%1").arg(execute_buffer.op, 2, 16, QChar('0')).toUpper()));
	pipeTable->setItem(2, 2,
		new QTableWidgetItem(QString("%1").arg(execute_buffer.dest, 3,  16, QChar('0')).toUpper()));
	pipeTable->setItem(3, 2,
		new QTableWidgetItem(QString("%1").arg(execute_buffer.in1, 3,  16, QChar('0')).toUpper()));
	pipeTable->setItem(4, 2,
		new QTableWidgetItem(QString("%1").arg(execute_buffer.in2, 3,  16, QChar('0')).toUpper()));
	pipeTable->setItem(5, 2,
		new QTableWidgetItem(QString("%1").arg(execute_buffer.shamt, 3,  16, QChar('0').toUpper())));

	pipeTable->setItem(0, 3,
		new QTableWidgetItem(QString("%1").arg(memory_buffer.ops >> 5, 1, 16, QChar('0')).toUpper()));
	pipeTable->setItem(1, 3,
		new QTableWidgetItem(QString("%1").arg(memory_buffer.ops, 2, 16, QChar('0')).toUpper()));
	pipeTable->setItem(7, 3,
		new QTableWidgetItem(QString("%1").arg(memory_buffer.dat, 3, 16, QChar('0')).toUpper()));
	pipeTable->setItem(6, 3,
		new QTableWidgetItem(QString("%1").arg(memory_buffer.mem, 3, 16, QChar('0')).toUpper()));
	pipeTable->setItem(8, 3,
		new QTableWidgetItem(QString("%1").arg(memory_buffer.reg, 3, 16, QChar('0')).toUpper()));

	qDebug() << "mem_buffer op = " << QString::number(memory_buffer.ops, 2);

	pipeTable->setItem(8, 4,
		new QTableWidgetItem(QString("%1").arg(writes_buffer.reg, 3, 16, QChar('0')).toUpper()));
	pipeTable->setItem(7, 4,
		new QTableWidgetItem(QString("%1").arg(writes_buffer.dat, 3, 16, QChar('0')).toUpper()));
}


MemoryHandler& CPU::returnManager() {
	return mem;
}

// void CPU::state(){
// 	printf("PC: %08x ", reg[XPC]);
// 	printf("ST: %08x ", reg[XSP]);
// 	printf("PIPE: %s ", (pipemode ? "True" : "False"));
// 	printf("HALT: %s ", (hlt ? "True" : "False"));
// 	printf("REGBUFF: %08X\n", reg_buff);
// 	printf("Negt FLAG: %X ", flagC(NFL));
// 	printf("oVer FLAG: %X ", flagC(VFL));
// 	printf("ZERO FLAG: %X ", flagC(ZFL));
// 	printf("Carr FLAG: %X\n", flagC(CFL));
// 	for (int i = 0; i < 16; i++)
// 		printf("REG[%i] ~ %08X ", i, reg[i]);
// 	printf("\n");
// 	printf("Fetch: %08X\n", fetch_buffer);
// 	printf("Decode: %08X\n", decode_buffer);
// 	printf("Execute: [%02X] [%01X] [%08X] [%08X] [%08X]\n",
// 		execute_buffer.op, execute_buffer.shamt,
// 		execute_buffer.dest, execute_buffer.in1, execute_buffer.in2
// 	);
// 	printf("Memory [%01X] [%02u] [%08X] [%08X]\n",
// 		memory_buffer.ops, memory_buffer.reg, memory_buffer.mem, memory_buffer.dat);
// 	printf("Write Back [%02u] [%08X]\n", writes_buffer.reg, writes_buffer.dat);
// }

// #include "cpu.moc"
