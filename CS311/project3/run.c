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
/* TODO: Implement 5-stage pipeplined MIPS simulator           */
/*                                                             */
/***************************************************************/

Control_unit control_init(instruction *inst)
{
	Control_unit control;

	short opcode = OPCODE(inst);
	if (opcode == 0x0 || opcode == 0x4 || opcode == 0x5)
		control.ex_ctrl.ALUSrc = 0;
	else
		control.ex_ctrl.ALUSrc = 1;

	control.ex_ctrl.ALUOp = opcode;

	if (opcode == 0x0)
		control.ex_ctrl.RegDst = 1;
	else
		control.ex_ctrl.RegDst = 0;

	if (opcode == 0x4 || opcode == 0x5) // beq,bne
		control.mem_ctrl.Branch = 1;
	else
		control.mem_ctrl.Branch = 0;

	if (opcode == 0x2b) // sw
		control.mem_ctrl.MemWrite = 1;
	else
		control.mem_ctrl.MemWrite = 0;

	if (opcode == 0x23) // lw
	{
		control.mem_ctrl.MemRead = 1;
		control.wb_ctrl.MemtoReg = 0;
	}
	else
	{
		control.mem_ctrl.MemRead = 0;
		control.wb_ctrl.MemtoReg = 1;
	}

	if ((opcode == 0x0 && FUNC(inst) == 0x8) || opcode == 0x2 || opcode == 0x3 || opcode == 0x4 || opcode == 0x5 || opcode == 0x2b) // jr,j,jal,beq,bne,sw
		control.wb_ctrl.RegWrite = 0;
	else
		control.wb_ctrl.RegWrite = 1;

	return control;
}

void Forward_Unit(uint32_t *forwardA, uint32_t *forwardB, uint32_t *forwardC, uint32_t *forwardD)
{
	//  ex forward unit
	if (CURRENT_STATE.EX_MEM_FORWARD_REG != 0)
	{
		if (CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_RS)
			*forwardA = 1;
		if (CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_RT)
			*forwardB = 1;
	}

	// mem forward unit
	if (CURRENT_STATE.MEM_WB_FORWARD_REG != 0)
	{
		if (CURRENT_STATE.EX_MEM_FORWARD_REG != CURRENT_STATE.ID_EX_RS)
		{
			if (CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_RS)
				*forwardC = 1;
		}
		if (CURRENT_STATE.EX_MEM_FORWARD_REG != CURRENT_STATE.ID_EX_RT)
		{
			if (CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_RT)
				*forwardD = 1;
		}
	}
}

uint32_t ALU(int alu1, int alu2, unsigned char op, unsigned char func, unsigned char sh)
{
	switch (op)
	{
	case 0x8: // addi
		return (alu1 + alu2);
	case 0xc: // andi
		return (alu1 & alu2);
	case 0xf: // lui
		return ((alu2 << 16) & 0xffff0000);
	case 0xd: // ori
		return (alu1 | (alu2 & 0xffff));
	case 0xa: // slti
		return (alu1 < alu2);
	case 0x23: // lw
		return (alu1 + alu2);
	case 0x2b: // sw
		return (alu1 + alu2);
	case 0x4: // beq
		return (alu1 == alu2);
	case 0x5: // bne
		return (alu1 != alu2);
	case 0x0:
		switch (func)
		{
		case 0x20: // add
			return (alu1 + alu2);
		case 0x24: // and
			return (alu1 & alu2);
		case 0x27: // nor
			return ~(alu1 | alu2);
		case 0x25: // or
			return (alu1 | alu2);
		case 0x2a: // slt
			return (alu1 < alu2);
		case 0x0: // sll
			return (alu2 << sh);
		case 0x2: // srl
			return (alu2 >> sh) & 0x7fffffff;
		case 0x22: // sub
			return (alu1 - alu2);
			break;
		default:
			printf("Unknown function code type: %d\n", func);
			break;
		}
	default:
		printf("Unknown instruction type: %d\n", op);
		break;
	}
}

void Hazard_Detection_Unit(instruction *inst)
{
	if (CURRENT_STATE.EX_MEM_MEM.MemRead && ((CURRENT_STATE.EX_MEM_DEST == RS(inst)) || CURRENT_STATE.EX_MEM_DEST == RT(inst)))
		CURRENT_STATE.HAZARD = 1;
}

void Decoder(instruction *inst)
{
	CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[RS(inst)];
	CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[RT(inst)];
	CURRENT_STATE.ID_EX_RS = RS(inst);
	CURRENT_STATE.ID_EX_RT = RT(inst);

	switch (OPCODE(inst))
	{
	case 0x0: // R
		CURRENT_STATE.ID_EX_SH = SHAMT(inst);
		CURRENT_STATE.ID_EX_FUNC = FUNC(inst);
		switch (FUNC(inst))
		{
		case 0x8: // jr
			CURRENT_STATE.JUMP_PC = CURRENT_STATE.REGS[RS(inst)];
			CURRENT_STATE.PCSrc = JUMP;
			break;
		default:
			CURRENT_STATE.ID_EX_DEST = RD(inst);
			break;
		}
		break;
	case 0x2: // j
		CURRENT_STATE.ID_EX_IMM = SIGN_EX(IMM(inst));
		CURRENT_STATE.JUMP_PC = ((CURRENT_STATE.PIPE[ID_STAGE] & 0xf0000000) | TARGET(inst) << 2);
		CURRENT_STATE.PCSrc = JUMP;
		break;
	case 0x3: // jal
		CURRENT_STATE.REGS[31] = CURRENT_STATE.PIPE[ID_STAGE] + 4;
		CURRENT_STATE.ID_EX_IMM = SIGN_EX(IMM(inst));
		CURRENT_STATE.JUMP_PC = ((CURRENT_STATE.PIPE[ID_STAGE] & 0xf0000000) | TARGET(inst) << 2);
		CURRENT_STATE.PCSrc = JUMP;
		break;
	case 0x4:
	case 0x5:
		CURRENT_STATE.ID_EX_IMM = SIGN_EX(IMM(inst));
		CURRENT_STATE.ID_EX_DEST = RT(inst);
		if (CURRENT_STATE.PREDICTION)
		{
			CURRENT_STATE.BRANCH_PC = CURRENT_STATE.PIPE[ID_STAGE] + (CURRENT_STATE.ID_EX_IMM << 2) + 4;
			CURRENT_STATE.PCSrc = BRANCH;
		}
	default: // I
		CURRENT_STATE.ID_EX_IMM = SIGN_EX(IMM(inst));
		CURRENT_STATE.ID_EX_DEST = RT(inst);
		break;
	}
}

void pipeline_init()
{
	for (int i = 3; i >= 0; i--)
	{
		if (CURRENT_STATE.HAZARD && i == 1)
		{
			CURRENT_STATE.PIPE[EX_STAGE] = 0;
			break;
		}
		else
		{
			CURRENT_STATE.PIPE[i + 1] = CURRENT_STATE.PIPE[i];
		}
	}

	int start = 0;
	for (int i = 0; i < 5; i++)
	{
		if (CURRENT_STATE.PIPE[i] == 0)
			start++;
	}

	if (start == 5)
		CURRENT_STATE.PREDICTION = NOT_TAKE;

	CURRENT_STATE.PREDICT_SUCCESS = 1;
	CURRENT_STATE.HAZARD = 0;
	CURRENT_STATE.PCSrc = IF;

	CURRENT_STATE.EX_MEM_FORWARD_REG = CURRENT_STATE.EX_MEM_DEST;
	CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.EX_MEM_ALU_OUT;
	CURRENT_STATE.MEM_WB_FORWARD_REG = CURRENT_STATE.MEM_WB_DEST;
	CURRENT_STATE.MEM_WB_FORWARD_VALUE = (CURRENT_STATE.MEM_WB_WB.MemtoReg == 1) ? CURRENT_STATE.MEM_WB_ALU_OUT : CURRENT_STATE.MEM_WB_MEM_OUT;
}

void IF_Stage()
{
	if (CURRENT_STATE.PC - MEM_TEXT_START >= NUM_INST * 4)
		CURRENT_STATE.PIPE[IF_STAGE] = 0;
	else if (CURRENT_STATE.HAZARD)
		CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
	else if (CURRENT_STATE.PREDICT_SUCCESS)
	{
		CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
		CURRENT_STATE.IF_PC = CURRENT_STATE.PC + 4;
	}

	switch (CURRENT_STATE.PCSrc)
	{
	case JUMP:
		CURRENT_STATE.PC = CURRENT_STATE.JUMP_PC;
		break;
	case BRANCH:
		CURRENT_STATE.PC = CURRENT_STATE.BRANCH_PC;
		break;
	default:
		CURRENT_STATE.PC = CURRENT_STATE.IF_PC;
		break;
	}
}

void ID_Stage()
{
	if (!CURRENT_STATE.PIPE[ID_STAGE])
	{
		CURRENT_STATE.ID_EX_REG1 = 0;
		CURRENT_STATE.ID_EX_REG2 = 0;
		CURRENT_STATE.ID_EX_IMM = 0;
		CURRENT_STATE.ID_EX_RS = 0;
		CURRENT_STATE.ID_EX_RT = 0;
		CURRENT_STATE.ID_EX_DEST = 0;

		CURRENT_STATE.ID_EX_EX.ALUOp = 0;
		CURRENT_STATE.ID_EX_EX.ALUSrc = 0;
		CURRENT_STATE.ID_EX_EX.RegDst = 0;

		CURRENT_STATE.ID_EX_MEM.Branch = 0;
		CURRENT_STATE.ID_EX_MEM.MemRead = 0;
		CURRENT_STATE.ID_EX_MEM.MemWrite = 0;

		CURRENT_STATE.ID_EX_WB.MemtoReg = 0;
		CURRENT_STATE.ID_EX_WB.RegWrite = 0;
		return;
	}

	instruction *inst = get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]);
	Control_unit control = control_init(inst);

	Hazard_Detection_Unit(inst);
	Decoder(inst);

	CURRENT_STATE.ID_EX_WB = control.wb_ctrl;
	CURRENT_STATE.ID_EX_MEM = control.mem_ctrl;
	CURRENT_STATE.ID_EX_EX = control.ex_ctrl;
}

void EX_Stage()
{
	if (!CURRENT_STATE.PIPE[EX_STAGE])
	{
		CURRENT_STATE.EX_MEM_ALU_OUT = 0;
		CURRENT_STATE.EX_MEM_BR_TARGET = 0;
		CURRENT_STATE.EX_MEM_BR_TAKE = 0;
		CURRENT_STATE.EX_MEM_DEST = 0;

		CURRENT_STATE.EX_MEM_MEM.Branch = 0;
		CURRENT_STATE.EX_MEM_MEM.MemRead = 0;
		CURRENT_STATE.EX_MEM_MEM.MemWrite = 0;

		CURRENT_STATE.EX_MEM_WB.MemtoReg = 0;
		CURRENT_STATE.EX_MEM_WB.RegWrite = 0;
		return;
	}

	int alu1;
	int alu2;
	uint32_t alu_out;
	uint32_t forwardA = 0;
	uint32_t forwardB = 0;
	uint32_t forwardC = 0;
	uint32_t forwardD = 0;
	unsigned char aluop = CURRENT_STATE.ID_EX_EX.ALUOp;
	unsigned char func = CURRENT_STATE.ID_EX_FUNC;
	unsigned char sh = CURRENT_STATE.ID_EX_SH;

	Forward_Unit(&forwardA, &forwardB, &forwardC, &forwardD);

	alu1 = forwardA ? CURRENT_STATE.EX_MEM_FORWARD_VALUE : (forwardC ? CURRENT_STATE.MEM_WB_FORWARD_VALUE : CURRENT_STATE.ID_EX_REG1);
	alu2 = forwardB ? CURRENT_STATE.EX_MEM_FORWARD_VALUE : (forwardD ? CURRENT_STATE.MEM_WB_FORWARD_VALUE : CURRENT_STATE.ID_EX_REG2);

	if (CURRENT_STATE.ID_EX_EX.ALUSrc)
		alu2 = CURRENT_STATE.ID_EX_IMM;

	if (aluop == 2 || aluop == 3 || (aluop == 0 && func == 8)) // jump one stall
		CURRENT_STATE.PIPE[ID_STAGE] = 0;
	else
		CURRENT_STATE.EX_MEM_ALU_OUT = ALU(alu1, alu2, aluop, func, sh);

	if (aluop == 4 || aluop == 5) // beq, bne
	{
		CURRENT_STATE.EX_MEM_MEM.Branch = 1;
		CURRENT_STATE.EX_MEM_BR_TARGET = CURRENT_STATE.PIPE[EX_STAGE] + (CURRENT_STATE.ID_EX_IMM << 2) + 4;
		CURRENT_STATE.EX_MEM_BR_TAKE = CURRENT_STATE.EX_MEM_ALU_OUT;

		if (CURRENT_STATE.PREDICTION)
		{
			CURRENT_STATE.PIPE[ID_STAGE] = 0;
			CURRENT_STATE.PC = CURRENT_STATE.EX_MEM_BR_TARGET;
		}
	}

	CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_DEST;
	CURRENT_STATE.EX_MEM_WB = CURRENT_STATE.ID_EX_WB;
	CURRENT_STATE.EX_MEM_MEM = CURRENT_STATE.ID_EX_MEM;
}

void MEM_Stage()
{
	if (!CURRENT_STATE.PIPE[MEM_STAGE])
	{
		CURRENT_STATE.MEM_WB_ALU_OUT = 0;
		CURRENT_STATE.MEM_WB_MEM_OUT = 0;
		CURRENT_STATE.MEM_WB_DEST = 0;

		CURRENT_STATE.MEM_WB_WB.MemtoReg = 0;
		CURRENT_STATE.MEM_WB_WB.RegWrite = 0;
		return;
	}

	if (CURRENT_STATE.EX_MEM_MEM.Branch && (CURRENT_STATE.EX_MEM_BR_TAKE != CURRENT_STATE.PREDICTION))
	{
		CURRENT_STATE.PREDICT_SUCCESS = 0;
		CURRENT_STATE.PREDICTION = !CURRENT_STATE.PREDICTION;
		if (CURRENT_STATE.EX_MEM_BR_TAKE)
		{
			CURRENT_STATE.PCSrc = BRANCH;
			CURRENT_STATE.BRANCH_PC = CURRENT_STATE.EX_MEM_BR_TARGET;
		}
		else
		{
			CURRENT_STATE.PCSrc = IF;
			CURRENT_STATE.IF_PC = CURRENT_STATE.PIPE[MEM_STAGE] + 4;
		}
		CURRENT_STATE.PIPE[IF_STAGE] = 0;
		CURRENT_STATE.PIPE[ID_STAGE] = 0;
		CURRENT_STATE.PIPE[EX_STAGE] = 0;
	}

	if (CURRENT_STATE.EX_MEM_MEM.MemRead) // lw
		CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);
	else
		CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;

	if (CURRENT_STATE.EX_MEM_MEM.MemWrite) // sw
		mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.REGS[CURRENT_STATE.EX_MEM_DEST]);

	CURRENT_STATE.MEM_WB_WB = CURRENT_STATE.EX_MEM_WB;
	CURRENT_STATE.MEM_WB_DEST = CURRENT_STATE.EX_MEM_DEST;
}

void WB_Stage()
{
	if (!CURRENT_STATE.PIPE[WB_STAGE])
		return;

	if (CURRENT_STATE.MEM_WB_WB.RegWrite)
	{
		if (CURRENT_STATE.MEM_WB_WB.MemtoReg)
			CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_ALU_OUT;
		else
			CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_MEM_OUT;
	}

	INSTRUCTION_COUNT++;
}

void process_instruction()
{
	pipeline_init();
	WB_Stage();
	MEM_Stage();
	EX_Stage();
	ID_Stage();
	IF_Stage();

	// Zero reg is hard-wired to zero!
	CURRENT_STATE.REGS[0] = 0;
	if (CURRENT_STATE.PIPE[WB_STAGE] >= MEM_REGIONS[0].start + (NUM_INST * 4))
		RUN_BIT = FALSE;
	if (!CURRENT_STATE.PIPE[IF_STAGE] && !CURRENT_STATE.PIPE[ID_STAGE] && !CURRENT_STATE.PIPE[EX_STAGE] && !CURRENT_STATE.PIPE[MEM_STAGE] && CURRENT_STATE.PIPE[WB_STAGE])
		RUN_BIT = FALSE;
}
