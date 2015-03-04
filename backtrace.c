#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <asm/ptrace.h> //for struct pt_regs
#include <unistd.h>
#include "backtrace.h"

typedef unsigned char u8;

/*
 * These will be re-linked against their real values
 * during the second link stage.
 */
extern const unsigned long uallsyms_addresses[] __attribute__((weak));
extern const u8 uallsyms_names[] __attribute__((weak));

/*
 * Tell the compiler that the count isn't in the small data section if the arch
 * has one (eg: FRV).
 */
extern const unsigned long uallsyms_num_syms
__attribute__((weak, section(".rodata")));

#ifdef DEBUG_BACKTRACE
static int nested_depth = 10;
#define _CORE_SIZE (2* 1024 * 1024)
#else
#endif

struct mips_frame_info {
	void		*func;
	unsigned long	* ra;
	unsigned long	func_size;
	int		frame_size;
	int		pc_offset;
};

static inline int is_jal_jalr_jr_ins(union mips_instruction *ip)
{
	if (ip->j_format.opcode == jal_op)
		return 1;
	if (ip->r_format.opcode != spec_op)
		return 0;
	return ip->r_format.func == jalr_op || ip->r_format.func == jr_op;
}

static inline int is_sp_move_ins(union mips_instruction *ip)
{
	/* addiu/daddiu sp,sp,-imm */
	if (ip->i_format.rs != 29 || ip->i_format.rt != 29)
		return 0;
	if (ip->i_format.opcode == addiu_op || ip->i_format.opcode == daddiu_op)
		return 1;
	return 0;
}

static inline int is_ra_save_ins(union mips_instruction *ip)
{
	/* sw / sd $ra, offset($sp) */
	return (ip->i_format.opcode == sw_op || ip->i_format.opcode == sd_op) &&
		ip->i_format.rs == 29 &&
		ip->i_format.rt == 31;
}

/* address validation: addr must belong to symbol list - uallsyms_addresses[] */
static inline int is_usym_addr(unsigned long addr)
{
	if (!uallsyms_addresses || uallsyms_num_syms <= 0)
		return 0;
	if (addr >= uallsyms_addresses[0] && addr < uallsyms_addresses[uallsyms_num_syms - 1])
		return 1;
	return 0;
}

static pid_t hold_ra()
{
	/* do nothing. */
	return getpid();
}

/* get general registers value.*/
static __always_inline void prepare_frametrace(struct pt_regs *regs)
{
	pid_t _i = 0;
	_i = hold_ra(); //ensure ra is pointed to the function itself.
	printf("%d\n", _i);
	
	__asm__ __volatile__(
		".set noreorder\n\t"
		".set push\n\t"
		".set noat\n\t"

		"1: la $1, 1b\n\t"
		"sw $1, %0\n\t"
		"sw $29, %1\n\t"
		"sw $31, %2\n\t"

		".set pop\n\t"
		".set reorder\n\t"
		: "=m" (regs->cp0_epc),
		"=m" (regs->regs[29]), "=m" (regs->regs[31])
		: : "memory");
}

/* search for the position of target in ascending order base[].
return index in base[] if found target;
return the index that 'target' should be inserted into base[] if not found target, if target should be inserted at the front, return -1. */
static int bsearch_index(const unsigned long base[], int len, unsigned long target)
{
	int start = 0, end = len - 1;
	int middle;
	
	if (!base || base[0] > target)
	{
		return -1;
	}
	if (base[len - 1] < target)
	{
		return (len - 1);
	}

	/* binary search */
	while (start <= end)
	{
		middle = (start + end) / 2;
		if (base[middle] == target)
		{
			return middle;
		}
		if (base[end] == target)
		{
			return end;
		}
		if ((start + 1 == end) || (start == end))
		{
			return start;
		}
		
		/* always ensure start < target, and end > end */
		if (base[middle] > target)
		{
			end = middle;
		}
		if (base[middle] < target)
		{
			start = middle;
		}
	}
	
	return -1;
}

/* get function name at index of symbol table. */
static char * get_func_name(int index)
{
	int pos = 0, len;
	char * name = NULL;
	
	if (index >= 0 && index < uallsyms_num_syms)
	{
		/* find the position of function name */
		for (pos = 0; index > 0; index--)
		{
			pos += uallsyms_names[pos] + 1;
		}
		
		/* len(1)-type(1)-name */
		len = uallsyms_names[pos] - 1; //len is include type, so name length is len-1
		name = (char *)malloc(sizeof(char) * (len + 1));
		if (!name)
			return NULL;

		memset(name, 0, len + 1);
		pos = pos + 2;	//skip len and type(t, T etc.)
		for (index = 0; index < len; index++, pos++)
		{
			name[index] = uallsyms_names[pos];
		}
	}

	return name;
}

/* turn addr to function name. */
char * addr_to_name(unsigned long addr)
{
	int index = 0;
	
	if (!is_usym_addr(addr))
	{
		return NULL;
	}

	index = bsearch_index(uallsyms_addresses, uallsyms_num_syms, addr);

	return get_func_name(index);
}

/* recursively look for sp and ra. sp for stack address space, ra for text section. */
static void print_caller(unsigned long sp, unsigned long ra)
{
	int index = 0;
	union mips_instruction * ip;
	unsigned int max_insns;
	unsigned int i;
	struct mips_frame_info info;
	char * caller_name = NULL;

	memset(&info, 0, sizeof(info));

	if (!is_usym_addr(ra))
	{
		return;
	}

	/* looking for the position of function where ra is pointed at. */
#if 0
	while (index < uallsyms_num_syms)
	{
		if (uallsyms_addresses[index] <= ra && uallsyms_addresses[index + 1] > ra)
		{
			break;
		}
		index++;
	}
	//printf("index = %d\n", index);
#else
	index = bsearch_index(uallsyms_addresses, uallsyms_num_syms, ra);
#endif

	ip = (union mips_instruction * )uallsyms_addresses[index]; //the first instruction in function
	if (!ip)
	{
		return;
	}

	caller_name = get_func_name(index);
	printf("\t=>%s()+0x%lx\n", caller_name, ra - (unsigned long)ip);

	/* only search in instructions already executed. */
	max_insns = (ra - (unsigned long)ip) / sizeof(union mips_instruction);
	if (max_insns == 0)
	{
		max_insns = 128U;	/* unknown function size */
	}
	max_insns = max_insns < 128U ? max_insns : 128U;

	info.func = ip;
	info.frame_size = 0;
	info.func_size = 0; //not used
	info.pc_offset = -1; //not used
	info.ra = NULL;

	/* find sp and ra (userspace functions use fp, so sp is not changed) */
	for (i = 0; i < max_insns; i++, ip++)
	{
		if (is_jal_jalr_jr_ins(ip))
			break;
		if (!info.frame_size) {
			if (is_sp_move_ins(ip))
			{
				info.frame_size = - ip->i_format.simmediate; //size of function stack
			}
			continue;
		}
		if (info.pc_offset == -1 && is_ra_save_ins(ip)) { //find ra
			info.ra = (unsigned long *)(sp + ip->i_format.simmediate);
			break;
		}
	}

	/* jump to caller's stack. */
	sp = sp + info.frame_size;

	if (caller_name)
		free(caller_name);

	if (info.ra)
	{
		print_caller(sp, *(info.ra));
	}
}

/* print back trace functions */
void show_backtrace()
{
	struct pt_regs regs;
	prepare_frametrace(&regs);

	printf("Call trace:\n");
	print_caller(regs.regs[29], regs.regs[31]);
	printf("\n");
}

