/*	$NetBSD: elf_machdep.h,v 1.1 2002/06/05 01:04:21 fredette Exp $	*/

#define	ELF32_MACHDEP_ENDIANNESS	ELFDATA2MSB
#define	ELF32_MACHDEP_ID_CASES						\
		case EM_PARISC:						\
			break;

#define	ELF64_MACHDEP_ENDIANNESS	XXX	/* break compilation */
#define	ELF64_MACHDEP_ID_CASES						\
		/* no 64-bit ELF machine types supported */

#define	ELF32_MACHDEP_ID	EM_PARISC

#define ARCH_ELFSIZE		32	/* MD native binary size */

/* hppa relocation types */
#define R_PARISC_NONE	         0 /* No reloc */
#define R_PARISC_DIR32	   	 1
#define R_PARISC_DIR21L	   	 2
#define R_PARISC_DIR17R	   	 3
#define R_PARISC_DIR17F	   	 4
#define R_PARISC_DIR14R	   	 6
#define R_PARISC_DIR14F	   	 7
#define R_PARISC_PCREL12F  	 8
#define R_PARISC_PCREL32   	 9
#define R_PARISC_PCREL21L  	10
#define R_PARISC_PCREL17R  	11
#define R_PARISC_PCREL17F  	12
#define R_PARISC_PCREL17C  	13
#define R_PARISC_PCREL14R  	14
#define R_PARISC_PCREL14F  	15
#define R_PARISC_DPREL21L  	18
#define R_PARISC_DPREL14WR 	19
#define R_PARISC_DPREL14DR 	20
#define R_PARISC_DPREL14R  	22
#define R_PARISC_DPREL14F  	23
#define R_PARISC_DLTREL21L     	26
#define R_PARISC_DLTREL14R     	30
#define R_PARISC_DLTREL14F     	31
#define R_PARISC_DLTIND21L     	34
#define R_PARISC_DLTIND14R     	38
#define R_PARISC_DLTIND14F     	39
#define R_PARISC_SETBASE       	40
#define R_PARISC_SECREL32      	41
#define R_PARISC_BASEREL21L    	42
#define R_PARISC_BASEREL17R    	43
#define R_PARISC_BASEREL17F    	44
#define R_PARISC_BASEREL14R    	46
#define R_PARISC_BASEREL14F    	47
#define R_PARISC_SEGBASE       	48
#define R_PARISC_SEGREL32      	49
#define R_PARISC_PLTOFF21L     	50
#define R_PARISC_PLTOFF14R     	54
#define R_PARISC_PLTOFF14F     	55
#define R_PARISC_LTOFF_FPTR32  	57
#define R_PARISC_LTOFF_FPTR21L 	58
#define R_PARISC_LTOFF_FPTR14R 	62
#define R_PARISC_FPTR64        	64
#define R_PARISC_PLABEL32      	65
#define R_PARISC_PLABEL21L     	66
#define R_PARISC_PLABEL14R     	70
#define R_PARISC_PCREL64       	72
#define R_PARISC_PCREL22C      	73
#define R_PARISC_PCREL22F      	74
#define R_PARISC_PCREL14WR     	75
#define R_PARISC_PCREL14DR     	76
#define R_PARISC_PCREL16F      	77
#define R_PARISC_PCREL16WF     	78
#define R_PARISC_PCREL16DF     	79
#define R_PARISC_DIR64         	80
#define R_PARISC_DIR64WR       	81
#define R_PARISC_DIR64DR       	82
#define R_PARISC_DIR14WR       	83
#define R_PARISC_DIR14DR       	84
#define R_PARISC_DIR16F        	85
#define R_PARISC_DIR16WF       	86
#define R_PARISC_DIR16DF       	87
#define R_PARISC_GPREL64       	88
#define R_PARISC_DLTREL14WR    	91
#define R_PARISC_DLTREL14DR    	92
#define R_PARISC_GPREL16F      	93
#define R_PARISC_GPREL16WF     	94
#define R_PARISC_GPREL16DF     	95
#define R_PARISC_LTOFF64      	96
#define R_PARISC_DLTIND14WR   	99
#define R_PARISC_DLTIND14DR     100
#define R_PARISC_LTOFF16F       101
#define R_PARISC_LTOFF16WF      102
#define R_PARISC_LTOFF16DF      103
#define R_PARISC_SECREL64       104
#define R_PARISC_BASEREL14WR    107
#define R_PARISC_BASEREL14DR    108
#define R_PARISC_SEGREL64       112
#define R_PARISC_PLTOFF14WR     115
#define R_PARISC_PLTOFF14DR     116    
#define R_PARISC_PLTOFF16F      117    
#define R_PARISC_PLTOFF16WF     118    
#define R_PARISC_PLTOFF16DF     119    
#define R_PARISC_LTOFF_FPTR64   120
#define R_PARISC_LTOFF_FPTR14WR 123
#define R_PARISC_LTOFF_FPTR14DR 124
#define R_PARISC_LTOFF_FPTR16F  125
#define R_PARISC_LTOFF_FPTR16WF 126
#define R_PARISC_LTOFF_FPTR16DF 127
#define R_PARISC_COPY 	       128
#define R_PARISC_IPLT 	       129
#define R_PARISC_EPLT 	       130
#define R_PARISC_TPREL32        153
#define R_PARISC_TPREL21L       154
#define R_PARISC_TPREL14R       158
#define R_PARISC_LTOFF_TP21L    162
#define R_PARISC_LTOFF_TP14R    166
#define R_PARISC_LTOFF_TP14F    167
#define R_PARISC_TPREL64        216
#define R_PARISC_TPREL14WR      219    	  
#define R_PARISC_TPREL14DR      220    	  
#define R_PARISC_TPREL16F       221    	  
#define R_PARISC_TPREL16WF      222    	  
#define R_PARISC_TPREL16DF      223    	  
#define R_PARISC_LTOFF_TP64     224
#define R_PARISC_LTOFF_TP14WR   227
#define R_PARISC_LTOFF_TP14DR   228
#define R_PARISC_LTOFF_TP16F    229
#define R_PARISC_LTOFF_TP16WF   230
#define R_PARISC_LTOFF_TP16DF   231
#define R_PARISC_GNU_VTENTRY    232
#define R_PARISC_GNU_VTINHERIT  233

#define	R_TYPE(name)	__CONCAT(R_PARISC_,name)
