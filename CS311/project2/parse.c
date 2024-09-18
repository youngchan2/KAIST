/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;

instruction parsing_instr(const char *buffer, const int index)
{
	instruction instr;
	/* Implement your function here */
	instr = INST_INFO[index];
	int binary = fromBinary((char *)buffer);
	mem_write_32(MEM_TEXT_START + index, binary);
	instr.opcode = (binary >> 26) & 0x3f;

	switch (instr.opcode)
	{
	// R
	case 0x0:
		instr.r_t.r_i.rs = (binary >> 21) & 0x1f;
		instr.r_t.r_i.rt = (binary >> 16) & 0x1f;
		instr.r_t.r_i.r_i.r.rd = (binary >> 11) & 0x1f;
		instr.r_t.r_i.r_i.r.shamt = (binary >> 6) & 0x1f;
		instr.func_code = binary & 0x3f;
		break;
	// I
	case 0x8:
	case 0xc:
	case 0x4:
	case 0x5:
	case 0xf:
	case 0x23:
	case 0xd:
	case 0xa:
	case 0x2b:
		instr.r_t.r_i.rs = (binary >> 21) & 0x1f;
		instr.r_t.r_i.rt = (binary >> 16) & 0x1f;
		instr.r_t.r_i.r_i.imm = binary & 0xffff;
		break;
	// J
	case 0x2:
	case 0x3:
		instr.r_t.target = binary & 0x3FFFFFF;
		break;
	}
	return instr;
}

void parsing_data(const char *buffer, const int index)
{
	/* Implement your function here */
	mem_write_32(MEM_DATA_START + index, fromBinary((char *)buffer));
}

void print_parse_result()
{
	int i;
	printf("Instruction Information\n");

	for (i = 0; i < text_size / 4; i++)
	{
		printf("INST_INFO[%d].value : %x\n", i, INST_INFO[i].value);
		printf("INST_INFO[%d].opcode : %d\n", i, INST_INFO[i].opcode);

		switch (INST_INFO[i].opcode)
		{
		// Type I
		case 0x8:  // ADDI
		case 0xc:  // ANDI
		case 0xf:  // LUI
		case 0xd:  // ORI
		case 0xa:  // SLTI
		case 0x23: // LW
		case 0x2b: // SW
		case 0x4:  // BEQ
		case 0x5:  // BNE
			printf("INST_INFO[%d].rs : %d\n", i, INST_INFO[i].r_t.r_i.rs);
			printf("INST_INFO[%d].rt : %d\n", i, INST_INFO[i].r_t.r_i.rt);
			printf("INST_INFO[%d].imm : %d\n", i, INST_INFO[i].r_t.r_i.r_i.imm);
			break;

			// TYPE R
		case 0x0: // ADD, AND, NOR, OR, SLT, SLL, SRL, SUB
			printf("INST_INFO[%d].func_code : %d\n", i, INST_INFO[i].func_code);
			printf("INST_INFO[%d].rs : %d\n", i, INST_INFO[i].r_t.r_i.rs);
			printf("INST_INFO[%d].rt : %d\n", i, INST_INFO[i].r_t.r_i.rt);
			printf("INST_INFO[%d].rd : %d\n", i, INST_INFO[i].r_t.r_i.r_i.r.rd);
			printf("INST_INFO[%d].shamt : %d\n", i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
			break;

			// TYPE J
		case 0x2: // J
		case 0x3: // JAL
			printf("INST_INFO[%d].target : %d\n", i, INST_INFO[i].r_t.target);
			break;

		default:
			printf("Not available instruction\n");
			assert(0);
		}
	}

	printf("Memory Dump - Text Segment\n");
	for (i = 0; i < text_size; i += 4)
		printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
	for (i = 0; i < data_size; i += 4)
		printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
	printf("Current PC: %x\n", CURRENT_STATE.PC);
}
