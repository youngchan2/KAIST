# Project 3. MIPS Pipelined Simulator
Skeleton developed by CMU,
modified for KAIST CS311 purpose by THKIM, BKKIM and SHJEON.

## Instructions
There are three files you may modify: `util.h`, `run.h`, and `run.c`.

### 1. util.h

We have setup the basic CPU\_State that is sufficient to implement the project.
However, you may decide to add more variables, and modify/remove any misleading variables.

### 2. run.h

You may add any additional functions that will be called by your implementation of `process_instruction()`.
In fact, we encourage you to split your implementation of `process_instruction()` into many other helping functions.
You may decide to have functions for each stages of the pipeline.
Function(s) to handle flushes (adding bubbles into the pipeline), etc.

### 3. run.c

**Implement** the following function:

    void process_instruction()

The `process_instruction()` function is used by the `cycle()` function to simulate a `cycle` of the pipelined simulator.
Each `cycle()` the pipeline will advance to the next instruction (if there are no stalls/hazards, etc.).
Your internal register, memory, and pipeline register state should be updated according to the instruction
that is being executed at each stage.

1. EX_control
EX stage에서 사용될 selection bit의 모음
RegDst, ALUOp, ALUsrc가 있으며 ALUOp는 편의상 instruction의 opcode를 사용함

2. MEM_control
MEM stage에서 사용될 selection bit의 모음
Branch, MemWrite, MemRead가 있음

3. WB_control
WB stage에서 사용될 selection bit의 모음
MemtoReg, RegWrite가 있음

4. Control_unit
위 3가지의 pipeline register의 모음
ID stage에서 control unit을 통해 pipeline register를 control_init함수를 통해 initialize함

5. CPU_State
- ID_EX_EX: ID/EX에 있는 EX pipeline register
- ID_EX_MEM, EX_MEM_MEM: ID/EX, EX/MEM에 있는 MEM pipeline register
- ID_EX_WB, EX_MEM_WB, MEM_WB_WB: ID/EX, EX/MEM, MEM/WB에 있는 WB pipeline register
- ID_EX_SH, ID_EX_MEM, ID_EX_RS, ID_EX_RT: ALU, forwarding, hazard detection 등에 사용되는 instruction의 parsing한 결과를 Decoder를 통해 따로 저장함

- EX_MEM_BR_TAKE: ALU 결과를 통해 branch를 take하는지 아닌지를 결정
- ID_EX_DEST, EX_MEM_DEST: R type은 rd, branch를 제외한 I type은 rt에 계산 결과를 저장함

- EX_MEM_FORWARD_REG, EX_MEM_FORWARD_VALUE: EX forward하기 위한 register와 value
- MEM_WB_FORWARD_REG, MEM_WB_FORWARD_VALUE: MEM forward하기 위한 register와 value
    forwarding register와 value는 pipeline 시작하기 전에 설정한 후에 EX stage에서 forwarding condition에 충족하면 forwarding 진행함

- PCSrc: PC가 branch target / jump target / pc+4 중 어느 곳으로 이동할지 결정할 selection bit
- PREDICTION: 1-bit predictor에서 사용될 현재의 예측 상태, 시작은 NOT_TAKE로 설정함
- PREDICT_SUCCESS: branch prediction이 성공했는지를 나타내는 Flag, 예측이 성공한 경우에만 pc를 계속해서 update함 (실패한 경우 MEM stage에서 flush 됨)
- HAZARD: ID stage에서 진행하는 load-use hazard flag, hazard_detection_unit에서 hazard condition 충족하면 1로 set하여 IF stage에서 PC를 update하지 않고 stall 추가함, pipe_init에서 stall 진행
