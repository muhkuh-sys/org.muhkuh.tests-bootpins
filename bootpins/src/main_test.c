
#include "main_test.h"

#include "netx_test.h"
#include "rdy_run.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"

/*-----------------------------------*/

#if ASIC_TYP==ASIC_TYP_NETX500 || ASIC_TYP==ASIC_TYP_NETX50
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

#endif


static void get_values(BOOTPINS_PARAMETER_T *ptTestParams)
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


#elif ASIC_TYP==ASIC_TYP_NETX500
	HOSTDEF(ptNetxControlledGlobalRegisterBlock1Area);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;
	unsigned long *pulId;
	CHIPID_T tChipID;


	/* Get the current boot mode. */
	ulValue  = ptNetxControlledGlobalRegisterBlock1Area->ulSta_netx;
	ulRDY = (ulValue & MSK_sta_netx_rdy_in) >> SRT_sta_netx_rdy_in;
	ulRUN = (ulValue & MSK_sta_netx_run_in) >> SRT_sta_netx_run_in;
	ulValue  = ulRDY << SRT_BOOTMODE_RDY;
	ulValue |= ulRUN << SRT_BOOTMODE_RUN;
	ptTestParams->ulBootMode = ulValue;

	/* The netX500/100 has no strapping options. */
	ptTestParams->ulStrappingOptions = 0;

	/* Detect netX500 or netX100. */
	pulId = (unsigned long*)0x00200008U;
	ulValue = *pulId;
	tChipID = CHIPID_unknown;
	if( ulValue==0x00001000 )
	{
		tChipID = CHIPID_netX500;
	}
	else if( ulValue==0x00003002 )
	{
		tChipID = CHIPID_netX100;
	}
	ptTestParams->ulChipID = tChipID;


#elif ASIC_TYP==ASIC_TYP_NETX90_MPW
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptSampleAtPornStatArea);
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

	/* Get the strapping options. */
	ulValue = ptSampleAtPornStatArea->aulSample_at_porn_stat_in[1];
	ulStrappingOptions  = (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_mosi)) >>  HOSTSRT(sample_at_porn_stat_in1_sqi_mosi);
	ulStrappingOptions |= (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_miso)) >> (HOSTSRT(sample_at_porn_stat_in1_sqi_miso)-1);
	ulStrappingOptions |= (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_sio2)) >> (HOSTSRT(sample_at_porn_stat_in1_sqi_sio2)-2);
	ptTestParams->ulStrappingOptions = ulStrappingOptions;

	/* Set the chip ID. */
	ptTestParams->ulChipID = CHIPID_netX90_MPW;


#elif ASIC_TYP==ASIC_TYP_NETX90
	HOSTDEF(ptAsicCtrlComArea);
	HOSTDEF(ptSampleAtPornStatArea);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;
	unsigned long ulStrappingOptions;


	/* Get the current boot mode. */
	ulValue = ptAsicCtrlComArea->ulRdy_run_cfg;
	ulRDY = (ulValue & HOSTMSK(rdy_run_cfg_RDY_IN)) >> HOSTSRT(rdy_run_cfg_RDY_IN);
	ulRUN = (ulValue & HOSTMSK(rdy_run_cfg_RUN_IN)) >> HOSTSRT(rdy_run_cfg_RUN_IN);
	ulValue  = ulRDY << SRT_BOOTMODE_RDY;
	ulValue |= ulRUN << SRT_BOOTMODE_RUN;
	ptTestParams->ulBootMode = ulValue;

	/* Get the strapping options. */
	ulValue = ptSampleAtPornStatArea->aulSample_at_porn_stat_in[1];
	ulStrappingOptions  = (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_mosi)) >>  HOSTSRT(sample_at_porn_stat_in1_sqi_mosi);
	ulStrappingOptions |= (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_miso)) >> (HOSTSRT(sample_at_porn_stat_in1_sqi_miso)-1);
	ulStrappingOptions |= (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_sio2)) >> (HOSTSRT(sample_at_porn_stat_in1_sqi_sio2)-2);
	ptTestParams->ulStrappingOptions = ulStrappingOptions;

	/* Set the chip ID. */
	ptTestParams->ulChipID = CHIPID_netX90;


#elif ASIC_TYP==ASIC_TYP_NETX50
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


#elif ASIC_TYP==ASIC_TYP_NETX56
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


#elif ASIC_TYP==ASIC_TYP_NETX10
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
#else
#       error "Unknown ASIC_TYP!"
#endif

}



TEST_RESULT_T test_main(TEST_PARAMETER_T *ptTestParam);
TEST_RESULT_T test_main(TEST_PARAMETER_T *ptTestParam)
{
	TEST_RESULT_T tTestResult;
	BOOTPINS_PARAMETER_T *ptTestParams;
	unsigned long ulTimer;
	int iElapsed;


	systime_init();

	uprintf("\f. *** Bootpins test by doc_bacardi@users.sourceforge.net ***\n");
	uprintf("V" VERSION_ALL "\n\n");

	/* Switch off SYS led. */
	rdy_run_setLEDs(RDYRUN_OFF);

	/* Delay for 500ms to decharge the RDY/RUN lines. */
	ulTimer = systime_get_ms();
	do
	{
		iElapsed = systime_elapsed(ulTimer, 500);
	} while( iElapsed==0 );

	/* Get the test parameter. */
	ptTestParams = (BOOTPINS_PARAMETER_T*)(ptTestParam->pvInitParams);
	get_values(ptTestParams);

	rdy_run_setLEDs(RDYRUN_GREEN);
	tTestResult = TEST_RESULT_OK;

	return tTestResult;
}

/*-----------------------------------*/

