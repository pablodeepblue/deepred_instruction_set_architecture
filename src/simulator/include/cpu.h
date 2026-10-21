//
// Created by minec on 3/28/2025.
//

#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "isadefines.h"
#include "ppu.h"
#include "worker_pipe.h"

#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <chrono>
#include <thread>
#include <QThread>
#include <QString>

#include <QWidget>
#include <QApplication>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QRandomGenerator>
#include <iostream>
#include <QObject>
#include <QCheckBox>

//pipeline stages
// fetch -word-> decode -executable-> execute -update-> memory -update-> writeback
//decode
//execute
//memory
//writeback
//runs in reverse order

#define BLOCKED 0b00
#define IDLE 	0b01
#define END     0b10

#define NFL 0b1000
#define VFL 0b0100
#define ZFL 0b0010
#define CFL 0b0001

#define MEMPAS  NOP 	// memory pass to reg with nop
#define MEMLDW  LDUR 	// memory load word
#define MEMLDB  LDURB 	// memory load byte
#define MEMSTW  STUR 	// memory store word
#define MEMSTB  STURB 	// memory store byte

// can be thought of as op(in1, in2) -> dest
typedef struct executable{
	int32_t  in2 = 0;  //input 2 of operation
	int32_t  in1 = 0;  //input 1 of operation
	uint32_t dest = 0; //destination of operation (can be mem or reg)
	uint8_t shamt = 0;
	uint8_t op = NOP;   //operarion
} executable;

//passed to both memory and writeback
//mem will either load or store based
typedef struct mupdate{
	int32_t dat = 0; // if store M[mem] = data, if load data = M[mem]
	uint8_t ops = NOP; // if MEMPAS, squash
	uint8_t reg = 15; // location of where to store
	int32_t mem = 0; // location where to load/store
} mupdate;

typedef struct rupdate{
	int32_t dat = 0;
	uint8_t reg = 15; // reg 15 equiv to squash
} rupdate;

constexpr uint32_t NOP_WORD = NOP << 24;
constexpr uint32_t MAX_UINT32 = (uint32_t)(-1);
constexpr uint32_t MSB = 0x80000000;

class MemoryHandler{
	bool cache = true;
	Cache_lv1 memD;
	Cache_lv1 memI;
	Bus bus;

	public:
	MemoryHandler(Memory* ram_) : memD(ram_, L1D ), memI(ram_, L1I), bus(ram_, 0) {}

	Cache_lv1& getDataMemory(){
		return memD;
	}

	Cache_lv1& getInstructMemory(){
		return memI;
	}

	Bus& getBus(){
		return bus;
	}

	void print(){
		if(cache) {
			memD.print();
			memI.print();
		}
		else bus.print();
	}

	int memD_flush(byte_ stage){
		if (cache) return memD.flush(stage);
		else return bus.flush(stage);
	}

	Memory& getMemory(){
		return bus.getMemory();
	}

	void setCache(bool val) {
		cache = val;
	}

	byte_ memI_buffer_address(word address, byte_ stage){
		if (cache) return memI.buffer_address(address, stage);
		else return bus.buffer_address(address, stage);
	}
	byte_ memI_buffer_status(byte_ stage){
		if (cache) return memI.buffer_status(stage);
		else return bus.buffer_status(stage);
	}
	memory_line memI_load_buffer(byte_ stage){
		if (cache) return memI.load_buffer(stage);
		else return bus.load_buffer(stage);
	}

	byte_ memD_buffer_address(word address, byte_ stage){
		if (cache) return memD.buffer_address(address, stage);
		else return bus.buffer_address(address, stage);
	}
	byte_ memD_buffer_status(byte_ stage){
		if (cache) return memD.buffer_status(stage);
		else return bus.buffer_status(stage);
	}
	memory_line memD_load_buffer(byte_ stage){
		if (cache) return memD.load_buffer(stage);
		else return bus.load_buffer(stage);
	}

	// store
	byte_ memD_store_byte_(word address, byte_ data, byte_ stage){
		if (cache) return memD.store_byte_(address, data, stage);
		else return bus.store_byte_(address, data, stage);
	}

	byte_ memD_store_word(word address, word data, byte_ stage){
		if (cache) return memD.store_word(address, data, stage);
		else return bus.store_word(address, data, stage);
 	}
	
	cache_line* memD_return_storage(){
		return memD.return_storage();
	}

	cache_line* memI_return_storage(){
		return memI.return_storage();
	}

	bool is_cache(){
		return cache;
	}

	void reset(){
		memD.reset();
		memI.reset();
		bus.reset();
	}
};

class Worker_pipe;

class CPU : public QObject {
	Q_OBJECT
	// fetch -word-> decode -executable-> execute -update-> memory -update-> writeback

	// 	memory_line prefetch_buffer, uint32_t fetch_buffer=NOP_WORD,
	// 	decode_buffer, execute_buffer, memory_buffer, writes_buffer
private:
	Worker_pipe *worker;
	int hlt = 0;
	int pipemode = 0;
	int cycle_count = 0;
	QTableWidget *cacheTable;
	QTableWidget *cacheDTable;
	QTableWidget *memTable;
	QTableWidget *pipeTable;
	QTableWidget *regTable;
	QLabel *halt_display;
	QLabel *cycle_display;
	QLabel *pc;

	QLineEdit *memLoad_file;
	QLineEdit *multiStep_num;


	MemoryHandler mem;
	PPU ppu;
	uint32_t input = -1;
	int32_t reg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int16_t reg_buff = 0;
	uint8_t flags = 0; //XXXXNZVC
	uint32_t flower= (uint32_t)(-1);
	memory_line prefetch_buffer;
	uint32_t prefetch_buffer_ = 0;
	uint32_t fetch_buffer=NOP_WORD;
	uint32_t decode_buffer=NOP_WORD;
	// QApplication &app;

	// Data in each pipe stage
	executable execute_buffer = {
		.in2=0,
		.in1=0,
		.dest=0,
		.shamt=0,
		.op=NOP,
	};

	mupdate memory_buffer = {
		.dat=0,
	    .ops=MEMPAS,
		.reg=XZR,
		.mem=0,
	};

	rupdate writes_buffer = rupdate{
		.dat=0,
		.reg=XZR,
	};

	bool discard = false;

	//each func in charge of clearing thier buffer;
	//this is to avoid multiwrites

	inline void regF(int reg_);

	inline void regU(int reg_);

	inline int regC(int reg_);

	inline int flagC(int fn);

	void branch(mupdate& mup, executable& ex, int clear);

	inline uint8_t NFLA(uint64_t result);

	inline uint8_t VFLA(uint32_t a, uint32_t b, uint64_t result);

	inline uint8_t ZFLA(int64_t result);

	inline uint8_t CFLA(int64_t result);


	int fetch();

	// decoder will handle 2 complement all imm and regextracted values are signed
	int decode();

	int execute();

	int memory();

	int writeback();


	inline word address_to_tag(const word address);

	inline word address_to_index(const word address);

	int prefetch();

	void cycle();

public slots:
	void update_ui();

public:
	MemoryHandler& returnManager();

	inline void reset();

	void reset_all();

	void setWorker(Worker_pipe *workerObj);

	CPU(Memory *memory, PPU ppu_);

	Cache_lv1& getDataMemory();

	Cache_lv1& getInstructMemory();

	void state();

	int run();

	uint32_t F(int _blocked);

	executable D(int _blocked);

	mupdate E(int _blocked);

	rupdate M();

	void W();

	void clock_pipeline();

	void clock_nonpipeline();

	int run_nonpipline();

	int single_step_nonpipline();

	int run_pipline();

	int single_step_pipline();

	void open_ui(QApplication &app);

	char* getTextFromInput();

	int getStep();


};


#endif //CPU_H
