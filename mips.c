#include "mips.h"

/******************************************************************************
 *      DO NOT MODIFY THE CODE BELOW
 ******************************************************************************/
#ifndef CS2100_ROUTINE_MASK_OUT
uint32_t _PC;
int32_t _RF[32]                   = {0};
int32_t _DataMemory[MAX_MEM >> 2] = {0};

// All the control signals needed
bool _RegDst;
bool _ALUSrc;
bool _MemtoReg;
bool _RegWrite;
bool _MemRead;
bool _MemWrite;
bool _Branch;
uint8_t _ALUOp;
uint8_t _ALUCtrl;

void RegFile(uint8_t RR1, uint8_t RR2, uint8_t WR, int32_t WD, int32_t* RD1,
             int32_t* RD2, bool RegWrite) {
    // Because we need to send out multiple outputs,
    // we will use passing by pointers.
    *RD1 = (RR1 > 0) ? _RF[RR1] : 0;
    *RD2 = (RR2 > 0) ? _RF[RR2] : 0;

    if (RegWrite && WR) _RF[WR] = WD;
}

int32_t Memory(uint32_t Address, int32_t WrData, bool MemRead, bool MemWrite) {
    // We can do a sanity check here.
    // You can at most do one memory operation.
    // Will assume that "error" raises hell.
    if (MemRead && MemWrite) {
        error("Cannot do both read and write at the same time.");
    }

    if (!(MemRead || MemWrite)) {
        return 0;
    }

    if ((Address > MAX_MEM) || (Address > MAX_MEM)) {
        error("Address %u is out of range. Simulator exiting...\n", Address);
    }

    if (MemRead) {
        return _DataMemory[Address >> 2];
    }

    if (MemWrite) {
        _DataMemory[Address >> 2] = WrData;
    }

    return 0;
}
#endif
/******************************************************************************
 *      DO NOT MIDIFY THE CODE ABOVE
 ******************************************************************************/

// Here starts your code for Assignment 2 Part A
// If you need to define some macros, you can do so below this comment.
#define MUX_FUNCTION(ctrl, in1, in0) ((ctrl) ? (in1) : (in0))


#ifndef ASSIGNMENT2_QUESTION1A

uint8_t mux_u8(bool ctrl, uint8_t in0, uint8_t in1) {
    return MUX_FUNCTION(ctrl, in0, in1);
}

uint32_t mux_u32(bool ctrl, uint32_t in0, uint32_t in1) {
    // TODO: Implement mux for u32
    return MUX_FUNCTION(ctrl, in0, in1);
}

int32_t mux_i32(bool ctrl, int32_t in0, int32_t in1) {
    // TODO: Implement mux for i32
    return MUX_FUNCTION(ctrl, in0, in1);
}

#endif  // End of Assignment 2, Question 1a

#ifndef ASSIGNMENT2_QUESTION1B

void decode(uint32_t in, struct instr* insn) {
    // Extract the fields directly based on their positions in the instruction word.
    insn->opcode = (in >> 26) & 0x3F;
    insn->rs = (in >> 21) & 0x1F;
    insn->rt = (in >> 16) & 0x1F;
    insn->rd = (in >> 11) & 0x1F;
    insn->shamt = (in >> 6) & 0x1F;
    insn->funct = in & 0x3F;
    insn->immed = in & 0xFFFF;
    insn->address = in & 0x03FFFFFF;
}

#endif  // End of Assignment 2, Question 1b

#ifndef ASSIGNMENT2_QUESTION2A

void Control(uint8_t opcode, bool* _RegDst, bool* _ALUSrc, bool* _MemtoReg,
             bool* _RegWrite, bool* _MemRead, bool* _MemWrite, bool* _Branch,
             uint8_t* _ALUOp) {
    // TODO: Implement Control
    *_RegDst = 0;
    *_ALUSrc = 1;
    *_MemRead = 0;
    *_MemWrite = 0;
    *_RegWrite = 0;
    *_Branch = 0;
    if (!opcode) {
        //R-format instruction
        *_RegDst = 1;
        *_ALUSrc = 0;
        *_MemtoReg = 0;
        *_RegWrite = 1;
        *_ALUOp = 2;
    }
    if (opcode == 4) {
        *_ALUSrc = 0;
    }
    if (opcode == 0x23) {
        //lw
        *_MemRead = 1;
        *_MemtoReg = 1;
        *_RegWrite = 1;
        *_ALUOp = 0;
    }
    if (opcode == 0x2b) {
        //sw
        *_MemWrite = 1;
        *_ALUOp = 0;
    }
    if (opcode == 0x4) {
        *_Branch = 1;
        *_ALUOp = 1;
    }
    if (opcode == 0x2) {
        *_Branch = 1;
    }
}

#endif  // End of Assignment 2, Question 2a

#ifndef ASSIGNMENT2_QUESTION2B

uint8_t ALUControl(uint8_t ALUOp, uint8_t funct) {
    // TODO: Implement ALUControl
    if (_ALUOp == 2) { // R-type instruction.
    // Need to decode funct code.
    switch (funct) {
        case 0x20: // add
        case 0x21: // addu - see Note 1.
            return 2; // ALU to perform Add
        case 0x22: // sub
        case 0x23: // subu - see Note 1.
            return 6; // ALU to perform Sub
        case 0x24:
            return 0; // ALU to perform And
        case 0x25:
            return 1; // ALU to perform Or
        case 0x27: // nor
            return 0xC; // Nor
        case 0x2a:
            return 7; // slt - see Note 2.
        default:
            error("invalid function value")
    }
    } else { // Non R-type
        if (_ALUOp == 0)
            return 2; // Ask ALU to add
        if (_ALUOp == 1)
            return 6; // Ask ALU to sub
    }
    return 0;
}

#endif  // End of Assignment 2, Question 2b

#ifndef ASSIGNMENT2_QUESTION2C

int32_t ALU(int32_t in0, int32_t in1, uint8_t ALUControl, bool* ALUiszero) {
    // TODO: Implement ALU
    int32_t result;
    switch (ALUControl) {
        case 0:
            result = in0 & in1;
            break;
        case 1:
            result = in0 | in1;
            break;
        case 2:
            result = in0 + in1;
            break;
        case 6:
            result = in0 - in1;
            break;
        case 7:
            result = (int32_t)(in0 < in1);
            break;
        case 12:
            result = ~(in0 | in1);
            break;
    }
    *ALUiszero = (result == 0);
    return(result);
}

#endif  // End of Assignment 2, Question 2c

#ifndef ASSIGNMENT2_QUESTION3

void execute(uint32_t insn) {
    struct instr instruction;
    decode(insn, &instruction); // Decode the instruction

    // Extract relevant fields from the instruction struct
    uint8_t opcode = instruction.opcode;
    uint8_t rs = instruction.rs;
    uint8_t rt = instruction.rt;
    uint8_t rd = instruction.rd;
    uint8_t shamt = instruction.shamt;
    uint8_t funct = instruction.funct;
    int32_t immed = instruction.immed;
    int32_t address = instruction.address;

    // Determine control signals for this instruction
    Control(opcode, &_RegDst, &_ALUSrc, &_MemtoReg, &_RegWrite, &_MemRead, &_MemWrite, &_Branch, &_ALUOp);

    /**
     * ALU stage
    */
   
    // Perform ALU operation based on the ALU control signal
    int32_t ALUResult;
    bool ALUisZero;
    int32_t secondOperand; // Define the second operand for ALU

    secondOperand = mux_i32(_ALUSrc, immed, _RF[rt]);
 
    ALUResult = ALU(_RF[rs], secondOperand, ALUControl(_ALUOp, funct), &ALUisZero);

    /**
     * Memory stage and Register Write stage 
     **/

    int32_t dataToWrite, dataFromMemory;
    dataFromMemory = Memory(ALUResult, _RF[(uint8_t)mux_u8(_RegDst, rt, rd)], _MemRead, _MemWrite);

    dataToWrite = mux_i32(_MemtoReg, dataFromMemory, ALUResult);

    if (_RegWrite) {
        RegFile(rs, (uint8_t)mux_u8(_RegDst, rt, rd), rd, dataToWrite, &_RF[rs], &_RF[rt], _RegWrite);
    }

    // Update the PC based on control signals
    if (_Branch && ALUisZero) {
        if (opcode == 0x4) {
            // BEQ instruction: PC = PC + 4 + (immed * 4)
            _PC = _PC + 4 + (immed << 2);
        }
    } else {
        // Default behavior: PC = PC + 4
        _PC += 4;
    }

}



#endif  // End of Assignment 2, Question 3
