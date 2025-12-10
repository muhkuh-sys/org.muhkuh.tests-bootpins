#ifndef __DETECT_H__
#define __DETECT_H__


typedef enum CHIPID_ENUM
{
	CHIPID_unknown                              =  0,
	CHIPID_netX500                              =  1,
	CHIPID_netX100                              =  2,
	CHIPID_netX50                               =  3,
	CHIPID_netX10                               =  4,
	CHIPID_netX51A_netX50_compatibility_mode    =  5,
	CHIPID_netX51B_netX50_compatibility_mode    =  6,
	CHIPID_netX51A                              =  7,
	CHIPID_netX51B                              =  8,
	CHIPID_netX52A                              =  9,
	CHIPID_netX52B                              = 10,
	CHIPID_netX4000_RELAXED                     = 11,
	CHIPID_netX4000_FULL                        = 12,
	CHIPID_netX4000_SMALL                       = 13,
	CHIPID_netX90_MPW                           = 14,
	CHIPID_netX90                               = 15,
	CHIPID_netX90B                              = 16,
	CHIPID_netX90BPhyR3                         = 17,
	CHIPID_netX90C                              = 18,
	CHIPID_netX90BPhyR2or3                      = 19,
	CHIPID_netX9x2_MPW                          = 20
} CHIPID_T;


#define MSK_BOOTMODE_RDY 0x00000001U
#define SRT_BOOTMODE_RDY 0
#define MSK_BOOTMODE_RUN 0x00000002U
#define SRT_BOOTMODE_RUN 1


typedef union UNIQUE_ID_UNION
{
	unsigned char auc[16];
	unsigned long aul[16/sizeof(unsigned long)];
} UNIQUE_ID_T;

#define BOOTPINS_FLAGS_PHY_IS_CRITICAL 0x00000001

typedef struct BOOTPINS_PARAMETER_STRUCT
{
	unsigned long ulFlags;
	unsigned long ulBootMode;
	unsigned long ulStrappingOptions;
	unsigned long ulChipID;
	unsigned int  sizUniqueIdInBits;
	UNIQUE_ID_T tUniqueID;
} BOOTPINS_PARAMETER_T;


void get_values(BOOTPINS_PARAMETER_T *ptTestParams);


#endif  /* __DETECT_H__ */
