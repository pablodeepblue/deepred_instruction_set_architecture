// Created by minec on 3/5/2025.
//
#include "memory.h"
#include <iostream>
#include <memory.h>

// Class Memory ----------------------------------------------------------------
int Memory::cycle(){
    if (--cooldown == 0) blocked = false;
    return 1;
}

memory_line* Memory::return_storage() {
    return memory_data;
}

byte_ Memory::flush_cycle(const byte_ stage){
    if(stagesrv == stage){
        cycle();
    }
    return stagesrv == stage;
}

// load instructions
byte_ Memory::buffer_address(const word line, const byte_ stage){
    if (stagesrv == 0) stagesrv = stage;
    if (stagesrv != stage) return blocked_error;
    if (blocked & cycle()) return cache_wait;
    for (int i = 0; i < LINE_SIZE; i++) buffer.data[i] = memory_data[line].data[i];
    block();
    return cache_wait;
}

byte_ Memory::buffer_status(const byte_ stage) {
    if (stagesrv != stage) return false;
    else  return (!blocked);
}

memory_line Memory::load_buffer(const byte_ stage) {
    if (blocked || (stagesrv != stage)) return memory_line{};
    else {
        stagesrv = 0;
        return buffer;
    }
}

//store instructions
byte_ Memory::store_byte_(const word line, const word index, const word offset, const byte_ data, byte_ stage){
    //printf("[Memory] storing byte_ line %u index %u offset %u data %u\n", line, index, offset, data);
    if (stagesrv == 0)  stagesrv = stage;
    if (stagesrv != stage) return blocked_error;
    if (blocked && cycle()) return cache_wait;
    const word byte_pos = (3 - offset) << 3;
    const word mask = 0xFF << byte_pos;
    memory_data[line].data[index] &= ~mask;
    memory_data[line].data[index] |= data << byte_pos;
    block();
    return cache_wait;
}

byte_ Memory::store_word(const word line, const word index, const word data, byte_ stage){
    ////printf("[Memory] storing word line %u index %u data %u\n", line, index, data);
    if (stagesrv == 0)  stagesrv = stage;
    if (stagesrv != stage) return blocked_error;
    if (blocked && cycle()) return cache_wait;
    memory_data[line].data[index] = data;
    block();
    return cache_wait;
}

byte_ Memory::store_line(const word line, const cache_line &data, const byte_ stage){
    //printf("[Memory] storing line %u\n", line);
    if (stagesrv == 0)  stagesrv = stage;
    if (stagesrv != stage) return blocked_error;
    if (blocked && cycle()) return cache_wait;
    for (int i = 0; i < LINE_SIZE; i++)
        memory_data[line].data[i] = data.data[i];
    block();
    return cache_wait;
}

byte_ Memory::flush(const byte_ stage){
    if (stagesrv == 0) {
        blocked = false;
        cooldown = 0;
        stagesrv = stage;
    }
    if (stagesrv != stage) return blocked_error;
    return cache_wait;
}

// Class Bus -------------------------------------------------


Memory& Bus::getMemory() {
    return *ram;
}

void Bus::sdl_startup(){ }

byte_ Bus::flush_cycle(const byte_ stage){
    return ram->flush_cycle(stage);
}

// load
byte_ Bus::buffer_address(const word address, const byte_ stage){
    return ram->buffer_address(address_to_line(address), stage);
}

byte_ Bus::buffer_status(const byte_ stage){
    return ram->buffer_status(stage);
}

memory_line Bus::load_buffer(const byte_ stage) {
    return ram->load_buffer(stage);
}

// store
byte_ Bus::store_byte_(const word address, const byte_ data, const byte_ stage){
    return ram->store_byte_(address_to_line(address), address_to_index(address), address_to_offset(address), data, stage);
}

byte_ Bus::store_word(const word address, const word data, const byte_ stage){
    return ram->store_word(address_to_line(address), address_to_index(address), data, stage);
}

byte_ Bus::store_line(const word address, const cache_line &data, const byte_ stage) {
    return ram->store_line(address_to_line(address), data, stage);
}

cache_line* Bus::return_storage() {
    return NULL;
}

byte_ Bus::flush(const byte_ stage){
    return ram->flush(stage);
}


// move bus here

cache_line buffer_miss;

cache_line* Cache_lv1::return_storage() {
    return cache_memory;
}

Bus& Cache_lv1::getBus() {
    return bus;
}

inline void Cache_lv1::block(){
    blocked = true;
    cooldown = 1;
}

inline void Cache_lv1::unblock(){
    blocked = false;
    cooldown = 0;
    status = cache_hit;
}

inline void Cache_lv1::from_cache_to_mem(const cache_line &from, memory_line &to){
    for (int i = 0; i < LINE_SIZE; i++) to.data[i] = from.data[i];
}

Memory& Cache_lv1::getMemory() {
    return bus.getMemory();
}

void Cache_lv1::buff(const word address){
    buffer_addr_load = address;
    status = cache_fetch;
    cooldown = 255;
}

inline void Cache_lv1::set_load(const word address){
    buff(address);
    blocked = true;
    store = false;
    byte_mode = false;
    cooldown = 255;
}

inline void Cache_lv1::set_store_word(const word address, const word data){
    buff(address);
    blocked = true;
    store = true;
    byte_mode = false;
    store_data = data;
    cooldown = 255;
}

inline void Cache_lv1::set_store_byte_(const word address, const byte_ data){
    buff(address);
    blocked = true;
    store = true;
    byte_mode = true;
    store_data = data;
    cooldown = 255;
}

inline void Cache_lv1::write_word(const word tag, const byte_ index, const byte_ offset_word, const word data){
    cache_memory[index].update = true;
    //printf("UPDATED %d %d %d\n", cache_memory[index].update, offset_word, index);
    cache_memory[index].tag = tag;
    cache_memory[index].data[offset_word] = data;
}

inline void Cache_lv1::write_byte_(const word tag, const byte_ index, const byte_ offset_word, const byte_ offset_byte_, const byte_ data){
    cache_memory[index].update = true;
    cache_memory[index].tag = tag;
    word byte_pos = (3 - offset_byte_) << 3;
    word mask = 0xFF << byte_pos;
    cache_memory[index].data[offset_word] &= ~mask;
    cache_memory[index].data[offset_word] |= data << byte_pos;
}

int Cache_lv1::cycle(){
    //printf("cache_cycle (%02X)(%02X)\n", label, stagesrv);
    write_through();
    if (cooldown != 0) {
        cooldown--;
    }
    if(cooldown == 0) blocked = false;
    if (status == cache_fetch){
        if (bus.buffer_address(buffer_addr_load, label) == cache_wait && bus.buffer_status(label)) {
	        status = cache_hit;
            unblock();
            const word tag = address_to_tag(buffer_addr_load);
            const byte_ index = address_to_index(buffer_addr_load);
            memory_line mem = bus.load_buffer(label);
            if (cache_memory[index].valid && cache_memory[index].update){
                buffer_miss.tag = cache_memory[index].tag;
                buffer_miss_index = index;
                for (int i = 0; i < LINE_SIZE; i++) buffer_miss.data[i] = cache_memory[index].data[i];
                status = cache_miss;
            }
            cache_memory[index].tag = tag;
            cache_memory[index].valid = true;
            cache_memory[index].update = false;
            //printf("LOADED %08X %d\n", data_to_address(cache_memory[index].tag, index, 0, 0), cache_memory[index].valid);
            for (int i = 0; i < LINE_SIZE; i++)
                cache_memory[index].data[i] = mem.data[i];
            if (store){
                const byte_ offset_word = address_to_offset_word(buffer_addr_load);
                if(byte_mode){
                    const byte_ offset_byte_ = address_to_offset_word(buffer_addr_load);
                    write_byte_(tag, index, offset_word, offset_byte_, store_data);
                }
                else{
                    write_word(tag, index, offset_word, store_data);
                }
            }
            from_cache_to_mem(cache_memory[index], buffer);
        }
    }
    write_through();
    return 1;
}

// load
byte_ Cache_lv1::buffer_address(const word address, const byte_ stage){
    if (stagesrv == 0)  stagesrv = stage;
    if (stagesrv != stage) return blocked_error;
    if (blocked && cycle()) return cache_wait; 
    const word tag = address_to_tag(address);
    const byte_ index = address_to_index(address); 
    if (cache_memory[index].tag == tag && cache_memory[index].valid){
        from_cache_to_mem(cache_memory[index], buffer);
        blocked = false;
        cache_hit_count++;
        return cache_wait;
    }
    //printf("STORED [LDA] %08X ", address);
    set_load(address);
    cache_miss_count++;
    return cache_miss;
}

byte_ Cache_lv1::buffer_status(const byte_ stage) {
    if (stagesrv != stage) return false;
    else  return (!blocked);
}

byte_ Cache_lv1::flush_cycle(const byte_ stage){
    if (stagesrv == stage){
        cycle();
        bus.flush_cycle(label);
    }
    return stagesrv == stage;
}

memory_line Cache_lv1::load_buffer(const byte_ stage) {
    if (blocked || (stagesrv != stage)) return memory_line{};
    else {
        stagesrv = 0;
        return buffer;
    }
}

// store
byte_ Cache_lv1::store_byte_(const word address, const byte_ data, const byte_ stage){
    if (stagesrv == 0)  stagesrv = stage;
    if (stagesrv != stage) return blocked_error;
    if (blocked && cycle()) return cache_wait;
    const word tag = address_to_tag(address);
    const byte_ index = address_to_index(address);
    if (cache_memory[index].tag == tag && cache_memory[index].valid){
        status = cache_hit;
        const byte_ offset_word = address_to_offset_word(address);
        const byte_ offset_byte_ = address_to_offset_byte_(address);
        write_byte_(tag, index, offset_word, offset_byte_, data);
        cache_hit_count++;
        return cache_wait;
    }
    set_store_byte_(address, data);
    cache_miss_count++;
    return cache_miss;
}



byte_ Cache_lv1::store_word(const word address, const word data, const byte_ stage){
    if (stagesrv == 0) stagesrv = stage;
    if (stagesrv != stage) return blocked_error;
    if (blocked && cycle()) return cache_wait;
    //printf("STORED %08X ", address);
    const word tag = address_to_tag(address);
    const byte_ index = address_to_index(address);
    if (cache_memory[index].tag == tag && cache_memory[index].valid){
        status = cache_hit;
        const byte_ offset_word = address_to_offset_word(address);
        write_word(tag, index, offset_word, data);
        cache_hit_count++;
        return cache_wait;
    }
    //printf("MISS current:%08X\n", data_to_address(cache_memory[index].tag, index, 0, 0));
    set_store_word(address, data);
    cache_miss_count++;
    return cache_miss;
}

void Cache_lv1::write_through() {
    if (status == cache_miss) {
        //printf("[CACHE] WRITE TO MEM\n");
        const word address = data_to_address(buffer_miss.tag, buffer_miss_index, 0, 0);
        if (bus.store_line(address, buffer_miss, label) == cache_wait && bus.buffer_status(label)){
            bus.load_buffer(label);
            status = cache_hit;
            unblock();
            return;
        }
        blocked = true;
        cooldown = 255;
        return;
    }
}

byte_ Cache_lv1::flush(const byte_ stage){
    if (stagesrv == 0) {
        flush_count = 0;
        stagesrv = stage;
    }
    if (stagesrv != stage) return blocked_error;
    cache_line cl = cache_memory[flush_count];
    while (!cl.valid || !cl.update) {
        //printf("bye %u", flush_count);
        flush_count++;
        if (flush_count == 32) {
            blocked = false;
            cooldown = 0;
            return cache_wait;
        }
        cl = cache_memory[flush_count];
    }
    const word address = data_to_address(cl.tag, flush_count, 0, 0);
    if(bus.store_line(address, cl, label) == cache_wait && bus.buffer_status(label)) {
        cache_memory[flush_count].update = false;
        bus.load_buffer(label);
        flush_count++;
    }
    if (flush_count == 32) {
        blocked = false;
        cooldown = 0;
    }
    else {
        blocked = true;
        cooldown = 255;
    }
    return cache_wait;
} 
