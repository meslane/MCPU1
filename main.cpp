#include "MCPU.hpp"

#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string> 

using namespace std;

int main(int argc, char** argv) {
    uint8_t memory[65536];
    string allocation;
    uint16_t romSpace;
    
    ifstream f(argv[1], ios::out | ios::binary);
    ifstream cfg("config.txt");
    
    if (!f) {
        cout << "ERROR: cannot read file " << argv[1] << endl;
        return 1;
    }
    if (!cfg) {
        cout << "ERROR: cannot read config file" << endl;
        return 1;
    }
    
    getline(cfg, allocation);
    romSpace = static_cast<uint16_t>(atoi(allocation.c_str()));
    
    f.read((char*)&memory, romSpace);
    
    cout << "Read " << f.gcount() << "/" << romSpace << " bytes from file " << argv[1] << " ";
    cout << "(" <<  (1 - (static_cast<float>(f.gcount())/romSpace)) * 100 << "% free)" << endl;
    
    f.close();
    cfg.close();
    
    cout << "Starting simulator:" << endl << endl;
    MCPU cpu(memory);
    
    cpu.run();
    
    return 0;
}