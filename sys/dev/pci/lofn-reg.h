#ifndef WH_LOFN_REG_H_7007b582_
#define WH_LOFN_REG_H_7007b582_

#define LOFN_BAR 0x10

/*
 * LOFN_WIN_xE_yE is xE for bytes within words, yE for words within
 *  registers.
 */
#define LOFN_WIN_BE_LE 0x0000
#define LOFN_WIN_BE_BE 0x2000
#define LOFN_WIN_LE_LE 0x4000
#define LOFN_WIN_LE_BE 0x6000

/*
 * Data register space, window-relative.
 */
#define LOFN_BIGNUM_BASE 0x0000
#define LOFN_BIGNUM(reg,word) (LOFN_BIGNUM_BASE+(((reg)&15)<<7)+(((word)&15)<<2))

/*
 * Data length space, window-relative.
 */
#define LOFN_LENGTH_BASE 0x1000
#define LOFN_LENGTH(reg) (LOFN_LENGTH_BASE+(((reg)&15)<<2))

/*
 * RNG FIFO space, window-relative.
 */
#define LOFN_RNG_FIFO 0x1080
#define LOFN_RNG_FIFO_LEN 0x40 // bytes

/*
 * ALU code space, window-relative.
 */
#define LOFN_CODE_BASE 0x1100
#define LOFN_CODE(inx) (LOFN_CODE_BASE+(((inx)&31)<<2))

/*
 * Control and status registers, window-relative.
 */
#define LOFN_COMMAND 0x1fd4
#define LOFN_COMMAND_PC 0x0000003f
#define LOFN_COMMAND_PC_S 0
#define LOFN_COMMAND_PC_M 0x3f
#define LOFN_STATUS  0x1fd8
#define LOFN_STATUS_CARRY     0x00000008
#define LOFN_STATUS_RNG_UF    0x00001000
#define LOFN_STATUS_RNG_READY 0x00004000
#define LOFN_STATUS_DONE      0x00008000
#define LOFN_INTENB  0x1fdc
#define LOFN_INTENB_RNG  0x00004000
#define LOFN_INTENB_DONE 0x00008000
#define LOFN_RNGCFG  0x1fe0
#define LOFN_RNGCFG_OUT_PRESCALE 0x00000080
#define LOFN_RNGCFG_1ST_PRESCALE 0x00000f00
#define LOFN_CFG_1   0x1fe4
#define LOFN_CFG_1_RESET 0x00000001
// XXX I see 0x00000002 set, dunno meaning
#define LOFN_CFG_1_PLL   0x00000038
#define LOFN_CFG_1_PLL_BYP  0x00000000
#define LOFN_CFG_1_PLL_X1   0x00000008
#define LOFN_CFG_1_PLL_X1_5 0x00000010
#define LOFN_CFG_1_PLL_X2   0x00000018
#define LOFN_CFG_1_PLL_X2_5 0x00000020
#define LOFN_CFG_1_PLL_X3   0x00000028
#define LOFN_CFG_1_PLL_X4   0x00000030
// XXX what does 0x00000038 mean?
#define LOFN_CFG_2   0x1fe8
#define LOFN_CFG_2_RNG_ENB 0x00000001
#define LOFN_CFG_2_ALU_ENB 0x00000002
#define LOFN_CHIPID  0x1fec
#define LOFN_CHIPID_ID 0x0000ffff
#define LOFN_CHIPID_ID_S 0
#define LOFN_CHIPID_ID_M 0xffff

#define LOFN_INST_LAST 0x80000000
#define LOFN_INST_OPC  0x7c000000
#define LOFN_INST_OPC_S 26
#define LOFN_INST_OPC_M 0x1f
#define LOFN_INST_OPC_MODEXP 0x00 // Modular exponentiation
#define LOFN_INST_OPC_MODMUL 0x01 // Modular multiplication
#define LOFN_INST_OPC_MODRED 0x02 // Modular reduction
#define LOFN_INST_OPC_MODADD 0x03 // Modular addition
#define LOFN_INST_OPC_MODSUB 0x04 // Modular subtraction
#define LOFN_INST_OPC_ADD    0x05 // Addition
#define LOFN_INST_OPC_SUB    0x06 // Subtraction
#define LOFN_INST_OPC_ADD_C  0x07 // Addition with carry
#define LOFN_INST_OPC_SUB_B  0x08 // Subtraction with borrow
#define LOFN_INST_OPC_2K_MUL 0x09 // 2048-bit multiplication
#define LOFN_INST_OPC_SHR    0x0a // Shift right
#define LOFN_INST_OPC_SHL    0x0b // Shift left
#define LOFN_INST_OPC_INC    0x0c // Increment
#define LOFN_INST_OPC_DEC    0x0d // Decrement
#define LOFN_INST_OPC_SETLEN 0x0e // "Set length tag" - ??
#define LOFN_INST_OPC_NOP    0x1f // NOP
#define LOFN_INST_RD   0x03e00000
#define LOFN_INST_RD_S 21
#define LOFN_INST_RD_M 0x1f
#define LOFN_INST_RA   0x001f0000
#define LOFN_INST_RA_S 16
#define LOFN_INST_RA_M 0x1f
#define LOFN_INST_RB   0x0000f800
#define LOFN_INST_RB_S 11
#define LOFN_INST_RB_M 0x1f
#define LOFN_INST_RM   0x000007c0
#define LOFN_INST_RM_S 6
#define LOFN_INST_RM_M 0x1f
#define LOFN_INST_VAL  0x000007ff
#define LOFN_INST_VAL_S 0
#define LOFN_INST_VAL_M 0x7ff
#define LOFN_INST_RESV_RRRR 0x0000003f
#define LOFN_INST_RESV_RRV  0x0000f800

#define LOFN_INST_RRRR(last,opc,rd,ra,rb,rm)\
	( ((last) ? LOFN_INST_LAST : 0) |			\
	   (((opc) & LOFN_INST_OPC_M) << LOFN_INST_OPC_S) |	\
	   (((rd) & LOFN_INST_RD_M) << LOFN_INST_RD_S) |	\
	   (((ra) & LOFN_INST_RA_M) << LOFN_INST_RA_S) |	\
	   (((rb) & LOFN_INST_RB_M) << LOFN_INST_RB_S) |	\
	   (((rm) & LOFN_INST_RM_M) << LOFN_INST_RM_S) )
#define LOFN_INST_RRV(last,opc,rd,ra,val)\
	( ((last) ? LOFN_INST_LAST : 0) |			\
	   (((opc) & LOFN_INST_OPC_M) << LOFN_INST_OPC_S) |	\
	   (((rd) & LOFN_INST_RD_M) << LOFN_INST_RD_S) |	\
	   (((ra) & LOFN_INST_RA_M) << LOFN_INST_RA_S) |	\
	   (((val) & LOFN_INST_VAL_M) << LOFN_INST_VAL_S) )

#endif
