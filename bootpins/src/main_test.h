
#ifndef __MAIN_TEST_H__
#define __MAIN_TEST_H__

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
	CHIPID_netX52B                              = 10
} CHIPID_T;


#define MSK_BOOTMODE_RDY 0x00000001U
#define SRT_BOOTMODE_RDY 0
#define MSK_BOOTMODE_RUN 0x00000002U
#define SRT_BOOTMODE_RUN 1


typedef struct BOOTPINS_PARAMETER_STRUCT
{
	unsigned long ulBootMode;
	unsigned long ulStrappingOptions;
	unsigned long ulChipID;
} BOOTPINS_PARAMETER_T;


#endif  /* __MAIN_TEST_H__ */

