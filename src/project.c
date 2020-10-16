
#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
  // change ALUControls data type to int from char //
  unsigned Z = 0;

  switch (ALUControl) {
    case 0: Z = A + B; // add
    break;

    case 1: Z = A - B; // subtract
    break;

    case 2: if (A < B) { // set less than
      Z = 1;
    }
    else {
      Z = 0;
    }
    break;

    case 3: if (A < B) { // set less than unsigned
      Z = 1;
    }
    else {
      Z = 0;
    }
    break;

    case 4: if (A == 1 && B == 1) { // and
      Z = 1;
    }
    else {
      Z = 0;
    }
    break;

    case 5: if (A == 0 && B == 0) { // or
      Z = 0;
    }
    else {
      Z = 1;
    }
    break;

    case 6: Z = B << 16; // shift left 16
    break;

    case 7: Z = ~A; // not
    break;

    default: return;
  }

  *ALUresult = Z;

  // Determine if the result is 0.
  if (Z == 0) {
    *Zero = 1;
  }
  else {
    *Zero = 0;
  }

}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
  if (PC % 4 == 0) //To make sure the word is aligned in the Memory//
    *instruction = Mem[PC >> 2]; //Fetch instruction
  else
    return 1; //if it is not aligned then it will return 1 and I assume that is what halt means I'm not sure
    // Richy, I think you are correct here. - Phil

  return 0; //returns 0 if the instruction was fetched
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
  *op = (instruction & 0xfc000000) >> 26; // need the leftmost 6 bits.
	*r1 = (instruction & 0x03e00000) >> 21; // Need the next 5 bits.
	*r2 = (instruction & 0x001f0000) >> 16;  // Next 5 bits.
	*r3 = (instruction & 0x0000f800) >> 11; // Next 5 bits.
	*funct = instruction & 0x0000003f; // We need the rightmost 6 bits. All others should be set to 0.
	*offset = instruction & 0x0000ffff; // We need the rightmost 16 bits. All others should be set to 0.
	*jsec = instruction & 0x03ffffff; // This will give us only the the rightmost 26 bits.
}



/* instruction decode */
/* 15 Points */
//kirk's
//return 1 if problem (halt)
int instruction_decode(unsigned op,struct_controls *controls)
{
  switch (op)
  {
    case 0: // R-Type
      *controls = (struct_controls) {1, 0, 0, 0, 0, 7, 0, 0, 1};
      break;
    case 2: //jump
      *controls = (struct_controls) {2, 1, 2, 0, 2, 0, 0, 2, 0};
      break;
    case 11: //sltiu
      *controls = (struct_controls) {0, 0, 0, 0, 0, 3, 0, 1, 1};
      break;
    case 10: //slti
      *controls = (struct_controls) {0, 0, 0, 0, 0, 2, 0, 1, 1};
      break;
    case 35: //lw
      *controls = (struct_controls) {0, 0, 0, 1, 1, 0, 0, 1, 1};
      break;
    case 43: //sw
      *controls = (struct_controls) {0, 0, 0, 0, 0, 0, 1, 1, 0};
      break;
    case 4: //beq
      *controls = (struct_controls) {2, 0, 1, 0, 2, 1, 0, 2, 0};
      break;
    case 8: // Addi
      *controls = (struct_controls) {0, 0, 0, 0, 0, 0, 0, 1, 1};
      break;
    case 15: // lui
      *controls = (struct_controls) {0, 0, 0, 0, 0, 6, 0, 1, 1};
      break;
    default:
      return 1;
  }
  return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
	*data1 = Reg[r1];	//Extract content from register in address r1
	*data2 = Reg[r2];	//Extract content from register in address r2
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
  // using ints in place of unsigned currently
  if(((offset >> 15)) == 1) //checks if 16th bit is one and changed the extended value to a negative number
    *extended_value = offset | 0xffff0000; // Sets the upper 16 bits on.
  else //if not then the number stays put because it is a positive number
    *extended_value = offset & 0x0000ffff; // Sets the upper 16 bits off.
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
  // I-type instructions
  if (ALUOp >=0 && ALUOp < 7)
  {
    // ALUSrc of 1 indicates we need to use the extended value, rather than data2.
    if (ALUSrc == 1)
      {
        ALU(data1, extended_value, ALUOp, ALUresult, Zero);
        return 0;
      }

    else
    {
      ALU(data1, data2, ALUOp, ALUresult, Zero);
      return 0;
    }

    return 0;
  }

  // R-type instructions
  else if (ALUOp == 7)
  {
    if (funct == 32) // add
      ALUOp = 0;

    else if  (funct == 36) // and
      ALUOp = 4;

    else if  (funct == 37) // or
      ALUOp = 5;

    else if  (funct == 42) // slt
      ALUOp = 2;

    else if  (funct == 43) // sltu
      ALUOp = 3;

    else
      return 1; // halt condition reached. Invalid funct code.

    ALU(data1, data2, ALUOp, ALUresult, Zero);

    return 0;
  }

return 1; // Invalid ALUOp code sent.
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
  if (MemRead == 1) //if read is asserted than we have to set the memory data to the memory of ALUresult but shifted
    {
      if (ALUresult % 4 != 0)
        return 1; // halt condition met.

      *memdata = Mem[ALUresult >> 2];
        return 0;
    }
  else if (MemWrite == 1) //if write is asserted than it sets the memory of ALUresult to data2
    {
      if (ALUresult % 4 != 0)
        return 1; // halt condition met.

      Mem[ALUresult >> 2] = data2;
        return 0;
    }

  else
    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    //if MemtoReg is deasserted: value in the register write data comes from ALU
    //if MemtoReg is asserted: value in the register write data comes from data memory

    if(RegWrite == 1) { //if RegWrite is asserted than the register destination specified by the write register input is written with the value from the write data input
      if (MemtoReg == 1) { //if MemtoReg is asserted: value in the register write data comes from data memory
        if (RegDst == 1) //if MemtoReg is deasserted: value in the register write data comes from ALU
          Reg[r3] = memdata; //if RegDst is asserted than the register destination number for the write register comes from the [rd field == r3 from spimcore.c]
        else
          Reg[r2] = memdata; //if RegDst is deasserted the register destination number to write register comes from the rt field [rt field == r2 from spimcore.c]
      }
      else { //if MemtoReg is deasserted: value in the register write data comes from ALU
        if (RegDst == 1) //if RegDst is asserted than the register destination number for the write register comes from the [rd field == r3 from spimcore.c]
          Reg[r3] = ALUresult;
        else //if RegDst is deasserted the register destination number to write register comes from the rt field [rt field == r2 from spimcore.c]
          Reg[r2] = ALUresult;
      }

    }

}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
  // increment PC by 4 after each completed instruction
  *PC = *PC + 4;

  if(Jump == 1)
    {
      *PC = (jsec << 2) | (*PC & 0xf0000000);
    }

  // branch and zero are 1, PC is equal to the sign extended
  // value shifted to the left 2 bits
  else if (Branch == 1 && Zero == 1)
    {
      *PC = *PC + (extended_value << 2);
    }

  else
    return;
}
