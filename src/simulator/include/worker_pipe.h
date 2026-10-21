//
// Created by minec on 3/28/2025.
//

#ifndef WORKER_H
#define WORKER_H


//
// Created by minec on 3/28/2025.
//
#include <QApplication>
#include <QThread>
#include <QObject>
#include <QDebug>

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QRandomGenerator>
#include <iostream>

#include <iostream>
#include <string>
#include <string.h>
#include <cctype>
#include <type_traits>
#include <fstream>
#include <QDebug>


#include "memory.h"
#include "cpu.h"
// #include "cpu.cpp"


class CPU;

class Worker_pipe : public QObject {
private:
    Q_OBJECT // Required for signals and slots
    CPU *cpu;
    bool nopipe = false;
    bool nocache = false;

    int file_reader(char filepath[], memory_line* mem, uint32_t offset, uint32_t fileoffset, bool big_endian);
    uint32_t big_endian_reformat(uint32_t big_endian_word);

signals:
    void update();
    void loadMem_sig();

public slots:
    void run_pipe();
    void single_step();
    void load_mem();
    void multi_step();
    void reset_sim();
    void setNoPipe();
    void setNoCache();

public:
    explicit Worker_pipe(CPU *cpu);

};



#endif //WORKER_H
