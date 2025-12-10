#include "netx_io_areas.h"

#include "detect.h"

#include <string.h>


void get_values(BOOTPINS_PARAMETER_T *ptTestParams)
{
#if ASIC_TYP==ASIC_TYP_NETX56
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptSqiArea);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;
	unsigned long ulStrappingOptions;
	unsigned long ulChipSubType;
	unsigned long *pulId;
	CHIPID_T tChipID;


	/* Get the current boot mode. */
	ulValue = ptAsicCtrlArea->ulRdy_run_cfg;
	ulRDY = (ulValue & HOSTMSK(rdy_run_cfg_RDY_IN)) >> HOSTSRT(rdy_run_cfg_RDY_IN);
	ulRUN = (ulValue & HOSTMSK(rdy_run_cfg_RUN_IN)) >> HOSTSRT(rdy_run_cfg_RUN_IN);
	ulValue  = ulRDY << SRT_BOOTMODE_RDY;
	ulValue |= ulRUN << SRT_BOOTMODE_RUN;
	ptTestParams->ulBootMode = ulValue;

	/* Get the netX56 strapping options. */
	ulValue = ptSqiArea->ulSqi_pio_in;
	ulStrappingOptions  = (ulValue&HOSTMSK(sqi_pio_in_sclk))>> HOSTSRT(sqi_pio_in_sclk);
	ulStrappingOptions |= (ulValue&HOSTMSK(sqi_pio_in_mosi))>>(HOSTSRT(sqi_pio_in_mosi)-1);
	ulStrappingOptions |= (ulValue&HOSTMSK(sqi_pio_in_miso))>>(HOSTSRT(sqi_pio_in_miso)-2);
	ptTestParams->ulStrappingOptions = ulStrappingOptions;

	/* The netX500/100 has no chip ID. */
	/* Get the chip subtype from mem_a18 and mem_a19:
	 *  18 19
	 *   0  0  netX50
	 *   1  0  netX51
	 *   0  1  netX52
	 *   1  1  reserved
	 */
	ulValue = ptAsicCtrlArea->ulSample_at_nres;
	ulChipSubType  = (ulValue&HOSTMSK(sample_at_nres_sar_mem_a18))>> HOSTSRT(sample_at_nres_sar_mem_a18);
	ulChipSubType |= (ulValue&HOSTMSK(sample_at_nres_sar_mem_a19))>>(HOSTSRT(sample_at_nres_sar_mem_a19)-1);

	/* Detect netX500 or netX100. */
	pulId = (unsigned long*)0x080f0008U;
	ulValue = *pulId;
	tChipID = CHIPID_unknown;
	if( ulValue==0x00006003 )
	{
		switch( ulChipSubType )
		{
		case 0:
			tChipID = CHIPID_netX51A_netX50_compatibility_mode;
			break;
		case 1:
			tChipID = CHIPID_netX51A;
			break;
		case 2:
			tChipID = CHIPID_netX52A;
		}
	}
	else if( ulValue==0x00106003 )
	{
		switch( ulChipSubType )
		{
		case 0:
			tChipID = CHIPID_netX51B_netX50_compatibility_mode;
			break;
		case 1:
			tChipID = CHIPID_netX51B;
			break;
		case 2:
			tChipID = CHIPID_netX52B;
		}
	}
	ptTestParams->ulChipID = tChipID;

	/* The netX56 has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));

#else
#       error "Unknown ASIC_TYP!"
#endif

}
