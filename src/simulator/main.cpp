#include "memory.h"
#include <QApplication>

//#include "cpu.cpp"
#include "ppu.h"
#include "cpu.h"
#include "worker_pipe.h"

#include <iostream>
#include <string>
#include <string.h>
#include <cctype>
#include <type_traits>
#include <fstream>
#include <QDebug>



// fix so it is fetch wait return
// allow unaligned data?


uint32_t big_endian_reformat(uint32_t big_endian_word) {
    return ((big_endian_word << 24) & 0xFF000000) |
    ((big_endian_word << 8) & 0x00FF0000) |
    ((big_endian_word >> 8) & 0x0000FF00) |
    ((big_endian_word >> 24)& 0x000000FF);
}

int file_reader(char filepath[], memory_line* mem, uint32_t offset, uint32_t fileoffset, bool big_endian){
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return 1;
    }
    printf("reading %s\n", filepath);
    uint32_t read;
    uint32_t c = offset;
    while (file.read(reinterpret_cast<char*>(&read), sizeof(read))) {
        if (fileoffset != 0){
            fileoffset--;
            continue;
        }
        // Here word will hold a 32-bit value from the file
        uint32_t actual = big_endian ? big_endian_reformat(read) : read;
        printf("Read word (%X/%X): %08X\n", c, MAXMEMORY/4, actual);
        mem[c/LINE_SIZE].data[c%LINE_SIZE] = actual;
        c++;
        if (c >= MAXMEMORY/4) return 1;
    }
    file.close();
    printf("OK\n");
    return 0;

}

//NVFC
int main(int argc, char *argv[]) {

    Memory memory;
    memory_line *ram = memory.return_storage();
    PPU ppu(
        ram
    );


    CPU cpu2(&memory, ppu);

    // qDebug() << argv[1];
    //
    // if(argc > 1 && file_reader(argv[1], ram, 0, 0, true)) {
    //     printf("No args provided or file does not exist\n");
    //     return 1;
    // }

    printf("Cache No Pipeline\n");

    QThread *threads;
    Worker_pipe *workers;

    workers = new Worker_pipe(&cpu2);  // Initialize worker
    cpu2.setWorker(workers);

    threads = new QThread();      // Initialize thread

    workers->moveToThread(threads);

    // Start work when thread starts
    // QObject::connect(threads, &QThread::started, workers, &Worker_pipe::run_pipe);
    threads->start();



    // //cpu2.getDataMemory().print();
    // printf("----OAM--------\n");
    // memory.print(OAM_T/32, NAMETABLE_T/32);
    // printf("----NAMETABLE--------\n");
    // memory.print(NAMETABLE_T/32, ATR_T/32);
    // printf("----ATR_T--------\n");
    // memory.print(ATR_T/32, PALLET_T/32);
    // printf("----PALLET_T--------\n");
    // memory.print(PALLET_T/32, TILE_T/32);
    // printf("----TILE_T-------\n");
    // memory.print(TILE_T/32);
    // printf("----END-------\n");

    QApplication app(argc, argv);  // create app
    cpu2.open_ui(app); // boot up ui, will loop forever

    return 0;  //
}


