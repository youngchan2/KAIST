#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define TEXT 0x400000
#define DATA 0x10000000
#define MAX_BINARY 10000

typedef struct datanode
{
	char name[50];
	int value;
	int address;
} dataNode;

typedef struct Node
{
	dataNode data;
	struct Node *next;
} node;

typedef struct r
{
	int op;
	int rs;
	int rt;
	int rd;
	int shamt;
	int function;
} R;

typedef struct i
{
	int op;
	int rs;
	int rt;
	int constant;
} I;

typedef struct j
{
	int op;
	int address;
} J;

node *createNode(dataNode data)
{
	node *newNode = (node *)malloc(sizeof(node));
	newNode->data = data;
	newNode->next = NULL;

	return newNode;
}

void addNode(node **head, dataNode data)
{
	node *newNode = createNode(data);

	if (*head == NULL)
	{
		*head = newNode;
		return;
	}
	else
	{
		node *current = *head;
		while (current->next != NULL)
		{
			current = current->next;
		}
		current->next = newNode;
	}
}

int findAddress(node *head, char *name)
{
	node *current = head;
	while (current != NULL)
	{
		if (!strcmp((current->data).name, name))
		{
			return (current->data).address;
		}

		current = current->next;
	}
	return -1;
}

void *vtob(int value, char *tmp, int size)
{
	for (int i = 0; i < size; i++)
	{
		tmp[size - 1 - i] = (value & 1) + '0';
		value >>= 1;
	}
	tmp[size] = '\0';
}

int rtoi(char *reg)
{
	char *tmp = malloc(50);
	int len;
	if (reg[strlen(reg) - 1] == ',')
		len = strlen(reg) - 2;
	else
		len = strlen(reg) - 1;
	strncpy(tmp, reg + 1, len);
	tmp[len] = '\0';

	int val = (int)strtol(tmp, NULL, 10);
	free(tmp);

	return val;
}

R R_format(int num, int opcode, int funct)
{
	R format;
	int rs, rt, rd, shamt;

	if (num == 2) // jr
	{
		char *input = malloc(50);
		scanf("%s", input);
		int r = rtoi(input);
		rs = r;
		rt = 0;
		rd = 0;
		shamt = 0;

		free(input);
	}
	else
	{
		char *input1 = malloc(50);
		char *input2 = malloc(50);
		char *input3 = malloc(50);

		scanf("%s", input1);
		scanf("%s", input2);
		scanf("%s", input3);

		if (num == 6 || num == 7) // sll srl
		{
			rs = 0;
			rd = rtoi(input1);
			rt = rtoi(input2);

			if (!strncmp(input3, "0x", 2))
				shamt = (int)strtol(input3, NULL, 16);
			else
				shamt = atoi(input3);
		}
		else
		{
			rs = rtoi(input2);
			rt = rtoi(input3);
			rd = rtoi(input1);
			shamt = 0;
		}

		free(input1);
		free(input2);
		free(input3);
	}

	format.op = opcode;
	format.rs = rs;
	format.rt = rt;
	format.rd = rd;
	format.shamt = shamt;
	format.function = funct;

	return format;
}

I I_format(int num, int opcode)
{
	I format;
	int rs, rt, constant;

	if (num == 13) // lui
	{
		char *input1 = malloc(50);
		char *input2 = malloc(50);

		scanf("%s", input1);
		scanf("%s", input2);

		rs = 0;
		rt = rtoi(input1);
		if (!strncmp(input2, "0x", 2))
			constant = (int)strtol(input2 + 2, NULL, 16);
		else
			constant = atoi(input2);
		free(input1);
		free(input2);
	}
	else if (num == 14 || num == 17) // lw sw
	{
		char *input1 = malloc(50);
		char *input2 = malloc(50);

		char *tmp_const = malloc(50);
		char *tmp_rs = malloc(50);

		scanf("%s", input1);
		scanf("%s", input2);

		rt = rtoi(input1);

		int pos = 0;
		for (size_t i = 0; i < strlen(input2); i++)
		{
			if (input2[i] == '(')
			{
				pos = i;
			}
		}
		strncpy(tmp_const, input2, pos);
		tmp_const[pos] = '\0';

		if (tmp_const[0] == '-')
		{
			if (!strncmp(tmp_const + 1, "0x", 2))
				constant = (int)strtol(tmp_const + 2, NULL, 16);
			else
				constant = atoi(tmp_const);
		}
		else
		{
			if (!strncmp(tmp_const, "0x", 2))
				constant = (int)strtol(tmp_const + 2, NULL, 16);
			else
				constant = atoi(tmp_const);
		}

		strncpy(tmp_rs, input2 + pos + 1, strlen(input2) - 1 - (pos + 1));
		rs = rtoi(tmp_rs);

		free(input1);
		free(input2);
	}
	else
	{
		char *input1 = malloc(50);
		char *input2 = malloc(50);
		char *input3 = malloc(50);

		scanf("%s", input1);
		scanf("%s", input2);
		scanf("%s", input3);

		rt = rtoi(input1);
		rs = rtoi(input2);

		if (!strncmp(input3, "0x", 2))
			constant = (int)strtol(input3 + 2, NULL, 16);
		else
			constant = atoi(input3);
		free(input1);
		free(input2);
		free(input3);
	}

	format.op = opcode;
	format.rs = rs;
	format.rt = rt;
	format.constant = constant;

	return format;
}

I I_branch(int num, int opcode, node *label_list, int pc)
{
	I format;
	int rs, rt, constant, address;

	char *input1 = malloc(50);
	char *input2 = malloc(50);
	char *input3 = malloc(50);

	scanf("%s", input1);
	scanf("%s", input2);
	scanf("%s", input3);

	rs = rtoi(input1);
	rt = rtoi(input2);
	address = findAddress(label_list, input3);
	constant = ((address - pc) >> 2) & 0xffff;

	format.op = opcode;
	format.rs = rs;
	format.rt = rt;
	format.constant = constant;

	free(input1);
	free(input2);
	free(input3);

	return format;
}

J J_format(int opcode, node *label_list)
{
	J format;

	char *input = malloc(50);
	scanf("%s", input);

	int address = findAddress(label_list, input);
	address = address & 0x0fffffff;
	address = address >> 2;

	format.op = opcode;
	format.address = address;

	return format;
}

void *R_itob(R format, char *tmp)
{
	char *buf = malloc(50);

	vtob(format.op, buf, 6);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.rs, buf, 5);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.rt, buf, 5);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.rd, buf, 5);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.shamt, buf, 5);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.function, buf, 6);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	free(buf);
}

void *I_itob(I format, char *tmp)
{
	char *buf = malloc(50);
	vtob(format.op, buf, 6);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.rs, buf, 5);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.rt, buf, 5);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.constant, buf, 16);
	strcat(tmp, buf);

	free(buf);
}

void *J_itob(J format, char *tmp)
{
	char *buf = malloc(50);
	vtob(format.op, buf, 6);
	strcat(tmp, buf);
	memset(buf, 0, sizeof(buf));

	vtob(format.address, buf, 26);
	strcat(tmp, buf);
	free(buf);
}

int check_instruction(char *set[], char *s)
{
	for (int i = 0; i < 21; i++)
	{
		if (!strcmp(s, set[i]))
			return i;
	}
	return -1;
}

int check_type(int num)
{
	if (num >= 0 && num < 9)
		return 1;
	else if (num >= 9 && num < 18)
		return 2;
	else if (num >= 18 && num < 20)
		return 3;
	else
		return 4;
}

int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		printf("Usage: ./runfile <assembly file>\n"); // Example) ./runfile /sample_input/example1.s
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	}
	else
	{

		// To help you handle the file IO, the deafult code is provided.
		// If we use freopen, we don't need to use fscanf, fprint,..etc.
		// You can just use scanf or printf function
		// ** You don't need to modify this part **
		// If you are not famailiar with freopen,  you can see the following reference
		// http://www.cplusplus.com/reference/cstdio/freopen/

		// For input file read (sample_input/example*.s)

		char *file = (char *)malloc(strlen(argv[1]) + 3);
		strncpy(file, argv[1], strlen(argv[1]));

		if (freopen(file, "r", stdin) == 0)
		{
			printf("File open Error!\n");
			exit(1);
		}

		// From now on, if you want to read string from input file, you can just use scanf function.
		char *instruction[22] = {"add", "and", "jr", "nor", "or", "slt", "sll", "srl", "sub",
								 "addi", "andi", "beq", "bne", "lui", "lw", "ori", "slti", "sw",
								 "j", "jal",
								 "la"};
		int op[23] = {0, 0, 0, 0, 0, 0, 0, 0, 0,
					  0x8, 0xc, 0x4, 0x5, 0xf, 0x23, 0xd, 0xa, 0x2b,
					  0x2, 0x3,
					  0xf, 0xd};
		int fun[22] = {0x20, 0x24, 0x8, 0x27, 0x25, 0x2a, 0, 0x2, 0x22};
		char *binary = malloc(MAX_BINARY);

		char *s = malloc(100);
		bool data_flag;

		char label_name[50];
		char name[50];
		int value = 0;

		int data_pointer = DATA;
		int text_pointer = TEXT;
		int pc = TEXT;

		node *data_list = NULL;
		node *label_list = NULL;
		//-------------------------------------------------------FIRST READ START------------------------------------------------------------------------------
		while (scanf("%s", s) != -1)
		{
			if (!strcmp(s, ".data"))
			{
				data_flag = true;
				continue;
			}
			else if (!strcmp(s, ".text"))
			{
				data_flag = false;
				continue;
			}
			if (data_flag)
			{
				if (s[strlen(s) - 1] == ':')
				{
					memset(name, 0, sizeof(name));
					strncpy(name, s, strlen(s) - 1);
				}
				else
				{
					if (!strcmp(s, ".word"))
						continue;
					else
					{
						value = 0;
						if (!strncmp(s, "0x", 2))
							value = (int)strtol(s + 2, NULL, 16);
						else
							value = atoi(s);
					}
					dataNode data;
					strcpy(data.name, name);
					data.value = value;
					data.address = data_pointer;
					addNode(&data_list, data);
					data_pointer += 4;
				}
			}
			else
			{
				if (s[strlen(s) - 1] == ':')
				{
					strncpy(label_name, s, strlen(s) - 1);
					dataNode label;
					strcpy(label.name, label_name);
					label.value = 0;
					label.address = text_pointer;
					addNode(&label_list, label);
				}
				else
				{
					if (check_instruction(instruction, s) == 20) // la
					{
						char *arg1 = malloc(50);
						char *arg2 = malloc(50);
						scanf("%s %s", arg1, arg2);

						int address = findAddress(data_list, arg2);

						if (!(address & 0x0000ffff))
							text_pointer += 4;
						else
							text_pointer += 8;
					}
					else if (check_instruction(instruction, s) >= 0)
						text_pointer += 4;
				}
			}
		}

		int text_size = text_pointer - TEXT;
		int data_size = data_pointer - DATA;

		char *text_binary = malloc(33);
		char *data_binary = malloc(33);
		vtob(text_size, text_binary, 32);
		vtob(data_size, data_binary, 32);

		strcat(binary, text_binary);
		strcat(binary, data_binary);

		//-------------------------------------------------------FIRST READ FINSIH------------------------------------------------------------------------------
		//-------------------------------------------------------SECOND READ START------------------------------------------------------------------------------

		if (freopen(file, "r", stdin) == 0)
		{
			printf("File open Error!\n");
			exit(1);
		}
		bool text_flag = false;
		int instr_num;
		while (scanf("%s", s) != -1)
		{
			if (!strcmp(s, ".text"))
			{
				text_flag = true;
				continue;
			}
			if (text_flag)
			{
				if (s[strlen(s) - 1] == ':')
					continue;
				else if ((instr_num = check_instruction(instruction, s)) != -1)
				{
					pc += 4;
					int opcode = op[instr_num];
					char *instr_binary = malloc(33);
					int type = check_type(instr_num);

					if (type == 1)
					{
						R instr_R = R_format(instr_num, opcode, fun[instr_num]);
						R_itob(instr_R, instr_binary);
						strcat(binary, instr_binary);
					}
					else if (type == 2)
					{
						I instr_I;
						if (instr_num == 11 || instr_num == 12)
							instr_I = I_branch(instr_num, opcode, label_list, pc);
						else
							instr_I = I_format(instr_num, opcode);
						I_itob(instr_I, instr_binary);
						strcat(binary, instr_binary);
					}
					else if (type == 3)
					{
						J instr_J = J_format(opcode, label_list);
						J_itob(instr_J, instr_binary);
						strcat(binary, instr_binary);
					}
					else if (type == 4)
					{
						I lui;
						int rs, rt, constant, address;

						char *input1 = malloc(50);
						char *input2 = malloc(50);

						scanf("%s", input1);
						scanf("%s", input2);

						rs = 0;
						rt = rtoi(input1);
						address = findAddress(data_list, input2);
						constant = (address >> 16) & 0xffff;

						lui.op = op[13];
						lui.rs = rs;
						lui.rt = rt;
						lui.constant = constant;
						I_itob(lui, instr_binary);
						strcat(binary, instr_binary);
						memset(instr_binary, 0, sizeof(instr_binary));

						if (address & 0xffff)
						{
							pc += 4;
							I ori;
							ori.op = op[15];
							ori.rs = lui.rt;
							ori.rt = lui.rt;
							ori.constant = (address & 0xffff);
							I_itob(ori, instr_binary);
							strcat(binary, instr_binary);
						}

						free(input1);
						free(input2);
					}
				}
			}
		}

		node *current = data_list;
		while (current != NULL)
		{
			int value = current->data.value;
			char *buf = malloc(50);
			vtob(value, buf, 32);
			strcat(binary, buf);
			free(buf);
			current = current->next;
		}
		// For output file write
		// You can see your code's output in the sample_input/example#.o
		// So you can check what is the difference between your output and the answer directly if you see that file
		// make test command will compare your output with the answer
		file[strlen(file) - 1] = 'o';
		freopen(file, "w", stdout);

		// If you use printf from now on, the result will be written to the output file.
		printf("%s", binary);
	}
	return 0;
}
