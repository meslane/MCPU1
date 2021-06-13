import sys

def error():
    print("Assembly halted")
    sys.exit(1)

def detectInvalidChars(validChars, input):
    for c in input:
        if c not in validChars:
            return True
            
    return False
    
def printHex(data):
    return True #TODO: implement

inf = open(sys.argv[1], "r")

outFileName = sys.argv[1].split(".",1)[0] + ".mcpu" #remove file type and replace with .mcpu
if (len(sys.argv) > 2):
    outFileName = sys.argv[2]
    
outf = open(outFileName, "wb")

inData = inf.read().splitlines() #split into list
outData = [0] * 65536
dPointer = 0 #current memory address to be written

instList = (
"nop",
"add",
"sub",
"adc",
"sbb",
"not",
"and",
"ior",
"mva",
"mvb",
"mvc",
"mvd",
"mve",
"mvf",
"mvg",
"mvh",
"ldi",
"ldd",
"ldx",
"std",
"stx",
"jpd",
"jpx",
"srd",
"srx",
"lsp",
"psh",
"pop",
"ret",
"ldo",
"sti",
"hcf")

labelTable = {} #table for address labels

n = 1
for line in inData:
    line = line.lower().split()
    
    if (len(line) > 0):
        #invalid instruction
        if (("label" not in line[0]) and (line[0] not in instList)):
            print("ERROR: line {} - invalid macro '{}'".format(n, line[0]))
            error()
        
        #address labels
        if ("label" in line[0]):
            if (len(line) > 2): #defined address
                labelTable[line[1]] = int(line[2], 0)
                dPointer = int(line[2], 0) #update address to label index
            else: #assumed address
                labelTable[line[1]] = dPointer
        
        #nop
        if (line[0] == "nop"):
            dPointer += 1 #no nothing but advance the pointer
        
        #ALU and MOV ops + all ops greater than 25
        try:
            if ((instList.index(line[0]) > 0 and instList.index(line[0]) < 15) or (instList.index(line[0]) > 25)):
                opcode = instList.index(line[0])
                reg = 0
                
                if (len(line) > 1):
                    reg = ord(line[1]) - 97
                
                outData[dPointer] = (opcode << 3) | reg
                dPointer += 1
        except ValueError:
            pass
        
        #ldi
        if (line[0] == "ldi"):
            opcode = 16
            reg = ord(line[1]) - 97
            immediate = int(line[2], 0)
            
            if (immediate > 255 or immediate < -128):
                print("ERROR: line {} - immediate value of {} overflows one byte".format(n, line[2])) 
                error()
                
            if (immediate < 0):
                immediate &= 0xff
            
            outData[dPointer] = (opcode << 3) | reg
            dPointer += 1
            outData[dPointer] = immediate
            dPointer += 1
        
        #direct address instructions
        if (line[0] == "ldd" or line[0] == "std" or line[0] == "jpd" or line[0] == "srd"):
            opcode = instList.index(line[0])
            reg = 0
            
            if (line[0] == "ldd" or line[0] == "std"):
                reg = ord(line[1]) - 97
            else: #jump conditions
                if ("z" in line[1]):
                    reg |= 0b001
                if ("n" in line[1]):
                    reg |= 0b010
                if ("c" in line[1]):
                    reg |= 0b100
                if (detectInvalidChars(['z','n','c'], line[1]) and line[1] != "0"):
                    print("ERROR: line {} - invalid condition '{}'".format(n, line[1]))
                    error()
            
            try:
                msb = (int(line[2], 0) & 0xff00) >> 8
                lsb = (int(line[2], 0) & 0xff)
            except ValueError:
                try:
                    msb = (labelTable[line[2]] & 0xff00) >> 8
                    lsb = (labelTable[line[2]] & 0xff)
                except KeyError:
                    print("ERROR: line {} - invalid address or label '{}'".format(n, line[2]))
                    error()
            
            outData[dPointer] = (opcode << 3) | reg
            dPointer += 1
            outData[dPointer] = lsb
            dPointer += 1
            outData[dPointer] = msb
            dPointer += 1
            
        #indexed address instructions
        if (line[0] == "ldx" or line[0] == "stx" or line[0] == "jpx" or line[0] == "srx"):
            opcode = instList.index(line[0])
            reg = 0
            
            if (line[0] == "ldx" or line[0] == "stx"):
                reg = ord(line[1]) - 97
            else: #jump conditions
                if ("z" in line[1]):
                    reg |= 0b001
                if ("n" in line[1]):
                    reg |= 0b010
                if ("c" in line[1]):
                    reg |= 0b100
                if (detectInvalidChars(['z','n','c'], line[1]) and line[1] != "0"):
                    print("ERROR: line {} - invalid condition '{}'".format(n, line[1]))
                    error()
                    
            outData[dPointer] = (opcode << 3) | reg
            dPointer += 1
            
        #load stack pointer
        if (line[0] == "lsp"):
            opcode = 25
            
            try:
                msb = (int(line[1], 0) & 0xff00) >> 8
                lsb = (int(line[1], 0) & 0xff)
            except ValueError:
                try:
                    msb = (labelTable[line[1]] & 0xff00) >> 8
                    lsb = (labelTable[line[1]] & 0xff)
                except KeyError:
                    print("ERROR: line {} - invalid address or label '{}'".format(n, line[1]))
                    error()
            
            outData[dPointer] = (opcode << 3)
            dPointer += 1
            outData[dPointer] = lsb
            dPointer += 1
            outData[dPointer] = msb
            dPointer += 1
        
    n += 1

#trim output
while(outData[-1] == 0):
    del outData[-1]

fArray = bytearray(outData)
outf.write(fArray)

inf.close()
outf.close()