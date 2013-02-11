/* pj-opc.c -- Definitions for picoJava opcodes.
   Copyright 1999, 2000, 2002, 2003 Free Software Foundation, Inc.
   Contributed by Steve Chamberlain of Transmeta (sac@pobox.com).

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


#include "sysdep.h"
#include "opcode/pj.h"

const pj_opc_info_t pj_opc_info[512] =
{
{ 0x00,   -1, 1, {O_N, O_N}, {"nop"}},
{ 0x01,   -1, 1, {O_N, O_N}, {"aconst_null"}},
{ 0x02,   -1, 1, {O_N, O_N}, {"iconst_m1"}},
{ 0x03,   -1, 1, {O_N, O_N}, {"iconst_0"}},
{ 0x04,   -1, 1, {O_N, O_N}, {"iconst_1"}},
{ 0x05,   -1, 1, {O_N, O_N}, {"iconst_2"}},
{ 0x06,   -1, 1, {O_N, O_N}, {"iconst_3"}},
{ 0x07,   -1, 1, {O_N, O_N}, {"iconst_4"}},
{ 0x08,   -1, 1, {O_N, O_N}, {"iconst_5"}},
{ 0x09,   -1, 1, {O_N, O_N}, {"lconst_0"}},
{ 0x0a,   -1, 1, {O_N, O_N}, {"lconst_1"}},
{ 0x0b,   -1, 1, {O_N, O_N}, {"fconst_0"}},
{ 0x0c,   -1, 1, {O_N, O_N}, {"fconst_1"}},
{ 0x0d,   -1, 1, {O_N, O_N}, {"fconst_2"}},
{ 0x0e,   -1, 1, {O_N, O_N}, {"dconst_0"}},
{ 0x0f,   -1, 1, {O_N, O_N}, {"dconst_1"}},
{ 0x10,   -1, 2, {O_8, O_N}, {"bipush"}},
{ 0x11,   -1, 3, {O_16, O_N}, {"sipush"}},
{ 0x12,   -1, 2, {O_N, O_N}, {"ldc"}},
{ 0x13,   -1, 3, {O_N, O_N}, {"ldc_w"}},
{ 0x14,   -1, 3, {O_N, O_N}, {"ldc2_w"}},
{ 0x15,   -1, 2, {O_U8, O_N}, {"iload"}},
{ 0x16,   -1, 2, {O_U8, O_N}, {"lload"}},
{ 0x17,   -1, 2, {O_U8, O_N}, {"fload"}},
{ 0x18,   -1, 2, {O_U8, O_N}, {"dload"}},
{ 0x19,   -1, 2, {O_U8, O_N}, {"aload"}},
{ 0x1a,   -1, 1, {O_N, O_N}, {"iload_0"}},
{ 0x1b,   -1, 1, {O_N, O_N}, {"iload_1"}},
{ 0x1c,   -1, 1, {O_N, O_N}, {"iload_2"}},
{ 0x1d,   -1, 1, {O_N, O_N}, {"iload_3"}},
{ 0x1e,   -1, 1, {O_N, O_N}, {"lload_0"}},
{ 0x1f,   -1, 1, {O_N, O_N}, {"lload_1"}},
{ 0x20,   -1, 1, {O_N, O_N}, {"lload_2"}},
{ 0x21,   -1, 1, {O_N, O_N}, {"lload_3"}},
{ 0x22,   -1, 1, {O_N, O_N}, {"fload_0"}},
{ 0x23,   -1, 1, {O_N, O_N}, {"fload_1"}},
{ 0x24,   -1, 1, {O_N, O_N}, {"fload_2"}},
{ 0x25,   -1, 1, {O_N, O_N}, {"fload_3"}},
{ 0x26,   -1, 1, {O_N, O_N}, {"dload_0"}},
{ 0x27,   -1, 1, {O_N, O_N}, {"dload_1"}},
{ 0x28,   -1, 1, {O_N, O_N}, {"dload_2"}},
{ 0x29,   -1, 1, {O_N, O_N}, {"dload_3"}},
{ 0x2a,   -1, 1, {O_N, O_N}, {"aload_0"}},
{ 0x2b,   -1, 1, {O_N, O_N}, {"aload_1"}},
{ 0x2c,   -1, 1, {O_N, O_N}, {"aload_2"}},
{ 0x2d,   -1, 1, {O_N, O_N}, {"aload_3"}},
{ 0x2e,   -1, 1, {O_N, O_N}, {"iaload"}},
{ 0x2f,   -1, 1, {O_N, O_N}, {"laload"}},
{ 0x30,   -1, 1, {O_N, O_N}, {"faload"}},
{ 0x31,   -1, 1, {O_N, O_N}, {"daload"}},
{ 0x32,   -1, 1, {O_N, O_N}, {"aaload"}},
{ 0x33,   -1, 1, {O_N, O_N}, {"baload"}},
{ 0x34,   -1, 1, {O_N, O_N}, {"caload"}},
{ 0x35,   -1, 1, {O_N, O_N}, {"saload"}},
{ 0x36,   -1, 2, {O_U8, O_N}, {"istore"}},
{ 0x37,   -1, 2, {O_U8, O_N}, {"lstore"}},
{ 0x38,   -1, 2, {O_U8, O_N}, {"fstore"}},
{ 0x39,   -1, 2, {O_U8, O_N}, {"dstore"}},
{ 0x3a,   -1, 2, {O_U8, O_N}, {"astore"}},
{ 0x3b,   -1, 1, {O_N, O_N}, {"istore_0"}},
{ 0x3c,   -1, 1, {O_N, O_N}, {"istore_1"}},
{ 0x3d,   -1, 1, {O_N, O_N}, {"istore_2"}},
{ 0x3e,   -1, 1, {O_N, O_N}, {"istore_3"}},
{ 0x3f,   -1, 1, {O_N, O_N}, {"lstore_0"}},
{ 0x40,   -1, 1, {O_N, O_N}, {"lstore_1"}},
{ 0x41,   -1, 1, {O_N, O_N}, {"lstore_2"}},
{ 0x42,   -1, 1, {O_N, O_N}, {"lstore_3"}},
{ 0x43,   -1, 1, {O_N, O_N}, {"fstore_0"}},
{ 0x44,   -1, 1, {O_N, O_N}, {"fstore_1"}},
{ 0x45,   -1, 1, {O_N, O_N}, {"fstore_2"}},
{ 0x46,   -1, 1, {O_N, O_N}, {"fstore_3"}},
{ 0x47,   -1, 1, {O_N, O_N}, {"dstore_0"}},
{ 0x48,   -1, 1, {O_N, O_N}, {"dstore_1"}},
{ 0x49,   -1, 1, {O_N, O_N}, {"dstore_2"}},
{ 0x4a,   -1, 1, {O_N, O_N}, {"dstore_3"}},
{ 0x4b,   -1, 1, {O_N, O_N}, {"astore_0"}},
{ 0x4c,   -1, 1, {O_N, O_N}, {"astore_1"}},
{ 0x4d,   -1, 1, {O_N, O_N}, {"astore_2"}},
{ 0x4e,   -1, 1, {O_N, O_N}, {"astore_3"}},
{ 0x4f,   -1, 1, {O_N, O_N}, {"iastore"}},
{ 0x50,   -1, 1, {O_N, O_N}, {"lastore"}},
{ 0x51,   -1, 1, {O_N, O_N}, {"fastore"}},
{ 0x52,   -1, 1, {O_N, O_N}, {"dastore"}},
{ 0x53,   -1, 1, {O_N, O_N}, {"aastore"}},
{ 0x54,   -1, 1, {O_N, O_N}, {"bastore"}},
{ 0x55,   -1, 1, {O_N, O_N}, {"castore"}},
{ 0x56,   -1, 1, {O_N, O_N}, {"sastore"}},
{ 0x57,   -1, 1, {O_N, O_N}, {"pop"}},
{ 0x58,   -1, 1, {O_N, O_N}, {"pop2"}},
{ 0x59,   -1, 1, {O_N, O_N}, {"dup"}},
{ 0x5a,   -1, 1, {O_N, O_N}, {"dup_x1"}},
{ 0x5b,   -1, 1, {O_N, O_N}, {"dup_x2"}},
{ 0x5c,   -1, 1, {O_N, O_N}, {"dup2"}},
{ 0x5d,   -1, 1, {O_N, O_N}, {"dup2_x1"}},
{ 0x5e,   -1, 1, {O_N, O_N}, {"dup2_x2"}},
{ 0x5f,   -1, 1, {O_N, O_N}, {"swap"}},
{ 0x60,   -1, 1, {O_N, O_N}, {"iadd"}},
{ 0x61,   -1, 1, {O_N, O_N}, {"ladd"}},
{ 0x62,   -1, 1, {O_N, O_N}, {"fadd"}},
{ 0x63,   -1, 1, {O_N, O_N}, {"dadd"}},
{ 0x64,   -1, 1, {O_N, O_N}, {"isub"}},
{ 0x65,   -1, 1, {O_N, O_N}, {"lsub"}},
{ 0x66,   -1, 1, {O_N, O_N}, {"fsub"}},
{ 0x67,   -1, 1, {O_N, O_N}, {"dsub"}},
{ 0x68,   -1, 1, {O_N, O_N}, {"imul"}},
{ 0x69,   -1, 1, {O_N, O_N}, {"lmul"}},
{ 0x6a,   -1, 1, {O_N, O_N}, {"fmul"}},
{ 0x6b,   -1, 1, {O_N, O_N}, {"dmul"}},
{ 0x6c,   -1, 1, {O_N, O_N}, {"idiv"}},
{ 0x6d,   -1, 1, {O_N, O_N}, {"ldiv"}},
{ 0x6e,   -1, 1, {O_N, O_N}, {"fdiv"}},
{ 0x6f,   -1, 1, {O_N, O_N}, {"ddiv"}},
{ 0x70,   -1, 1, {O_N, O_N}, {"irem"}},
{ 0x71,   -1, 1, {O_N, O_N}, {"lrem"}},
{ 0x72,   -1, 1, {O_N, O_N}, {"frem"}},
{ 0x73,   -1, 1, {O_N, O_N}, {"drem"}},
{ 0x74,   -1, 1, {O_N, O_N}, {"ineg"}},
{ 0x75,   -1, 1, {O_N, O_N}, {"lneg"}},
{ 0x76,   -1, 1, {O_N, O_N}, {"fneg"}},
{ 0x77,   -1, 1, {O_N, O_N}, {"dneg"}},
{ 0x78,   -1, 1, {O_N, O_N}, {"ishl"}},
{ 0x79,   -1, 1, {O_N, O_N}, {"lshl"}},
{ 0x7a,   -1, 1, {O_N, O_N}, {"ishr"}},
{ 0x7b,   -1, 1, {O_N, O_N}, {"lshr"}},
{ 0x7c,   -1, 1, {O_N, O_N}, {"iushr"}},
{ 0x7d,   -1, 1, {O_N, O_N}, {"lushr"}},
{ 0x7e,   -1, 1, {O_N, O_N}, {"iand"}},
{ 0x7f,   -1, 1, {O_N, O_N}, {"land"}},
{ 0x80,   -1, 1, {O_N, O_N}, {"ior"}},
{ 0x81,   -1, 1, {O_N, O_N}, {"lor"}},
{ 0x82,   -1, 1, {O_N, O_N}, {"ixor"}},
{ 0x83,   -1, 1, {O_N, O_N}, {"lxor"}},
{ 0x84,   -1, 3, {O_U8, O_8}, {"iinc"}},
{ 0x85,   -1, 1, {O_N, O_N}, {"i2l"}},
{ 0x86,   -1, 1, {O_N, O_N}, {"i2f"}},
{ 0x87,   -1, 1, {O_N, O_N}, {"i2d"}},
{ 0x88,   -1, 1, {O_N, O_N}, {"l2i"}},
{ 0x89,   -1, 1, {O_N, O_N}, {"l2f"}},
{ 0x8a,   -1, 1, {O_N, O_N}, {"l2d"}},
{ 0x8b,   -1, 1, {O_N, O_N}, {"f2i"}},
{ 0x8c,   -1, 1, {O_N, O_N}, {"f2l"}},
{ 0x8d,   -1, 1, {O_N, O_N}, {"f2d"}},
{ 0x8e,   -1, 1, {O_N, O_N}, {"d2i"}},
{ 0x8f,   -1, 1, {O_N, O_N}, {"d2l"}},
{ 0x90,   -1, 1, {O_N, O_N}, {"d2f"}},
{ 0x91,   -1, 1, {O_N, O_N}, {"i2b"}},
{ 0x92,   -1, 1, {O_N, O_N}, {"i2c"}},
{ 0x93,   -1, 1, {O_N, O_N}, {"i2s"}},
{ 0x94,   -1, 1, {O_N, O_N}, {"lcmp"}},
{ 0x95,   -1, 1, {O_N, O_N}, {"fcmpl"}},
{ 0x96,   -1, 1, {O_N, O_N}, {"fcmpg"}},
{ 0x97,   -1, 1, {O_N, O_N}, {"dcmpl"}},
{ 0x98,   -1, 1, {O_N, O_N}, {"dcmpg"}},
{ 0x99,   -1, 3, {O_R16, O_N}, {"ifeq"}},
{ 0x9a,   -1, 3, {O_R16, O_N}, {"ifne"}},
{ 0x9b,   -1, 3, {O_R16, O_N}, {"iflt"}},
{ 0x9c,   -1, 3, {O_R16, O_N}, {"ifge"}},
{ 0x9d,   -1, 3, {O_R16, O_N}, {"ifgt"}},
{ 0x9e,   -1, 3, {O_R16, O_N}, {"ifle"}},
{ 0x9f,   -1, 3, {O_R16, O_N}, {"if_icmpeq"}},
{ 0xa0,   -1, 3, {O_R16, O_N}, {"if_icmpne"}},
{ 0xa1,   -1, 3, {O_R16, O_N}, {"if_icmplt"}},
{ 0xa2,   -1, 3, {O_R16, O_N}, {"if_icmpge"}},
{ 0xa3,   -1, 3, {O_R16, O_N}, {"if_icmpgt"}},
{ 0xa4,   -1, 3, {O_R16, O_N}, {"if_icmple"}},
{ 0xa5,   -1, 3, {O_R16, O_N}, {"if_acmpeq"}},
{ 0xa6,   -1, 3, {O_R16, O_N}, {"if_acmpne"}},
{ 0xa7,   -1, 3, {O_R16, O_N}, {"goto"}},
{ 0xa8,   -1, 3, {O_N, O_N}, {"jsr"}},
{ 0xa9,   -1, 2, {O_N, O_N}, {"ret"}},
{ 0xaa,   -1, 1, {O_N, O_N}, {"tableswitch"}},
{ 0xab,   -1, 1, {O_N, O_N}, {"lookupswitch"}},
{ 0xac,   -1, 1, {O_N, O_N}, {"ireturn"}},
{ 0xad,   -1, 1, {O_N, O_N}, {"lreturn"}},
{ 0xae,   -1, 1, {O_N, O_N}, {"freturn"}},
{ 0xaf,   -1, 1, {O_N, O_N}, {"dreturn"}},
{ 0xb0,   -1, 1, {O_N, O_N}, {"areturn"}},
{ 0xb1,   -1, 1, {O_N, O_N}, {"return"}},
{ 0xb2,   -1, 3, {O_N, O_N}, {"getstatic"}},
{ 0xb3,   -1, 3, {O_N, O_N}, {"putstatic"}},
{ 0xb4,   -1, 3, {O_N, O_N}, {"getfield"}},
{ 0xb5,   -1, 3, {O_N, O_N}, {"putfield"}},
{ 0xb6,   -1, 3, {O_N, O_N}, {"invokevirtual"}},
{ 0xb7,   -1, 3, {O_N, O_N}, {"invokespecial"}},
{ 0xb8,   -1, 3, {O_N, O_N}, {"invokestatic"}},
{ 0xb9,   -1, 5, {O_N, O_N}, {"invokeinterface"}},
{ 0xba,   -1, 1, {O_N, O_N}, {"bad_ba"}},
{ 0xbb,   -1, 3, {O_N, O_N}, {"new"}},
{ 0xbc,   -1, 2, {O_N, O_N}, {"newarray"}},
{ 0xbd,   -1, 3, {O_N, O_N}, {"anewarray"}},
{ 0xbe,   -1, 1, {O_N, O_N}, {"arraylength"}},
{ 0xbf,   -1, 1, {O_N, O_N}, {"athrow"}},
{ 0xc0,   -1, 3, {O_N, O_N}, {"checkcast"}},
{ 0xc1,   -1, 3, {O_N, O_N}, {"instanceof"}},
{ 0xc2,   -1, 1, {O_N, O_N}, {"monitorenter"}},
{ 0xc3,   -1, 1, {O_N, O_N}, {"monitorexit"}},
{ 0xc4,   -1, 1, {O_N, O_N}, {"wide"}},
{ 0xc5,   -1, 4, {O_N, O_N}, {"multianewarray"}},
{ 0xc6,   -1, 3, {O_N, O_N}, {"ifnull"}},
{ 0xc7,   -1, 3, {O_N, O_N}, {"ifnonnull"}},
{ 0xc8,   -1, 5, {O_N, O_N}, {"goto_w"}},
{ 0xc9,   -1, 5, {O_N, O_N}, {"jsr_w"}},
{ 0xca,   -1, 1, {O_N, O_N}, {"breakpoint"}},
{ 0xcb,   -1, 1, {O_N, O_N}, {"bytecode"}},
{ 0xcc,   -1, 1, {O_N, O_N}, {"try"}},
{ 0xcd,   -1, 1, {O_N, O_N}, {"endtry"}},
{ 0xce,   -1, 1, {O_N, O_N}, {"catch"}},
{ 0xcf,   -1, 1, {O_N, O_N}, {"var"}},
{ 0xd0,   -1, 1, {O_N, O_N}, {"endvar"}},
{ 0xd1,   -1, 1, {O_N, O_N}, {"bad_d1"}},
{ 0xd2,   -1, 1, {O_N, O_N}, {"bad_d2"}},
{ 0xd3,   -1, 1, {O_N, O_N}, {"bad_d3"}},
{ 0xd4,   -1, 1, {O_N, O_N}, {"bad_d4"}},
{ 0xd5,   -1, 1, {O_N, O_N}, {"bad_d5"}},
{ 0xd6,   -1, 1, {O_N, O_N}, {"bad_d6"}},
{ 0xd7,   -1, 1, {O_N, O_N}, {"bad_d7"}},
{ 0xd8,   -1, 1, {O_N, O_N}, {"bad_d8"}},
{ 0xd9,   -1, 1, {O_N, O_N}, {"bad_d9"}},
{ 0xda,   -1, 1, {O_N, O_N}, {"bad_da"}},
{ 0xdb,   -1, 1, {O_N, O_N}, {"bad_db"}},
{ 0xdc,   -1, 1, {O_N, O_N}, {"bad_dc"}},
{ 0xdd,   -1, 1, {O_N, O_N}, {"bad_dd"}},
{ 0xde,   -1, 1, {O_N, O_N}, {"bad_de"}},
{ 0xdf,   -1, 1, {O_N, O_N}, {"bad_df"}},
{ 0xe0,   -1, 1, {O_N, O_N}, {"bad_e0"}},
{ 0xe1,   -1, 1, {O_N, O_N}, {"bad_e1"}},
{ 0xe2,   -1, 1, {O_N, O_N}, {"bad_e2"}},
{ 0xe3,   -1, 1, {O_N, O_N}, {"bad_e3"}},
{ 0xe4,   -1, 1, {O_N, O_N}, {"bad_e4"}},
{ 0xe5,   -1, 1, {O_N, O_N}, {"bad_e5"}},
{ 0xe6,   -1, 1, {O_N, O_N}, {"bad_e6"}},
{ 0xe7,   -1, 1, {O_N, O_N}, {"bad_e7"}},
{ 0xe8,   -1, 1, {O_N, O_N}, {"bad_e8"}},
{ 0xe9,   -1, 1, {O_N, O_N}, {"bad_e9"}},
{ 0xea,   -1, 1, {O_N, O_N}, {"bad_ea"}},
{ 0xeb,   -1, 1, {O_N, O_N}, {"bad_eb"}},
{ 0xec,   -1, 1, {O_N, O_N}, {"bad_ec"}},
{ 0xed,   -1, 3, {O_16, O_N}, {"sethi"}},
{ 0xee,   -1, 3, {O_U8, O_8}, {"load_word_index"}},
{ 0xef,   -1, 3, {O_U8, O_8}, {"load_short_index"}},
{ 0xf0,   -1, 3, {O_U8, O_8}, {"load_char_index"}},
{ 0xf1,   -1, 3, {O_U8, O_8}, {"load_byte_index"}},
{ 0xf2,   -1, 3, {O_U8, O_8}, {"load_ubyte_index"}},
{ 0xf3,   -1, 3, {O_U8, O_8}, {"store_word_index"}},
{ 0xf4,   -1, 3, {O_U8, O_8}, {"na_store_word_index"}},
{ 0xf5,   -1, 3, {O_U8, O_8}, {"store_short_index"}},
{ 0xf6,   -1, 3, {O_U8, O_8}, {"store_byte_index"}},
{ 0xf7,   -1, 1, {O_N, O_N}, {"bad_f7"}},
{ 0xf8,   -1, 1, {O_N, O_N}, {"bad_f8"}},
{ 0xf9,   -1, 1, {O_N, O_N}, {"bad_f9"}},
{ 0xfa,   -1, 1, {O_N, O_N}, {"bad_fa"}},
{ 0xfb,   -1, 1, {O_N, O_N}, {"bad_fb"}},
{ 0xfc,   -1, 1, {O_N, O_N}, {"bad_fc"}},
{ 0xfd,   -1, 1, {O_N, O_N}, {"bad_fd"}},
{ 0xfe,   -1, 1, {O_N, O_N}, {"bad_fe"}},
{ 0xff, 0x00, 2, {O_N, O_N}, {"load_ubyte"}},
{ 0xff, 0x01, 2, {O_N, O_N}, {"load_byte"}},
{ 0xff, 0x02, 2, {O_N, O_N}, {"load_char"}},
{ 0xff, 0x03, 2, {O_N, O_N}, {"load_short"}},
{ 0xff, 0x04, 2, {O_N, O_N}, {"load_word"}},
{ 0xff, 0x05, 2, {O_N, O_N}, {"priv_ret_from_trap"}},
{ 0xff, 0x06, 2, {O_N, O_N}, {"priv_read_dcache_tag"}},
{ 0xff, 0x07, 2, {O_N, O_N}, {"priv_read_dcache_data"}},
{ 0xff, 0x08, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x09, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x0a, 2, {O_N, O_N}, {"load_char_oe"}},
{ 0xff, 0x0b, 2, {O_N, O_N}, {"load_short_oe"}},
{ 0xff, 0x0c, 2, {O_N, O_N}, {"load_word_oe"}},
{ 0xff, 0x0d, 2, {O_N, O_N}, {"return0"}},
{ 0xff, 0x0e, 2, {O_N, O_N}, {"priv_read_icache_tag"}},
{ 0xff, 0x0f, 2, {O_N, O_N}, {"priv_read_icache_data"}},
{ 0xff, 0x10, 2, {O_N, O_N}, {"ncload_ubyte"}},
{ 0xff, 0x11, 2, {O_N, O_N}, {"ncload_byte"}},
{ 0xff, 0x12, 2, {O_N, O_N}, {"ncload_char"}},
{ 0xff, 0x13, 2, {O_N, O_N}, {"ncload_short"}},
{ 0xff, 0x14, 2, {O_N, O_N}, {"ncload_word"}},
{ 0xff, 0x15, 2, {O_N, O_N}, {"iucmp"}},
{ 0xff, 0x16, 2, {O_N, O_N}, {"priv_powerdown"}},
{ 0xff, 0x17, 2, {O_N, O_N}, {"cache_invalidate"}},
{ 0xff, 0x18, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x19, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x1a, 2, {O_N, O_N}, {"ncload_char_oe"}},
{ 0xff, 0x1b, 2, {O_N, O_N}, {"ncload_short_oe"}},
{ 0xff, 0x1c, 2, {O_N, O_N}, {"ncload_word_oe"}},
{ 0xff, 0x1d, 2, {O_N, O_N}, {"return1"}},
{ 0xff, 0x1e, 2, {O_N, O_N}, {"cache_flush"}},
{ 0xff, 0x1f, 2, {O_N, O_N}, {"cache_index_flush"}},
{ 0xff, 0x20, 2, {O_N, O_N}, {"store_byte"}},
{ 0xff, 0x21, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x22, 2, {O_N, O_N}, {"store_short"}},
{ 0xff, 0x23, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x24, 2, {O_N, O_N}, {"store_word"}},
{ 0xff, 0x25, 2, {O_N, O_N}, {"soft_trap"}},
{ 0xff, 0x26, 2, {O_N, O_N}, {"priv_write_dcache_tag"}},
{ 0xff, 0x27, 2, {O_N, O_N}, {"priv_write_dcache_data"}},
{ 0xff, 0x28, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x29, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x2a, 2, {O_N, O_N}, {"store_short_oe"}},
{ 0xff, 0x2b, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x2c, 2, {O_N, O_N}, {"store_word_oe"}},
{ 0xff, 0x2d, 2, {O_N, O_N}, {"return2"}},
{ 0xff, 0x2e, 2, {O_N, O_N}, {"priv_write_icache_tag"}},
{ 0xff, 0x2f, 2, {O_N, O_N}, {"priv_write_icache_data"}},
{ 0xff, 0x30, 2, {O_N, O_N}, {"ncstore_byte"}},
{ 0xff, 0x31, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x32, 2, {O_N, O_N}, {"ncstore_short"}},
{ 0xff, 0x33, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x34, 2, {O_N, O_N}, {"ncstore_word"}},
{ 0xff, 0x35, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x36, 2, {O_N, O_N}, {"priv_reset"}},
{ 0xff, 0x37, 2, {O_N, O_N}, {"get_current_class"}},
{ 0xff, 0x38, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x39, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x3a, 2, {O_N, O_N}, {"ncstore_short_oe"}},
{ 0xff, 0x3b, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x3c, 2, {O_N, O_N}, {"ncstore_word_oe"}},
{ 0xff, 0x3d, 2, {O_N, O_N}, {"call"}},
{ 0xff, 0x3e, 2, {O_N, O_N}, {"zero_line"}},
{ 0xff, 0x3f, 2, {O_N, O_N}, {"priv_update_optop"}},
{ 0xff, 0x40, 2, {O_N, O_N}, {"read_pc"}},
{ 0xff, 0x41, 2, {O_N, O_N}, {"read_vars"}},
{ 0xff, 0x42, 2, {O_N, O_N}, {"read_frame"}},
{ 0xff, 0x43, 2, {O_N, O_N}, {"read_optop"}},
{ 0xff, 0x44, 2, {O_N, O_N}, {"priv_read_oplim"}},
{ 0xff, 0x45, 2, {O_N, O_N}, {"read_const_pool"}},
{ 0xff, 0x46, 2, {O_N, O_N}, {"priv_read_psr"}},
{ 0xff, 0x47, 2, {O_N, O_N}, {"priv_read_trapbase"}},
{ 0xff, 0x48, 2, {O_N, O_N}, {"priv_read_lockcount0"}},
{ 0xff, 0x49, 2, {O_N, O_N}, {"priv_read_lockcount1"}},
{ 0xff, 0x4a, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x4b, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x4c, 2, {O_N, O_N}, {"priv_read_lockaddr0"}},
{ 0xff, 0x4d, 2, {O_N, O_N}, {"priv_read_lockaddr1"}},
{ 0xff, 0x4e, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x4f, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x50, 2, {O_N, O_N}, {"priv_read_userrange1"}},
{ 0xff, 0x51, 2, {O_N, O_N}, {"priv_read_gc_config"}},
{ 0xff, 0x52, 2, {O_N, O_N}, {"priv_read_brk1a"}},
{ 0xff, 0x53, 2, {O_N, O_N}, {"priv_read_brk2a"}},
{ 0xff, 0x54, 2, {O_N, O_N}, {"priv_read_brk12c"}},
{ 0xff, 0x55, 2, {O_N, O_N}, {"priv_read_userrange2"}},
{ 0xff, 0x56, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x57, 2, {O_N, O_N}, {"priv_read_versionid"}},
{ 0xff, 0x58, 2, {O_N, O_N}, {"priv_read_hcr"}},
{ 0xff, 0x59, 2, {O_N, O_N}, {"priv_read_sc_bottom"}},
{ 0xff, 0x5a, 2, {O_N, O_N}, {"read_global0"}},
{ 0xff, 0x5b, 2, {O_N, O_N}, {"read_global1"}},
{ 0xff, 0x5c, 2, {O_N, O_N}, {"read_global2"}},
{ 0xff, 0x5d, 2, {O_N, O_N}, {"read_global3"}},
{ 0xff, 0x5e, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x5f, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x60, 2, {O_N, O_N}, {"write_pc"}},
{ 0xff, 0x61, 2, {O_N, O_N}, {"write_vars"}},
{ 0xff, 0x62, 2, {O_N, O_N}, {"write_frame"}},
{ 0xff, 0x63, 2, {O_N, O_N}, {"write_optop"}},
{ 0xff, 0x64, 2, {O_N, O_N}, {"priv_write_oplim"}},
{ 0xff, 0x65, 2, {O_N, O_N}, {"write_const_pool"}},
{ 0xff, 0x66, 2, {O_N, O_N}, {"priv_write_psr"}},
{ 0xff, 0x67, 2, {O_N, O_N}, {"priv_write_trapbase"}},
{ 0xff, 0x68, 2, {O_N, O_N}, {"priv_write_lockcount0"}},
{ 0xff, 0x69, 2, {O_N, O_N}, {"priv_write_lockcount1"}},
{ 0xff, 0x6a, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x6b, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x6c, 2, {O_N, O_N}, {"priv_write_lockaddr0"}},
{ 0xff, 0x6d, 2, {O_N, O_N}, {"priv_write_lockaddr1"}},
{ 0xff, 0x6e, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x6f, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x70, 2, {O_N, O_N}, {"priv_write_userrange1"}},
{ 0xff, 0x71, 2, {O_N, O_N}, {"priv_write_gc_config"}},
{ 0xff, 0x72, 2, {O_N, O_N}, {"priv_write_brk1a"}},
{ 0xff, 0x73, 2, {O_N, O_N}, {"priv_write_brk2a"}},
{ 0xff, 0x74, 2, {O_N, O_N}, {"priv_write_brk12c"}},
{ 0xff, 0x75, 2, {O_N, O_N}, {"priv_write_userrange2"}},
{ 0xff, 0x76, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x77, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x78, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x79, 2, {O_N, O_N}, {"priv_write_sc_bottom"}},
{ 0xff, 0x7a, 2, {O_N, O_N}, {"write_global0"}},
{ 0xff, 0x7b, 2, {O_N, O_N}, {"write_global1"}},
{ 0xff, 0x7c, 2, {O_N, O_N}, {"write_global2"}},
{ 0xff, 0x7d, 2, {O_N, O_N}, {"write_global3"}},
{ 0xff, 0x7e, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x7f, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x80, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x81, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x82, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x83, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x84, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x85, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x86, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x87, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x88, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x89, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x8a, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x8b, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x8c, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x8d, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x8e, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x8f, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x90, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x91, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x92, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x93, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x94, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x95, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x96, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x97, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x98, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x99, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x9a, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x9b, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x9c, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x9d, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x9e, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0x9f, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa0, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa1, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa2, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa3, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa4, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa5, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa6, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa7, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa8, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xa9, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xaa, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xab, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xac, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xad, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xae, 2, {O_N, O_N}, {"tm_putchar"}},
{ 0xff, 0xaf, 2, {O_N, O_N}, {"tm_exit"}},
{ 0xff, 0xb0, 2, {O_N, O_N}, {"tm_trap"}},
{ 0xff, 0xb1, 2, {O_N, O_N}, {"tm_minfo"}},
{ 0xff, 0xb2, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xb3, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xb4, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xb5, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xb6, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xb7, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xb8, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xb9, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xba, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xbb, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xbc, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xbd, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xbe, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xbf, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc0, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc1, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc2, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc3, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc4, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc5, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc6, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc7, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc8, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xc9, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xca, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xcb, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xcc, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xcd, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xce, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xcf, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd0, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd1, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd2, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd3, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd4, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd5, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd6, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd7, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd8, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xd9, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xda, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xdb, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xdc, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xdd, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xde, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xdf, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe0, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe1, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe2, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe3, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe4, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe5, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe6, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe7, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe8, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xe9, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xea, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xeb, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xec, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xed, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xee, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xef, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf0, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf1, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf2, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf3, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf4, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf5, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf6, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf7, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf8, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xf9, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xfa, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xfb, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xfc, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xfd, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xfe, 2, {O_N, O_N}, {"bad"}},
{ 0xff, 0xff, 2, {O_N, O_N}, {"bad"}},
};
