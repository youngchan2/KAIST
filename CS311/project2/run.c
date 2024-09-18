/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction *get_inst_info(uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction()
{
    /* Implement your function here */
    instruction *instr;
    instr = get_inst_info(CURRENT_STATE.PC);
    CURRENT_STATE.PC += 4;

    switch (instr->opcode)
    {
    case 0x0: // R
        switch (instr->func_code)
        {
        case 0x20: // add
            CURRENT_STATE.REGS[RD(instr)] = CURRENT_STATE.REGS[RS(instr)] + CURRENT_STATE.REGS[RT(instr)];
            break;
        case 0x24: // and
            CURRENT_STATE.REGS[RD(instr)] = CURRENT_STATE.REGS[RS(instr)] & CURRENT_STATE.REGS[RT(instr)];
            break;
        case 0x8: // jr
            CURRENT_STATE.PC = CURRENT_STATE.REGS[RS(instr)];
            break;
        case 0x27: // nor
            CURRENT_STATE.REGS[RD(instr)] = ~(CURRENT_STATE.REGS[RS(instr)] | CURRENT_STATE.REGS[RT(instr)]);
            break;
        case 0x25: // or
            CURRENT_STATE.REGS[RD(instr)] = CURRENT_STATE.REGS[RS(instr)] | CURRENT_STATE.REGS[RT(instr)];
            break;
        case 0x2a: // slt
            CURRENT_STATE.REGS[RD(instr)] = (CURRENT_STATE.REGS[RS(instr)] < CURRENT_STATE.REGS[RT(instr)]) ? 1 : 0;
            break;
        case 0x0: // sll
            CURRENT_STATE.REGS[RD(instr)] = CURRENT_STATE.REGS[RT(instr)] << SHAMT(instr);
            break;
        case 0x2: // srl
            CURRENT_STATE.REGS[RD(instr)] = CURRENT_STATE.REGS[RT(instr)] >> SHAMT(instr);
            break;
        case 0x22: // sub
            CURRENT_STATE.REGS[RD(instr)] = CURRENT_STATE.REGS[RS(instr)] - CURRENT_STATE.REGS[RT(instr)];
            break;
        }
        break;
    case 0x8: // addi
        CURRENT_STATE.REGS[RT(instr)] = CURRENT_STATE.REGS[RS(instr)] + SIGN_EX(IMM(instr));
        break;
    case 0xc: // andi
        CURRENT_STATE.REGS[RT(instr)] = CURRENT_STATE.REGS[RS(instr)] & IMM(instr);
        break;
    case 0x4: // beq
        if (CURRENT_STATE.REGS[RS(instr)] == CURRENT_STATE.REGS[RT(instr)])
            CURRENT_STATE.PC += SIGN_EX(IMM(instr)) << 2;
        break;
    case 0x5: // bne
        if (CURRENT_STATE.REGS[RS(instr)] != CURRENT_STATE.REGS[RT(instr)])
            CURRENT_STATE.PC += SIGN_EX(IMM(instr)) << 2;
        break;
    case 0xf: // lui
        CURRENT_STATE.REGS[RT(instr)] = IMM(instr) << 16;
        break;
    case 0x23: // lw
        CURRENT_STATE.REGS[RT(instr)] = mem_read_32(CURRENT_STATE.REGS[RS(instr)] + SIGN_EX(IMM(instr)));
        break;
    case 0xd: // ori
        CURRENT_STATE.REGS[RT(instr)] = CURRENT_STATE.REGS[RS(instr)] | IMM(instr);
        break;
    case 0xa: // slti
        CURRENT_STATE.REGS[RT(instr)] = ((int)CURRENT_STATE.REGS[RS(instr)] < (int)SIGN_EX(IMM(instr))) ? 1 : 0;
        break;
    case 0x2b: // sw
        mem_write_32(CURRENT_STATE.REGS[RS(instr)] + SIGN_EX(IMM(instr)), CURRENT_STATE.REGS[RT(instr)]);
        break;
    case 0x2: // j
        CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (TARGET(instr) << 2);
        break;
    case 0x3: // jal
        CURRENT_STATE.REGS[31] = CURRENT_STATE.PC;
        CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (TARGET(instr) << 2);
        break;
    }

    if ((CURRENT_STATE.PC - MEM_TEXT_START) / 4 >= NUM_INST)
        RUN_BIT = FALSE;
}
