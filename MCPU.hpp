#ifndef __MCPU_HPP__
#define __MCPU_HPP__

#include <cstdint>

class MCPU {
    private:
        uint8_t* memory; 
        uint8_t mode;
        uint8_t dataRegs[8];
        
        uint8_t IN; //input register
        uint8_t OUT; //output register
        
        uint16_t PC; //program counter
        uint16_t SP; //call stack pointer
        
        /* flags */
        uint8_t Z; //zero
        uint8_t N; //negative
        uint8_t C; //carry
        
        uint8_t ALU(uint8_t a, uint8_t b, uint8_t mode);
        void execute(uint8_t instr);
        
    public:
        MCPU(uint8_t* memory, uint8_t mode); 
        void setInput(uint8_t input);
        void run();
};

#endif