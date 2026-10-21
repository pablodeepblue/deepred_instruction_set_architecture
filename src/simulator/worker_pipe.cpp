// cpp

#include "worker_pipe.h"
#include "cpu.h"

Worker_pipe::Worker_pipe(CPU *cpu_) : cpu(cpu_) {
    // Here you are just initializing the member `cpu` with the passed pointer
}

void Worker_pipe::reset_sim() {
    // used for reference
    // cache_line* cache = cpu->getInstructMemory().return_storage();
    // memory_line* mem = cpu->getInstructMemory().getBus().getMemory().return_storage();


    // nopipe = false;
    cpu->reset_all();
    cpu->getDataMemory().reset();
    cpu->getInstructMemory().reset();

    cpu->getDataMemory().getMemory().reset();
    cpu->getInstructMemory().getMemory().reset();

    // cpu->getDataMemory().getBus().reset();
    // cpu->getInstructMemory().getBus().reset();

    emit update();
}
void Worker_pipe::setNoPipe() {
    if (nopipe == false) {
        nopipe = true;
    } else
        nopipe = false;
}

void Worker_pipe::setNoCache() {
    if (nocache == false) {
        nocache = true;
        cpu->returnManager().setCache(false);
        qDebug() << "nopipe = true";

    } else {
        nocache = false;
        cpu->returnManager().setCache(true);
    }

}

// Do stuff here

void Worker_pipe::run_pipe() {
    qDebug() << "Worker_pipe::run_pipe";
    int pip_c;
    // int pip_c = cpu->run_pipline();
    if (nopipe == false) {
        pip_c = cpu->run_pipline();
    } else {
        pip_c = cpu->run_nonpipline();
    }

    (void)(pip_c);
    emit update();
}

void Worker_pipe::single_step() {
    int pip_c;
    if (nopipe == false) {
        pip_c = cpu->single_step_pipline();
    } else{
        pip_c = cpu->single_step_nonpipline();
    }
    (void)(pip_c);
    emit update();
}

void Worker_pipe::multi_step() {
    int stepNum = cpu->getStep();
    qDebug() << stepNum;
    int pip_c;
    for (int i = 0; i < stepNum; i++) {
        qDebug() << i;
        if (nopipe == false) {
            pip_c = cpu->single_step_pipline();
        } else{
            pip_c = cpu->single_step_nonpipline();
        }
        (void)(pip_c);
    }
    emit update();
}

void Worker_pipe::load_mem() {

    memory_line *ram = cpu->getDataMemory().getMemory().return_storage();
    char* argv = cpu->getTextFromInput();

    if (file_reader(argv, ram, 0, 0, true)) {
        std::cout << "No Valid Argument" << std::endl;
    }
    emit update();
}

uint32_t Worker_pipe::big_endian_reformat(uint32_t big_endian_word) {
    return ((big_endian_word << 24) & 0xFF000000) |
    ((big_endian_word << 8) & 0x00FF0000) |
    ((big_endian_word >> 8) & 0x0000FF00) |
    ((big_endian_word >> 24)& 0x000000FF);
}

int Worker_pipe::file_reader(char filepath[], memory_line* mem, uint32_t offset, uint32_t fileoffset, bool big_endian){
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