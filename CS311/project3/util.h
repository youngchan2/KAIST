/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   util.h                                                    */
/*                                                             */
/***************************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FALSE 0
#define TRUE 1

/* Basic Information */
#define MEM_TEXT_START 0x00400000
#define MEM_TEXT_SIZE 0x00100000
#define MEM_DATA_START 0x10000000
#define MEM_DATA_SIZE 0x00100000
#define MIPS_REGS 32
#define BYTES_PER_WORD 4
#define PIPE_STAGE 5

#define IF_STAGE 0
#define ID_STAGE 1
#define EX_STAGE 2
#define MEM_STAGE 3
#define WB_STAGE 4

#define IF 0
#define JUMP 1
#define BRANCH 2

#define NOT_TAKE 0
#define TAKE 1

typedef struct inst_s
{
	short opcode;

	/*R-type*/
	short func_code;

	union
	{
		/* R-type or I-type: */
		struct
		{
			unsigned char rs;
			unsigned char rt;

			union
			{
				short imm;

				struct
				{
					unsigned char rd;
					unsigned char shamt;
				} r;
			} r_i;
		} r_i;
		/* J-type: */
		uint32_t target;
	} r_t;

	uint32_t value;

	// int32 encoding;
	// imm_expr *expr;
	// char *source_line;
} instruction;

/* You may add pipeline registers that you require */
typedef struct ex_control
{
	unsigned char ALUSrc;
	unsigned char ALUOp;
	unsigned char RegDst;
} EX_control;

typedef struct mem_control
{
	unsigned char Branch;
	unsigned char MemWrite;
	unsigned char MemRead;
} MEM_control;

typedef struct wb_control
{
	unsigned char MemtoReg;
	unsigned char RegWrite;
} WB_control;

typedef struct control_unit
{
	EX_control ex_ctrl;
	MEM_control mem_ctrl;
	WB_control wb_ctrl;
} Control_unit;

typedef struct CPU_State_Struct
{
	uint32_t PC;				   /* program counter for the IF stage*/
	uint32_t REGS[MIPS_REGS];	   /* register file */
	uint32_t REGS_LOCK[MIPS_REGS]; /* register lock to support stalls
					  Lock registers when data is not ready*/

	uint32_t PIPE[PIPE_STAGE]; /* PC being executed at each stage*/
	uint32_t PIPE_STALL[PIPE_STAGE];

	// IF_ID_latch
	instruction *IF_ID_INST;
	uint32_t IF_ID_NPC;

	// ID_EX_latch
	uint32_t ID_EX_NPC;
	uint32_t ID_EX_REG1;
	uint32_t ID_EX_REG2;
	short ID_EX_IMM;
	uint32_t ID_EX_SH;
	uint32_t ID_EX_FUNC;
	unsigned char ID_EX_DEST;
	EX_control ID_EX_EX;
	MEM_control ID_EX_MEM;
	WB_control ID_EX_WB;
	unsigned char ID_EX_RS;
	unsigned char ID_EX_RT;

	// EX_MEM_latch
	uint32_t EX_MEM_NPC;
	uint32_t EX_MEM_ALU_OUT;
	uint32_t EX_MEM_W_VALUE;
	uint32_t EX_MEM_BR_TARGET;
	uint32_t EX_MEM_BR_TAKE;
	unsigned char EX_MEM_DEST;
	MEM_control EX_MEM_MEM;
	WB_control EX_MEM_WB;

	// MEM_WB_latch
	uint32_t MEM_WB_NPC;
	uint32_t MEM_WB_ALU_OUT;
	uint32_t MEM_WB_MEM_OUT;
	uint32_t MEM_WB_BR_TAKE;
	unsigned char MEM_WB_DEST;
	WB_control MEM_WB_WB;

	// Forwarding
	unsigned char EX_MEM_FORWARD_REG;
	unsigned char MEM_WB_FORWARD_REG;
	uint32_t EX_MEM_FORWARD_VALUE;
	uint32_t MEM_WB_FORWARD_VALUE;

	// To choose right PC
	uint32_t IF_PC;
	uint32_t JUMP_PC;
	uint32_t BRANCH_PC;
	unsigned char PCSrc;
	unsigned char PREDICTION;
	unsigned char PREDICT_SUCCESS;
	unsigned char HAZARD;
} CPU_State;

typedef struct
{
	uint32_t start, size;
	uint8_t *mem;
} mem_region_t;

/* For PC * Registers */
extern CPU_State CURRENT_STATE;

/* For Instructions */
extern instruction *INST_INFO;
extern int NUM_INST;

/* For Memory Regions */
extern mem_region_t MEM_REGIONS[2];

/* For Execution */
extern int RUN_BIT;	  /* run bit */
extern int FETCH_BIT; /* instruction fetch bit */
extern int INSTRUCTION_COUNT;

extern int BR_BIT; /* Branch predictor enabled */
extern int FORWARDING_BIT;
extern uint64_t MAX_INSTRUCTION_NUM;
extern uint64_t CYCLE_COUNT;

/* Functions */
char **str_split(char *a_str, const char a_delim);
int fromBinary(const char *s);
uint32_t mem_read_32(uint32_t address);
void mem_write_32(uint32_t address, uint32_t value);
void cycle();
void run();
void go();
void mdump(int start, int stop);
void rdump();
void pdump();
void init_memory();
void init_inst_info();

/* YOU IMPLEMENT THIS FUNCTION in the run.c file */
void process_instruction();

#endif
