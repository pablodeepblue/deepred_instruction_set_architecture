#ifndef MEMORY_H
#define MEMORY_H
#include "isadefines.h"
#include <cstdio>


#define LINE_SIZE 8
#define INDEX_BITS 32 // num of registers, not really bits
#define MAXMEMORY 262144 // 256KB, 2^18

#define blocked_error   0xff
#define cache_wait      0b1000
#define cache_fetch     0b0100
#define cache_miss      0b0010
#define cache_hit       0b0001

#define L1I       0b1000
#define L1D       0b0100
#define MEMORY    0b0010
#define FETCH     0b0001

constexpr word log2(word value){
    if (value <= 0) return -1;
    if (value == 1) return 0;
    return 1 + log2(value/2);
}

constexpr word LINE_SIZE_BYTE = LINE_SIZE*4;
constexpr word LINE_SIZE_BYTE_LOG = log2(LINE_SIZE_BYTE);
constexpr word LINE_SIZE_BYTE_MASK = (1 << LINE_SIZE_BYTE_LOG) - 1;
// constexpr word MEMORY_LINE_SIZE = MAXMEMORY/(LINE_SIZE*4) + 1;
constexpr word MEMORY_LINE_SIZE = MAXMEMORY/LINE_SIZE_BYTE;


typedef struct memory_line {
    word data[LINE_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0}; // force 0 at init
} memory_line;

typedef struct cache_line {
    bool valid = false;
    bool update = false;
    word tag = 0;
    word data[LINE_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0}; // force 0 at init
} cache_line;

class Memory{
    private:
        byte_ blocked = false;
        byte_ cooldown = 0;
        memory_line memory_data[MEMORY_LINE_SIZE]; // 2000 * 8 * 4 = 65kb
        memory_line buffer;
        byte_ stagesrv = 0;

        constexpr void block(){
            blocked = true;
            cooldown = 99;
        }

        int cycle();

    public:

        void reset() {
            blocked = false;
            cooldown = 0;
            buffer = memory_line();
            stagesrv = 0;

            for (word i = 0; i < MEMORY_LINE_SIZE; i++){
                memory_data[i] = memory_line();
            }
        }

        Memory(){} // constructor

        void remove_srv(){
            stagesrv = 0;
        }

        memory_line* return_storage();

        void print(){
            print(0, MEMORY_LINE_SIZE);
        }

        void print(int line){
            print(line, MEMORY_LINE_SIZE);
                
        }

        void print(uint32_t line, uint32_t endline){
            for (uint32_t i = line; i < endline; i++ ){
                printf("(%08X) ", i << 5);
                for (word d : memory_data[i].data ){
                    printf("%08X ", d);
                }
                printf("\n");
            }
                
        }

        void print_dec(){
            for (uint32_t i = 0; i < MEMORY_LINE_SIZE; i++ ){
                printf("(%08X) ", i << 5);
                for (word d : memory_data[i].data ){
                    printf("%d ", d);
                }
                printf("\n");
            }
                
        }

        byte_ flush_cycle(byte_ stage);

        // load instructions
        byte_ buffer_address(word line, byte_ stage);

        byte_ buffer_status(byte_ stage);

        memory_line load_buffer(byte_ stage);

        //store instructions
        byte_ store_byte_(word line, word index, word offset, byte_ data, byte_ stage);

        byte_ store_word(word line, word index, word data, byte_ stage);

        byte_ store_line(word line, const cache_line &data, byte_ stage);

        byte_ flush(const byte_ stage);
};

class Bus {
    private:
        Memory* ram;

        word t = 0;

        memory_line print_ML(memory_line ml){
            printf("\nPRINTML (%08X)", t);
            for (word w : ml.data)
                printf("%08x ", w);
            printf("\n");
            return ml;
        }

        static inline constexpr word address_to_line(const word address){
            return address >> 5;
        }
        
        static inline constexpr word address_to_index(const word address){
            return (address>>2) & 0b111;
        }        

        static inline constexpr word address_to_offset(const word address){
            return address & 0b11;
        }
    
    public:
        int cache_miss_count = 0;
        int cache_hit_count = 0;
        
        Bus(Memory* ram_, byte_ label_) : ram(ram_) {
            (void)(label_);
        }

        void print(){
            ram->print();
        }

        int flush(){
            return 0;
        }

        byte_ flush_cycle(byte_ stage);

        Memory& getMemory();

        void sdl_startup();
        
        // cycle
        void cycle();
        // load
        byte_ buffer_address(word address, byte_ stage);
        byte_ buffer_status(byte_ stage);
        memory_line load_buffer(byte_ stage);

        // store
        byte_ store_byte_(word address, byte_ data, byte_ stage);

        byte_ store_word(word address, word data, byte_ stage);

        byte_ store_line(word address, const cache_line &data, byte_ stage);

        static bool iscache() {
            return false;
        }
        
        cache_line* return_storage();

        void reset(){
            cache_miss_count = 0;
            cache_hit_count = 0;
            ram->reset();
        }

        byte_ flush(const byte_ stage);
};

// move bus here
class Cache_lv1{
    private:
        byte_ label;
        byte_ blocked = false;
        byte_ status = cache_hit;
        byte_ cooldown = 0;
        int flush_count = 0;
        cache_line cache_memory[32];
        memory_line buffer;
        Bus bus;
        byte_ stagesrv = 0;
        
        word buffer_addr_load = 0;
        word store_data = 0;
        bool byte_mode = false;
        bool store = false;

        cache_line buffer_miss;
        byte_ buffer_miss_index = 0;

        inline void block();

        inline void unblock();

        static inline word address_to_tag(const word address){
            return address >> 10;
        }

        static inline byte_ address_to_index(word address){
            return (address >> 5) & 0b11111;
        }

        static inline byte_ address_to_offset_word(const word address){
            return (address >> 2) & 0b111;
        }

        static inline byte_ address_to_offset_byte_(word address){
            return address & 0b11;
        }

        static inline word data_to_address(word tag,
            byte_ index, byte_ offset_word, byte_ offset_byte_){
            return (tag << 10) | (index << 5) | (offset_word << 2) | offset_byte_;
        }

        static inline void from_cache_to_mem(const cache_line &from, memory_line &to);



        void buff(word address);

        inline void set_load(word address);

        inline void set_store_word(word address, word data);

        inline void set_store_byte_(word address, byte_ data);

        inline void write_word(word tag, byte_ index,
            byte_ offset_word, word data);

        inline void write_byte_(word tag, byte_ index,
            byte_ offset_word, byte_ offset_byte_, byte_ data);

        int cycle();

        void write_through();

    public:
        int cache_miss_count = 0;
        int cache_hit_count = 0;

        Memory &getMemory();

        void reset() {
            printf("(%d) Hit: %d\n", label,cache_hit_count);
            printf("(%d) Miss: %d\n", label,cache_miss_count);
            cache_miss_count = 0;
            cache_hit_count = 0;
            blocked = false;
            status = cache_hit;
            cooldown = 0;
            for (int i = 0; i < 32; i++) {
                cache_memory[i] = cache_line();
            }
            buffer = memory_line();
            stagesrv = 0;
            buffer_addr_load = 0;
            store_data = 0;
            byte_mode = false;
            store = false;
            buffer_miss = cache_line();
            buffer_miss_index = 0;
        }

        inline void print() const {
            for (int j = 0; j < 32; j++){
                printf("VALID %d DIRTY %d (%08x)",cache_memory[j].valid, cache_memory[j].update,
                    data_to_address(cache_memory[j].tag, j, 0, 0));
                for (const word i : cache_memory[j].data) printf("%08X ", i);
                printf("\n");
            }
            printf("\n");
        
        }
        
        Cache_lv1(Memory *memory, byte_ label_ ) : bus(memory, label_) {
            label = label_;
        }
        
        byte_ flush_cycle(byte_ stage);

        // get the bus object
        Bus& getBus();

        // return cache
        cache_line* return_storage();
        
        // load
        byte_ buffer_address(word address, byte_ stage);

        byte_ buffer_status(byte_ stage);

        memory_line load_buffer(byte_ stage);

        // store
        byte_ store_byte_(word address, byte_ data, byte_ stage);

        byte_ store_word(word address, word data, byte_ stage);

        byte_ flush(byte_ stage);

        static bool iscache() {
            return true;
        }

};
#endif
