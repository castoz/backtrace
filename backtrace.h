#ifndef __BACKTRACE_H
#define __BACKTRACE_H
#include <asm/ptrace.h> //for struct pt_regs

/*
 * Major opcodes; before MIPS IV cop1x was called cop3.
 */
enum major_op {
	spec_op, bcond_op, j_op, jal_op,
	beq_op, bne_op, blez_op, bgtz_op,
	addi_op, addiu_op, slti_op, sltiu_op,
	andi_op, ori_op, xori_op, lui_op,
	cop0_op, cop1_op, cop2_op, cop1x_op,
	beql_op, bnel_op, blezl_op, bgtzl_op,
	daddi_op, daddiu_op, ldl_op, ldr_op,
	spec2_op, jalx_op, mdmx_op, spec3_op,
	lb_op, lh_op, lwl_op, lw_op,
	lbu_op, lhu_op, lwr_op, lwu_op,
	sb_op, sh_op, swl_op, sw_op,
	sdl_op, sdr_op, swr_op, cache_op,
	ll_op, lwc1_op, lwc2_op, pref_op,
	lld_op, ldc1_op, ldc2_op, ld_op,
	sc_op, swc1_op, swc2_op, major_3b_op,
	scd_op, sdc1_op, sdc2_op, sd_op
};

/*
 * func field of spec opcode.
 */
enum spec_op {
	sll_op, movc_op, srl_op, sra_op,
	sllv_op, pmon_op, srlv_op, srav_op,
	jr_op, jalr_op, movz_op, movn_op,
	syscall_op, break_op, spim_op, sync_op,
	mfhi_op, mthi_op, mflo_op, mtlo_op,
	dsllv_op, spec2_unused_op, dsrlv_op, dsrav_op,
	mult_op, multu_op, div_op, divu_op,
	dmult_op, dmultu_op, ddiv_op, ddivu_op,
	add_op, addu_op, sub_op, subu_op,
	and_op, or_op, xor_op, nor_op,
	spec3_unused_op, spec4_unused_op, slt_op, sltu_op,
	dadd_op, daddu_op, dsub_op, dsubu_op,
	tge_op, tgeu_op, tlt_op, tltu_op,
	teq_op, spec5_unused_op, tne_op, spec6_unused_op,
	dsll_op, spec7_unused_op, dsrl_op, dsra_op,
	dsll32_op, spec8_unused_op, dsrl32_op, dsra32_op
};

/*
 * bitfields depend from byteorder.
 */
#ifdef __MIPSEB__
struct j_format {	/* Jump format */
	unsigned int opcode : 6;
	unsigned int target : 26;
};

struct i_format {	/* Immediate format (addi, lw, ...) */
	unsigned int opcode : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	signed int simmediate : 16;
};

struct u_format {	/* Unsigned immediate format (ori, xori, ...) */
	unsigned int opcode : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int uimmediate : 16;
};

struct c_format {	/* Cache (>= R6000) format */
	unsigned int opcode : 6;
	unsigned int rs : 5;
	unsigned int c_op : 3;
	unsigned int cache : 2;
	unsigned int simmediate : 16;
};

struct r_format {	/* Register format */
	unsigned int opcode : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int rd : 5;
	unsigned int re : 5;
	unsigned int func : 6;
};

struct sp3_format {	/* special-3 format */
	unsigned int opcode : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int offset : 10;
	unsigned int sp3_opcode : 6;
};

struct lx_format {	/* LX format */
	unsigned int opcode : 6;
	unsigned int base : 5;
	unsigned int index : 5;
	unsigned int rd: 5;
	unsigned int lx_opcode : 5;
	unsigned int sp3_opcode : 6;
};

struct p_format {	/* Performance counter format (R10000) */
	unsigned int opcode : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int rd : 5;
	unsigned int re : 5;
	unsigned int func : 6;
};

struct f_format {	/* FPU register format */
	unsigned int opcode : 6;
	unsigned int : 1;
	unsigned int fmt : 4;
	unsigned int rt : 5;
	unsigned int rd : 5;
	unsigned int re : 5;
	unsigned int func : 6;
};

struct ma_format {	/* FPU multipy and add format (MIPS IV) */
	unsigned int opcode : 6;
	unsigned int fr : 5;
	unsigned int ft : 5;
	unsigned int fs : 5;
	unsigned int fd : 5;
	unsigned int func : 4;
	unsigned int fmt : 2;
};

#elif defined(__MIPSEL__)

struct j_format {	/* Jump format */
	unsigned int target : 26;
	unsigned int opcode : 6;
};

struct i_format {	/* Immediate format */
	signed int simmediate : 16;
	unsigned int rt : 5;
	unsigned int rs : 5;
	unsigned int opcode : 6;
};

struct u_format {	/* Unsigned immediate format */
	unsigned int uimmediate : 16;
	unsigned int rt : 5;
	unsigned int rs : 5;
	unsigned int opcode : 6;
};

struct c_format {	/* Cache (>= R6000) format */
	unsigned int simmediate : 16;
	unsigned int cache : 2;
	unsigned int c_op : 3;
	unsigned int rs : 5;
	unsigned int opcode : 6;
};

struct r_format {	/* Register format */
	unsigned int func : 6;
	unsigned int re : 5;
	unsigned int rd : 5;
	unsigned int rt : 5;
	unsigned int rs : 5;
	unsigned int opcode : 6;
};

struct sp3_format {	/* special-3 format */
	unsigned int sp3_opcode : 6;
	unsigned int offset : 10;
	unsigned int rt : 5;
	unsigned int rs : 5;
	unsigned int opcode : 6;
};

struct lx_format {	/* LX format */
	unsigned int sp3_opcode : 6;
	unsigned int lx_opcode : 5;
	unsigned int rd: 5;
	unsigned int index : 5;
	unsigned int base : 5;
	unsigned int opcode : 6;
};

struct p_format {	/* Performance counter format (R10000) */
	unsigned int func : 6;
	unsigned int re : 5;
	unsigned int rd : 5;
	unsigned int rt : 5;
	unsigned int rs : 5;
	unsigned int opcode : 6;
};

struct f_format {	/* FPU register format */
	unsigned int func : 6;
	unsigned int re : 5;
	unsigned int rd : 5;
	unsigned int rt : 5;
	unsigned int fmt : 4;
	unsigned int : 1;
	unsigned int opcode : 6;
};

struct ma_format {	/* FPU multipy and add format (MIPS IV) */
	unsigned int fmt : 2;
	unsigned int func : 4;
	unsigned int fd : 5;
	unsigned int fs : 5;
	unsigned int ft : 5;
	unsigned int fr : 5;
	unsigned int opcode : 6;
};

#else /* !defined (__MIPSEB__) && !defined (__MIPSEL__) */
#error "MIPS but neither __MIPSEL__ nor __MIPSEB__?"
#endif

union mips_instruction {
	unsigned int word;
	unsigned short halfword[2];
	unsigned char byte[4];
	struct j_format j_format;
	struct i_format i_format;
	struct u_format u_format;
	struct c_format c_format;
	struct r_format r_format;
	struct f_format f_format;
        struct ma_format ma_format;
};

char * addr_to_name(unsigned long addr);
void show_backtrace();

#endif

