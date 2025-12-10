#include "netx_io_areas.h"

#include "detect.h"


void get_values(BOOTPINS_PARAMETER_T *ptTestParams)
{
#if ASIC_TYP==ASIC_TYP_NETX4000_RELAXED || ASIC_TYP==ASIC_TYP_NETX4000
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptRAPSysctrlArea);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;
	const unsigned long * const pulRomId = (const unsigned long * const)(0x04100020U);
	CHIPID_T tChipID;


	/* Get the current boot mode. */
	ulValue = ptAsicCtrlArea->ulRdy_run_cfg;
	ulRDY = (ulValue & HOSTMSK(rdy_run_cfg_RDY_IN)) >> HOSTSRT(rdy_run_cfg_RDY_IN);
	ulRUN = (ulValue & HOSTMSK(rdy_run_cfg_RUN_IN)) >> HOSTSRT(rdy_run_cfg_RUN_IN);
	ulValue  = ulRDY << SRT_BOOTMODE_RDY;
	ulValue |= ulRUN << SRT_BOOTMODE_RUN;
	ptTestParams->ulBootMode = ulValue;

	/* Get the strapping options. */
	ulValue   = ptRAPSysctrlArea->ulRAP_SYSCTRL_BOOTMODE;
	ulValue  &= HOSTMSK(RAP_SYSCTRL_BOOTMODE_BOOTMODE_CORE);
	ulValue >>= HOSTSRT(RAP_SYSCTRL_BOOTMODE_BOOTMODE_CORE);
	ptTestParams->ulStrappingOptions = ulValue;

	tChipID = CHIPID_unknown;
	/* Distinguish the RELAXED and FULL version. */
	ulValue = *pulRomId;
	if( ulValue==0x00108004U )
	{
		/* This is the RELAXED version. */
		tChipID = CHIPID_netX4000_RELAXED;
	}
	else if( ulValue==0x0010b004U )
	{
		/* This is the FULL version. It exists in 2 different packages.
		 *
		 * From the regdef:
		 *   Bit 0: Package selection
		 *     1 = netx_small I/O mode
		 *     0 = netx_full/Kontron I/O mode
		 */
		ulValue   = ptRAPSysctrlArea->aulRAP_SYSCTRL_OTP_CONFIG_[0];
		ulValue  &= HOSTMSK(RAP_SYSCTRL_OTP_CONFIG_0_MODE);
		ulValue >>= HOSTSRT(RAP_SYSCTRL_OTP_CONFIG_0_MODE);
		ulValue  &= 0x00000001U;
		if( ulValue!=0 )
		{
			/* This is the "small" package. */
			tChipID = CHIPID_netX4000_SMALL;
		}
		else
		{
			/* This is the "full" package. */
			tChipID = CHIPID_netX4000_FULL;
		}
	}
	ptTestParams->ulChipID = tChipID;

	/* Copy the unique ID. */
	ulValue = ptRAPSysctrlArea->aulRAP_SYSCTRL_OTP_CONFIG_[1];
	ptTestParams->sizUniqueIdInBits = 16;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));
	ptTestParams->tUniqueID.auc[0] = (unsigned char)((ulValue & MSK_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_122) >> SRT_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_122);
	ptTestParams->tUniqueID.auc[1] = (unsigned char)((ulValue & MSK_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_123) >> SRT_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_123);

#else
#       error "Unknown ASIC_TYP!"
#endif
}
