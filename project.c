#include "spimcore.h"
#include <stdlib.h>
#include <stdint.h>

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    // A and B may be represented as 2's complement, which would need to be addressed

    switch (ALUControl)
    {
    case 0: // Z = A + B
        *ALUresult = A + B;
        break;
    case 1: // Z = A - B
        *ALUresult = A-B;
        *Zero = *ALUresult == 0;
        break;
    case 2: // Z = 1 if A < B, 0 otherwise

        *ALUresult=((signed)A < (signed)B);
        break;
    case 3: // Z = 1 if A < B, 0 otherwise (A and B are unsigned integers)
        *ALUresult = A < B;
        break;
    case 4: // Z = A and B
        *ALUresult = A & B;
        break;
    case 5: // Z = A or B
        *ALUresult = A | B;
        break;
    case 6: // shift B left by 16 bits
        *ALUresult = B << 16;
        break;
    default: // Z = not A
        *ALUresult = ~A;
        *Zero=!*ALUresult;
        break;
    }
}

/* instruction fetch */
/* 10 Points */
// fetch instructed addressed by PC from Mem and write it to instruction
// return 1 if a halt condition occurs, otherwise 0
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{

    // PC is malaligned if it is not a multiple of 4
    if(PC%4 != 0)
    {
        return 1;
    }

    // add 4
    PC = PC >> 2;

    *instruction = Mem[PC];

    return 0;
}


/* instruction partition */
/* 10 Points */
// not tested
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
 *op = (instruction & 0xfc000000) >> 26;
    *r1 = (instruction & 0x03e00000) >> 21;
    *r2 = (instruction & 0x001f0000) >> 16;
    *r3 = (instruction & 0x0000f800) >> 11;

    *funct = instruction & 0x0000003f;
    *offset = instruction & 0x0000ffff;
    *jsec = instruction &0x03ffffff;
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
// 0: disabled 1: enabled OR 0/1 reprents path for multiplexor
    // 2: don't care

    switch (op)
    {
    case 0://r-type
        controls->ALUOp = 7;
        controls->ALUSrc = 0;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->MemWrite = 0;
        controls->RegDst = 1;
        controls->RegWrite = 1;
        break;
    case 0x23://loadword
        controls->ALUOp = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemRead = 1;
        controls->MemtoReg = 1;
        controls->MemWrite = 0;
        controls->RegDst = 0;
        controls->RegWrite = 1;
        break;
    case 0x2b: //sw
        controls->ALUOp = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 2;
        controls->MemWrite = 1;
        controls->RegDst = 2;
        controls->RegWrite = 0;
        break;
    case 0x4://beq
        controls->ALUOp = 1;
        controls->ALUSrc = 0;
        controls->Branch = 1;
        controls->Jump = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 2;
        controls->MemWrite = 0;
        controls->RegDst = 2;
        controls->RegWrite = 0;
        break;
    case 0x8: // addi

        controls->ALUOp = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->MemWrite = 0;
        controls->RegDst = 0;
        controls->RegWrite = 1;
        break;
    case 0xF:  //lui
        controls->ALUOp = 6;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->MemWrite = 0;
        controls->RegDst = 0;
        controls->RegWrite = 1;
        break;
    case 0xA: // slti
		controls->ALUOp = 6;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->MemWrite = 0;
        controls->RegDst = 0;
        controls->RegWrite = 1;
		break;
    case 0xB: //sltiu
        controls->ALUOp = 6;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->MemWrite = 0;
        controls->RegDst = 0;
        controls->RegWrite = 1;
        break;

    case 0x2: // jump
        controls->ALUOp = 0;
        controls->ALUSrc = 2;
        controls->Branch = 0;
        controls->Jump = 1;
        controls->MemRead = 0;
        controls->MemtoReg = 2;
        controls->MemWrite = 0;
        controls->RegDst = 2;
        controls->RegWrite = 0;
        break;
    default:
        return 1;
        break;
    }
    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    int16_t value = (int16_t)offset;
    *extended_value = (int32_t)value;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    char ALUControl = 0;
    if (ALUSrc==1)
    {
        data2=extended_value;
    }
    if (ALUOp==7)
    {
        switch (funct)
        {
        case 0x20: // 0b100000, add
            ALUControl = 0;

            break;
        case 0x22: // 0b100010, subtract
            ALUControl = 1;
            break;
        case 0x24: // 0b100100, AND
            ALUControl = 4;
            break;
        case 0x25: // 0b100101, OR
            ALUControl = 5;
            break;
        case 0x2a: // 0b101010, slt
            ALUControl = 2;
            break;
        case 0x2b:
            ALUControl = 3;
            break;
        default: // bad instruction
            return 1; // halt
            break;
        }
        ALU(data1, data2, ALUControl, ALUresult, Zero);

    }
    else
    {
        ALUControl = ALUOp;
        ALU(data1, data2, ALUControl, ALUresult, Zero);

    }
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
	if (MemRead == 1 && MemWrite == 1) // cannot read and write at the same time
		return 1;
	
    if (MemRead == 1)
    {
        if (ALUresult %4 != 0) // check for maligned word {
            return 1;
        *memdata = Mem[ALUresult >> 2];
    }
    else if (MemWrite == 1)
    {
        if (ALUresult %4 != 0) // check for maligned word {
            return 1;
        Mem[ALUresult >> 2] = data2;

    }

    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite == 0)
        return; // do not write

    unsigned data = MemtoReg == 0 ? ALUresult : memdata;

    if (RegDst == 1)
        Reg[r3] = data; // r-type
    else

        Reg[r2] = data; // i-type


}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    if (Jump == 1)
    {
        unsigned leadingFourBits = *PC & 0xf000000;
        *PC = (jsec << 2) | leadingFourBits;
        Zero = 0;
    }
    else if (Branch == 1 && Zero == 1)   // Branch
    {
        *PC = *PC + (extended_value << 2) + 4;
        Zero = 1;
    }
    else   // update PC for next instruction
    {
        *PC += 4;
        Zero = 0;
    }
}
a