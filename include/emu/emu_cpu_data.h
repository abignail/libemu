#ifndef EMU_CPU_DATA_H_
#define EMU_CPU_DATA_H_

#include <stdint.h>

#include "emu/emu_cpu_instruction.h"
#include "emu/emu_instruction.h"

enum emu_cpu_flag {
	f_cf = 0, f_pf = 2, f_af = 4, f_zf = 6, f_sf = 7, f_tf = 8, f_if = 9,
	f_df = 10, f_of = 11
};

#define CPU_FLAG_SET(cpu_p, fl) (cpu_p)->eflags |= 1 << (fl)
#define CPU_FLAG_UNSET(cpu_p, fl) (cpu_p)->eflags &= ~(1 << (fl))
#define CPU_FLAG_TOGGLE(cpu_p, fl) (cpu_p)->eflags ^= 1 << (fl)
#define CPU_FLAG_ISSET(cpu_p, fl) ((cpu_p)->eflags & (1 << (fl)))

struct emu_cpu
{
	struct emu *emu;
	struct emu_memory *mem;
	
	uint32_t eip;
	uint32_t eflags;
	uint32_t reg[8];
	uint16_t *reg16[8];
	uint8_t *reg8[8];

	struct emu_instruction 			instr;
	struct emu_cpu_instruction_info 	*cpu_instr_info;
};


#define MODRM_MOD(x) (((x) >> 6) & 3)
#define MODRM_REGOPC(x) (((x) >> 3) & 7)
#define MODRM_RM(x) ((x) & 7)
    
#define SIB_SCALE(x) (((x) >> 6) & 3)
#define SIB_INDEX(x) (((x) >> 3) & 7)
#define SIB_BASE(x) ((x) & 7)

#define PREFIX_ADSIZE (1 << 0)
#define PREFIX_OPSIZE (1 << 1)
#define PREFIX_LOCK (1 << 2)
#define PREFIX_CS_OVR (1 << 3)
#define PREFIX_DS_OVR (1 << 4)
#define PREFIX_ES_OVR (1 << 5)
#define PREFIX_FS_OVR (1 << 6)
#define PREFIX_GS_OVR (1 << 7)
#define PREFIX_SS_OVR (1 << 8)
#define PREFIX_F2 (1 << 9)
#define PREFIX_F3 (1 << 10)

#define OPSIZE_8 1
#define OPSIZE_16 2
#define OPSIZE_32 3


#define MAX_INT8           127
#define MIN_INT8          -128

#define MAX_UINT8          255
#define MIN_UINT8            0

#define MAX_INT16        32767
#define MIN_INT16       -MAX_INT16 -1

#define MAX_UINT16       65535
#define MIN_UINT16           0


#define MAX_INT32   2147483647
#define MIN_INT32  -MAX_INT32 -1

#define MAX_UINT32	4294967295U
#define MIN_UINT32           0




extern int64_t max_inttype_borders[][2][2];

#define INT(bits) int##bits##_t
#define UINT(bits) uint##bits##_t

#if !defined(INSTR_CALC)
#define INSTR_CALC(bits, a, b, c, operation)			\
UINT(bits) operand_a = a;								\
UINT(bits) operand_b = b;								\
UINT(bits) operation_result = operand_a operation operand_b;	\
c = operation_result;
#endif // INSTR_CALC

#if !defined(INSTR_SET_FLAG_ZF)
#define INSTR_SET_FLAG_ZF(cpu)						\
{															\
	if (operation_result == 0)								\
		CPU_FLAG_SET(cpu, f_zf);								\
	else													\
		CPU_FLAG_UNSET(cpu, f_zf);							\
}
#endif // INSTR_SET_FLAG_ZF

#if !defined(INSTR_SET_FLAG_PF)
#define INSTR_SET_FLAG_PF(cpu)						\
{															\
	int num_p_bits=0;										\
	int i;													\
	for ( i=0;i<8;i++ )										\
		if (operation_result & (1 << i) )					\
			num_p_bits++;									\
															\
	if ((num_p_bits % 2) == 0)								\
		CPU_FLAG_SET(cpu, f_pf);								\
	else													\
		CPU_FLAG_UNSET(cpu, f_pf);							\
}
#endif // INSTR_SET_FLAG_PF

#if !defined(INSTR_SET_FLAG_SF)
#define INSTR_SET_FLAG_SF(cpu)								\
{																	\
	if (operation_result & (1 << (sizeof(operation_result)*8 - 1)))	\
		CPU_FLAG_SET(cpu, f_sf);										\
	else															\
		CPU_FLAG_UNSET(cpu, f_sf);									\
}
#endif // INSTR_SET_FLAG_SF

#if !defined(INSTR_SET_FLAG_OF)
#define INSTR_SET_FLAG_OF(cpu, operand, bits)								\
{																				\
	int64_t sx = (INT(bits))operand_a;                                   \
	int64_t sy = (INT(bits))operand_b;                                   \
	int64_t sz = 0;                                                             \
																				\
	sz = sx operand sy;															\
																				\
	if (sz < max_inttype_borders[sizeof(operation_result)][0][0] || sz > max_inttype_borders[sizeof(operation_result)][0][1] \
	|| sz != (INT(bits))operation_result )									    \
	{                                                                           \
		CPU_FLAG_SET(cpu, f_of);                                                 \
	}else                                                                       \
	{                                                                           \
		CPU_FLAG_UNSET(cpu, f_of);                                               \
	}                                                                           \
}
#endif // INSTR_SET_FLAG_OF


#if !defined(INSTR_SET_FLAG_CF)
#define INSTR_SET_FLAG_CF(cpu, operand)									\
{																				\
	uint64_t ux = (uint64_t)operand_a;                                           \
	uint64_t uy = (uint64_t)operand_b;                                           \
	uint64_t uz = 0;                                                            \
																				\
	uz = ux operand uy;															\
																				\
	if (uz < max_inttype_borders[sizeof(operation_result)][1][0] || uz > max_inttype_borders[sizeof(operation_result)][1][1] \
	|| uz != (uint64_t)operation_result )									    \
	{                                                                           \
		CPU_FLAG_SET(cpu, f_cf);                                                 \
	}else                                                                       \
	{                                                                           \
		CPU_FLAG_UNSET(cpu, f_cf);                                               \
	}                                                                           \
}
#endif // INSTR_SET_FLAG_CF


#include <string.h>
#define WORD_UPPER_TO_BYTE(to,from) \
memcpy(&(to),((uint8_t *)&(from))+1,1);

#define WORD_LOWER_TO_BYTE(to,from) \
memcpy(&(to),&(from),1);

#define DWORD_UPPER_TO_WORD(to,from) \
memcpy(&(to),((uint8_t *)&(from))+2,2);

#define DWORD_LOWER_TO_WORD(to,from) \
memcpy(&(to),&(from),2);

#define QWORD_UPPER_TO_DWORD(to,from) \
memcpy(&(to),((uint8_t *)&(from))+4,4);

#define QWORD_LOWER_TO_DWORD(to,from) \
memcpy(&(to),&(from),4);

#define DWORD_FROM_WORDS(to, upper, lower) \
memcpy(&to,&lower,2); \
memcpy(&to+2,&lower,2); 

#define QWORD_FROM_DWORDS(to, upper, lower) \
memcpy(&to,&lower,4); \
memcpy(&to+4,&lower,4); 


#endif /*EMU_CPU_DATA_H_*/
