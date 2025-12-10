#include "netx_io_areas.h"

#include "detect.h"

#include <string.h>


//-------------------------------------
// some defines for the mysterious HIF registers
// taken from the "netX Program Reference Guide Rev0.3", page 16

#define MSK_sta_netx_rdy                        0x00000001U
#define SRT_sta_netx_rdy                        0
#define MSK_sta_netx_run                        0x00000002U
#define SRT_sta_netx_run                        1
#define MSK_sta_netx_netx_state                 0x0000000cU
#define SRT_sta_netx_netx_state                 2
#define MSK_sta_netx_host_state                 0x000000f0U
#define SRT_sta_netx_host_state                 4
#define MSK_sta_netx_netx_sta_code              0x0000ff00U
#define SRT_sta_netx_netx_sta_code              8
#define MSK_sta_netx_rdy_in                     0x00010000U
#define SRT_sta_netx_rdy_in                     16
#define MSK_sta_netx_run_in                     0x00020000U
#define SRT_sta_netx_run_in                     17
#define MSK_sta_netx_rdy_pol                    0x00040000U
#define SRT_sta_netx_rdy_pol                    18
#define MSK_sta_netx_run_pol                    0x00080000U
#define SRT_sta_netx_run_pol                    19
#define MSK_sta_netx_rdy_drv                    0x01000000U
#define SRT_sta_netx_rdy_drv                    24
#define MSK_sta_netx_run_drv                    0x02000000U
#define SRT_sta_netx_run_drv                    25



void get_values(BOOTPINS_PARAMETER_T *ptTestParams)
{
#if ASIC_TYP==ASIC_TYP_NETX50
	HOSTDEF(ptNetxControlledGlobalRegisterBlock1Area);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;


	/* Get the current boot mode. */
	ulValue  = ptNetxControlledGlobalRegisterBlock1Area->ulSta_netx;
	ulRDY = (ulValue & MSK_sta_netx_rdy_in) >> SRT_sta_netx_rdy_in;
	ulRUN = (ulValue & MSK_sta_netx_run_in) >> SRT_sta_netx_run_in;
	ulValue  = ulRDY << SRT_BOOTMODE_RDY;
	ulValue |= ulRUN << SRT_BOOTMODE_RUN;
	ptTestParams->ulBootMode = ulValue;

	/* The netX50 has no strapping options. */
	ptTestParams->ulStrappingOptions = 0;

	/* The netX50 has only one chip ID. */
	ptTestParams->ulChipID = CHIPID_netX50;

	/* The netX50 has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));

#else
#       error "Unknown ASIC_TYP!"
#endif
}
