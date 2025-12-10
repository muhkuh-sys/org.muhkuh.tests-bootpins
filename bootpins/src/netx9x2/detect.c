#include "netx_io_areas.h"

#include "detect.h"

#include <string.h>


void get_values(BOOTPINS_PARAMETER_T *ptTestParams)
{
#if ASIC_TYP==ASIC_TYP_NETX9X2_COM_MPW
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptGlobalAsicCtrlArea);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;
	CHIPID_T tChipID;


	/* Get the current boot mode. */
	ulValue = ptAsicCtrlArea->ulCom_asic_ctrl_rdy_run_cfg;
	ulRDY = (ulValue & HOSTMSK(com_asic_ctrl_rdy_run_cfg_RDY_IN)) >> HOSTSRT(com_asic_ctrl_rdy_run_cfg_RDY_IN);
	ulRUN = (ulValue & HOSTMSK(com_asic_ctrl_rdy_run_cfg_RUN_IN)) >> HOSTSRT(com_asic_ctrl_rdy_run_cfg_RUN_IN);
	ulValue  = ulRDY << SRT_BOOTMODE_RDY;
	ulValue |= ulRUN << SRT_BOOTMODE_RUN;
	ptTestParams->ulBootMode = ulValue;

	/* The netX9X2 has no strapping options. */
	ptTestParams->ulStrappingOptions = 0U;

	/* For now there is only the netX9x2 MPW. */
	tChipID = CHIPID_netX9x2_MPW;
	ptTestParams->ulChipID = tChipID;

	/* Copy the unique ID. */
	ptTestParams->sizUniqueIdInBits = sizeof(ptGlobalAsicCtrlArea->aulGlobal_asic_ctrl_netx_unique_id) * 8U;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));
	ptTestParams->tUniqueID.aul[0] = ptGlobalAsicCtrlArea->aulGlobal_asic_ctrl_netx_unique_id[0];
	ptTestParams->tUniqueID.aul[1] = ptGlobalAsicCtrlArea->aulGlobal_asic_ctrl_netx_unique_id[1];
	ptTestParams->tUniqueID.aul[2] = ptGlobalAsicCtrlArea->aulGlobal_asic_ctrl_netx_unique_id[2];
#else
#       error "Unknown ASIC_TYP!"
#endif
}
