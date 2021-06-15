#include "MCPU.hpp"

#include <stdio.h>
#include <stdlib.h> 
#include <conio.h>
#include <fstream>
#include <iostream>
#include <iomanip>

MCPU::MCPU(uint8_t* memory, uint8_t mode) {
    this->memory = memory;
    this->mode = mode;
    
    PC = 0;
    
    Z = 0;
    N = 0;
    C = 0;
}

uint8_t MCPU::ALU(uint8_t a, uint8_t b, uint8_t mode) {
    uint8_t result;
    
    switch (mode) {
        case 1: //add
            result = a + b;
            break;
        case 2: //sub
            b = ~b;
            result = a + b + 1;
            break;
        case 3: //adc
            result = a + b + this->C;
            break;
        case 4: //sbb
            b = ~b;
            result = a + b + (~this->C);
            break;
        case 5: //not
            result = ~a;
            break;
        case 6: //and
            result = a & b;
            break;
        case 7: //or
            result = a | b;
            break;
    }
    
    /* flag setting */
    if (result == 0) {
        this->Z = 1;
    }
    else {
        this->Z = 0;
    }
    
    if ((result & 0b10000000) != 0) {
        this->N = 1;
    }
    else {
        this->N = 0;
    }
    
    if ((((a & 0b10000000) != 0) || ((b & 0b10000000) != 0)) && ((result & 0b10000000) == 0)) {
        this->C = 1;
    }
    else {
        this->C = 0;
    }
    
    return result;
}

bool MCPU::execute(uint8_t instr) {
    uint8_t opcode = (instr & 0b11111000) >> 3;
    uint8_t reg = (instr & 0b00000111);
    
    uint16_t MSB;
    uint16_t LSB;
    uint8_t offset;
    
    bool doJump = false;
    
    if ((opcode >= 21) && (opcode <= 24)) {
        if (reg == 0b000) {
            doJump = true;
        }
        if ((reg & 0b001) != 0) {
            if (this->Z == 1) {
                doJump = true;
            }
        }
        if ((reg & 0b010) != 0) {
            if (this->N == 1) {
                doJump = true;
            }
        }
        if ((reg & 0b100) != 0) {
            if (this->C == 1) {
                doJump = true;
            }
        }
    }
    
    switch (opcode) {
        case 0: //nop
            break;
        case 1: //add
        case 2: //sub
        case 3: //adc
        case 4: //sbb
        case 5: //not
        case 6: //and
        case 7: //ior
            dataRegs[0] = ALU(dataRegs[0], dataRegs[reg], opcode);
            break;
        case 8: //MOVs
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            dataRegs[reg] = dataRegs[opcode - 8];
            break;
        case 16: //ldi
            this->PC++;
            dataRegs[reg] = memory[PC];
            break;
        case 17: //ldd
            this->PC++;
            LSB = memory[PC];
            this->PC++;
            MSB = memory[PC];
            
            dataRegs[reg] = memory[(MSB << 8) | LSB];
            break;
        case 18: //ldx
            LSB = dataRegs[7];
            MSB = dataRegs[6];
            
            dataRegs[reg] = memory[(MSB << 8) | LSB];
            break;
        case 19: //std
            this->PC++;
            LSB = memory[PC];
            this->PC++;
            MSB = memory[PC];
            
            memory[(MSB << 8) | LSB] = dataRegs[reg];
            break;
        case 20: //stx
            LSB = dataRegs[7];
            MSB = dataRegs[6];
            
            memory[(MSB << 8) | LSB] = dataRegs[reg];
            break;
        case 21: //jpd
            if (doJump) {
                this->PC++;
                LSB = memory[PC];
                this->PC++;
                MSB = memory[PC];
                
                PC = (MSB << 8) | LSB;
            }
            else {
                PC += 2; //jump past address
            }
            break;
        case 22: //jpx
            if (doJump) {
                LSB = dataRegs[7];
                MSB = dataRegs[6];
                
                PC = (MSB << 8) | LSB;
            }
            break;
        case 23: //srd
            if (doJump) {
                /* save PC to stack */
                LSB = (PC | 0xff);
                MSB = (PC | 0xff00) >> 8;
                
                memory[SP] = MSB;
                SP++;
                memory[SP] = LSB;
                SP++;
                
                /* do jump */
                this->PC++;
                LSB = memory[PC];
                this->PC++;
                MSB = memory[PC];
                
                PC = (MSB << 8) | LSB;
            }
            else {
                PC += 2; //jump past address
            }
            break;
        case 24: //srx
            if (doJump) {
                /* save PC to stack */
                LSB = (PC | 0xff);
                MSB = (PC | 0xff00) >> 8;
                
                memory[SP] = MSB;
                SP++;
                memory[SP] = LSB;
                SP++;
                
                /* do jump */
                LSB = dataRegs[7];
                MSB = dataRegs[6];
                
                PC = (MSB << 8) | LSB;
            }
            break;
        case 25: //lsp
            this->PC++;
            LSB = memory[PC];
            this->PC++;
            MSB = memory[PC];
            
            SP = (MSB << 8) | LSB;
            break;
        case 26: //psh
            memory[SP] = dataRegs[reg];
            SP++;
            break;
        case 27: //pop
            SP--;
            dataRegs[reg] = memory[SP];
            break;
        case 28: //ret
            SP--;
            MSB = memory[SP];
            SP--;
            LSB = memory[SP];
            
            PC = (MSB << 8) | LSB;
            break;
        case 29: //ldo
            OUT = dataRegs[reg];
            switch(mode) {
                case 0: //int
                    printf("%d ", OUT);
                    break;
                case 1: //hex
                    printf("%02x ", OUT);
                    break;
                case 2: //char
                    printf("%c", OUT);
                    break;
            }
            break;
        case 30: //sti
            dataRegs[reg] = IN;
            IN = 0; //clear IN after read
            break;
        default: //nop
            return false; //DEBUG ONLY
            break; 
    }
    
    if (!doJump) {
        PC++;
    }
    
    return true;
}

void MCPU::setInput(uint8_t input) {
    this->IN = input;
}

void MCPU::run() {
    uint64_t cycle = 0;
    uint16_t keypress;
    
    std::ofstream ofile;
    ofile.open("debug.txt");
    
    bool different = false;
    uint8_t prevDataRegs[8] = {255};
    uint8_t prevIN = 255; //input register
    uint8_t prevOUT = 255; //output register
    uint16_t prevSP = 65535; //call stack pointer
    uint8_t pZ; //zero
    uint8_t pN; //negative
    uint8_t pC; //carry
    
    bool run = true;
    ofile << "Cycle     | A| B| C| D| E| F| G| H|IN|OT|CNZ| SP | PC |" << std::endl;
    while (run) {
        if (kbhit()) { //if key is pressed
            keypress = _getch();
            if (keypress == 0 || keypress == 224) {
                keypress = _getch();
            }
            setInput(keypress);
        }
        run = execute(memory[PC]);
        
        /* debug section */
        if (IN != prevIN || OUT != prevOUT || SP != prevSP || Z != pZ || N != pN || C != pC) {
            different = true;
        }
        
        prevIN = IN;
        prevOUT = OUT;
        prevSP = SP;
        pZ = Z;
        pN = N;
        pC = C;
        
        for (int i = 0; i < 8; i++) {
            if (dataRegs[i] != prevDataRegs[i]) {
                different = true;
            }
            prevDataRegs[i] = dataRegs[i];
        }
        
        if (different) {
            ofile << std::dec << std::setw(10) << std::setfill('0') << cycle << "|";
            for (int i = 0; i < 8; i++) { 
                ofile << std::hex << std::setw(2) << std::setfill(' ') << static_cast<int>(dataRegs[i]);
                if (i == 7) {
                    ofile << "|";
                } else {
                    ofile << " ";
                }
            }
            
            ofile << std::setw(2) << std::setfill(' ') << static_cast<int>(IN) << " "; 
            ofile << std::setw(2) << std::setfill(' ') << static_cast<int>(OUT) << "|"; 
            ofile << static_cast<int>(C) << static_cast<int>(N) << static_cast<int>(Z) << "|";
            ofile << std::setw(4) << std::setfill('0') << static_cast<int>(SP) << " ";
            ofile << std::setw(4) << std::setfill('0') << static_cast<int>(PC) << "|" << std::endl;
            
            different = false;
        }
        /* end debug section */
        
        cycle++;
    }
    
    ofile.close();
}