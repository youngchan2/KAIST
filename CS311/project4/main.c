#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

typedef struct
{
	int read;
	int write;
	int writeback;
	int readhit;
	int writehit;
	int readmiss;
	int writemiss;
} cache_stat;

int get_bit(int n)
{
	if (n < 0)
	{
		return 0;
	}

	int cnt = 0;

	while (n)
	{
		n = n >> 1;
		++cnt;
	}

	return cnt - 1;
}

/***************************************************************/
/*                                                             */
/*                       Implement below                       */

typedef struct
{
	// design your own structure
	char valid;
	char dirty;
	uint32_t tag;
	uint32_t data;
	uint32_t time;
	uint32_t freq;
} cache_struct;

cache_struct **build_cache(int num_set, int num_way, int block_size)
{
	// generate an instance of cache_struct and return the instance
	// you can change the type of return value as a pointer if you want
	cache_struct **cache = (cache_struct **)malloc(sizeof(cache_struct *) * num_set);
	for (int i = 0; i < num_set; i++)
		cache[i] = (cache_struct *)malloc(sizeof(cache_struct) * num_way);
	for (int i = 0; i < num_set; i++)
		for (int j = 0; j < num_way; j++)
			memset(&cache[i][j], 0, sizeof(cache_struct));
	return cache;
}

void access_cache(cache_struct **cache, cache_stat *stat, char *op, uint32_t addr, int num_set, int num_way, int block_size, int lru_only_flag)
{
	// implement the logic of accessing to cache
	uint32_t tag, data, index, byte_offset;

	int s = get_bit(num_set);
	int b = get_bit(block_size);

	tag = addr >> (s + b);
	// index = (addr >> b) & ((1 << s) - 1);
	index = ((addr << (32 - s - b))) >> b >> (32 - s - b);
	byte_offset = addr % block_size;
	data = addr - byte_offset;

	cache_struct *set = cache[index];

	for (int i = 0; i < num_way; i++)
		if (set[i].valid)
			set[i].time++;

	for (int i = 0; i < num_way; i++)
	{
		if (set[i].valid)
		{
			if ((set[i].tag == tag))
			{
				if (op[0] == 'R')
					stat->readhit++;
				else
				{
					stat->writehit++;
					set[i].dirty = 1;
				}
				set[i].time = 0;
				set[i].freq++;
				return;
			}
		}
		else
		{
			if (op[0] == 'R')
				stat->readmiss++;
			else
				stat->writemiss++;

			set[i].valid = 1;
			set[i].dirty = (op[0] == 'W');
			set[i].tag = tag;
			set[i].data = data;
			set[i].time = 0;
			set[i].freq = 1;
			return;
		}
	}
	// evict
	int evict_location = 0;
	if (lru_only_flag) // lru
	{
		for (int i = 1; i < num_way; i++)
		{
			if (set[evict_location].time < set[i].time)
				evict_location = i;
		}
	}
	else // lflru
	{
		for (int i = 1; i < num_way; i++)
		{
			if (set[evict_location].freq == set[i].freq)
			{
				if (set[evict_location].time < set[i].time)
					evict_location = i;
			}
			else if (set[evict_location].freq > set[i].freq)
				evict_location = i;
		}
	}

	if (op[0] == 'R')
		stat->readmiss++;
	else
		stat->writemiss++;

	if (set[evict_location].dirty == 1)
		stat->writeback++;

	set[evict_location].valid = 1;
	set[evict_location].dirty = (op[0] == 'W');
	set[evict_location].tag = tag;
	set[evict_location].data = data;
	set[evict_location].time = 0;
	set[evict_location].freq = 1;
}

/*                       Implement above                       */
/*                                                             */
/***************************************************************/

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */
/*                                                             */
/***************************************************************/
void cdump(int capacity, int num_way, int block_size, int lru_only_flag)
{

	printf("Cache Configuration:\n");
	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", num_way);
	printf("Block Size: %dB\n", block_size);
	if (lru_only_flag)
		printf("Replacement Policy: LRU\n");
	else
		printf("Replacement Policy: LFLRU\n");
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat                                 */
/*                                                             */
/***************************************************************/
void sdump(cache_stat *stat)
{
	printf("Cache Stat:\n");
	printf("-------------------------------------\n");
	printf("Total reads: %d\n", stat->read);
	printf("Total writes: %d\n", stat->write);
	printf("Write-backs: %d\n", stat->writeback);
	printf("Read hits: %d\n", stat->readhit);
	printf("Write hits: %d\n", stat->writehit);
	printf("Read misses: %d\n", stat->readmiss);
	printf("Write misses: %d\n", stat->writemiss);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */
/*                                                             */
/***************************************************************/
void xdump(cache_struct **cache, int num_set, int num_way, int block_size)
{
	printf("Cache Content:\n");
	printf("-------------------------------------\n");
	for (int i = 0; i < num_way; i++)
	{
		if (i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]", i);
	}
	printf("\n");

	for (int i = 0; i < num_set; i++)
	{
		printf("SET[%d]:   ", i);

		for (int j = 0; j < num_way; j++)
		{
			uint32_t cache_block_addr = 0; // include only the information of tag and index (block offset must be 0)

			/***************************************************************/
			/*                                                             */
			/*                       Implement here                        */
			/*                                                             */
			/***************************************************************/
			cache_block_addr = cache[i][j].data;
			printf("0x%08x  ", cache_block_addr);
		}

		printf("\n");
	}

	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		printf("Usage: %s -c cap:assoc:block_size [-x] [-r] input_trace \n", argv[0]);
		exit(1);
	}

	int capacity;
	int num_set;
	int num_way;
	int block_size;
	int xflag = 0;
	int lru_only_flag = 0;

	{
		char *token;
		int option_flag = 0;

		while ((option_flag = getopt(argc, argv, "c:xr")) != -1)
		{
			switch (option_flag)
			{
			case 'c':
				token = strtok(optarg, ":");
				capacity = atoi(token);
				token = strtok(NULL, ":");
				num_way = atoi(token);
				token = strtok(NULL, ":");
				block_size = atoi(token);
				break;
			case 'x':
				xflag = 1;
				break;
			case 'r':
				lru_only_flag = 1;
				break;
			default:
				printf("Usage: %s -c cap:assoc:block_size [-x] [-r] input_trace \n", argv[0]);
				exit(1);
			}
		}
	}

	char *trace_name;
	trace_name = argv[argc - 1];

	FILE *fp;
	fp = fopen(trace_name, "r"); // read trace file

	if (fp == NULL)
	{
		printf("\nInvalid trace file: %s\n", trace_name);
		return 1;
	}

	cdump(capacity, num_way, block_size, lru_only_flag);

	// cache statistics initialization
	cache_stat stat;
	stat.read = 0;
	stat.write = 0;
	stat.writeback = 0;
	stat.readhit = 0;
	stat.writehit = 0;
	stat.readmiss = 0;
	stat.writemiss = 0;

	// initialize following variables
	// cache_struct cache = {};
	// int num_set = 1; // modify it

	/***************************************************************/
	/*                                                             */
	/*                       Implement here                        */
	/*                                                             */
	/***************************************************************/
	char instr[20];
	char *op;
	uint32_t addr;
	num_set = (capacity / num_way) / block_size;

	cache_struct **cache = build_cache(num_set, num_way, block_size);

	while (fgets(instr, sizeof(instr), fp))
	{
		op = strtok(instr, " ");
		addr = strtoul(strtok(NULL, ""), NULL, 16);
		if (op[0] == 'R')
			stat.read++;
		else
			stat.write++;
		access_cache(cache, &stat, op, addr, num_set, num_way, block_size, lru_only_flag);
	}

	sdump(&stat);
	if (xflag)
	{
		xdump(cache, num_set, num_way, block_size);
	}

	return 0;
}