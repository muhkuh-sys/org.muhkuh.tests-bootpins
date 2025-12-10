#include "netx_io_areas.h"

#include "detect.h"


void get_values(BOOTPINS_PARAMETER_T *ptTestParams)
{
#if ASIC_TYP==ASIC_TYP_NETX10
	HOSTDEF(ptAsicCtrlArea);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;
	unsigned long ulStrappingOptions;


	/* Get the current boot mode. */
	ulValue = ptAsicCtrlArea->ulRdy_run_cfg;
	ulRDY = (ulValue & HOSTMSK(rdy_run_cfg_RDY_IN)) >> HOSTSRT(rdy_run_cfg_RDY_IN);
	ulRUN = (ulValue & HOSTMSK(rdy_run_cfg_RUN_IN)) >> HOSTSRT(rdy_run_cfg_RUN_IN);
	ulValue  = ulRDY << SRT_BOOTMODE_RDY;
	ulValue |= ulRUN << SRT_BOOTMODE_RUN;
	ptTestParams->ulBootMode = ulValue;

	/* Get the netX10 strapping options.
	 * On netX10 the bits are read from these pins:
	 *   0: SPI0_SIO3
	 *   1: SPI0_SIO2
	 *   2: SPI0_MOSI
	 *   3: SPI0_CLK
	 */
	ulValue = ptAsicCtrlArea->ulSample_at_nres;
	ulStrappingOptions  = (ulValue&HOSTMSK(sample_at_nres_sar_spi0_sio3))>> HOSTSRT(sample_at_nres_sar_spi0_sio3);
	ulStrappingOptions |= (ulValue&HOSTMSK(sample_at_nres_sar_spi0_sio2))>>(HOSTSRT(sample_at_nres_sar_spi0_sio2)-1);
	ulStrappingOptions |= (ulValue&HOSTMSK(sample_at_nres_sar_spi0_mosi))>>(HOSTSRT(sample_at_nres_sar_spi0_mosi)-2);
	ulStrappingOptions |= (ulValue&HOSTMSK(sample_at_nres_sar_spi0_clk)) >>(HOSTSRT(sample_at_nres_sar_spi0_clk) -3);
	ptTestParams->ulStrappingOptions = ulStrappingOptions;

	/* The netX10 has only one chip ID. */
	ptTestParams->ulChipID = CHIPID_netX10;

	/* The netX10 has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));
#else
#       error "Unknown ASIC_TYP!"
#endif
}
