#include "main_test.h"

#include <string.h>

#include "netx_test.h"
#include "rdy_run.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"

/*-----------------------------------*/

#if ASIC_TYP==ASIC_TYP_NETX90
#define NETX90_PHY_VERSION_INVALID 0x00000000
#define NETX90_PHY_VERSION_2 0x01011492
#define NETX90_PHY_VERSION_3 0x01011493

#define NETX90_VERSIONREG_REV0 0x0900000D
#define NETX90_VERSIONREG_REV1 0x0900010D
#define NETX90_VERSIONREG_REV2 0x0901020D

static int enableClocks(void)
{
	HOSTDEF(ptAsicCtrlArea);
	int iResult;
	unsigned long ulMask;
	unsigned long ulEnable;
	unsigned long ulValue;


	/* Be pessimistic. */
	iResult = -1;

	/* Check if all necessary clocks can be enabled. */
	ulMask = HOSTMSK(clock_enable0_mask_xc_misc);
	ulEnable  = HOSTMSK(clock_enable0_xc_misc);
#if ASIC_TYP==ASIC_TYP_NETX90
	ulEnable |= HOSTMSK(clock_enable0_xc_misc_wm);
#endif
	ulMask |= HOSTMSK(clock_enable0_mask_xmac0);
	ulMask |= HOSTMSK(clock_enable0_mask_tpec0);
	ulMask |= HOSTMSK(clock_enable0_mask_rpec0);

	ulEnable |= HOSTMSK(clock_enable0_xmac0);
	ulEnable |= HOSTMSK(clock_enable0_tpec0);
	ulEnable |= HOSTMSK(clock_enable0_rpec0);
#if ASIC_TYP==ASIC_TYP_NETX90
	ulEnable |= HOSTMSK(clock_enable0_xmac0_wm);
	ulEnable |= HOSTMSK(clock_enable0_tpec0_wm);
	ulEnable |= HOSTMSK(clock_enable0_rpec0_wm);
#endif

	ulValue  = ptAsicCtrlArea->asClock_enable[0].ulMask;
	ulValue &= ulMask;
	ulValue ^= ulMask;
	if( ulValue!=0U )
	{
		uprintf("The Ethernet clocks are masked out.\n");
	}
	else
	{
		/* Enable the clocks. */
		ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
		ulValue |= ulEnable;
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
		ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;

		iResult = 0;
	}

	return iResult;
}



static void setup_phy_internal(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptIntPhyCfgComArea);
	unsigned long ulValue;
	unsigned long ulXmMiiCfg;
	unsigned long ulXcMdioCfg;
	unsigned long ulPhyCtrl;


	/* Connect to internal PHY. */
	ulXmMiiCfg = 8U;
	ulXcMdioCfg = 2U;
	ulPhyCtrl = HOSTDFLT(phy_ctrl0);

	/* Disable LVDS connections. */
	ulValue  = ulXmMiiCfg << HOSTSRT(io_config0_sel_xm0_mii_cfg);
#if ASIC_TYP==ASIC_TYP_NETX90
	ulValue |= HOSTMSK(io_config0_sel_xm0_mii_cfg_wm);
#endif
	ulValue |= ulXcMdioCfg << HOSTSRT(io_config0_sel_xc0_mdio);
#if ASIC_TYP==ASIC_TYP_NETX90
	ulValue |= HOSTMSK(io_config0_sel_xc0_mdio_wm);
#endif
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptAsicCtrlArea->asIo_config[0].ulConfig = ulValue;

	ulValue  = ulXmMiiCfg << HOSTSRT(io_config1_sel_xm1_mii_cfg);
#if ASIC_TYP==ASIC_TYP_NETX90
	ulValue |= HOSTMSK(io_config1_sel_xm1_mii_cfg_wm);
#endif
	ulValue |= ulXcMdioCfg << HOSTSRT(io_config1_sel_xc1_mdio);
#if ASIC_TYP==ASIC_TYP_NETX90
	ulValue |= HOSTMSK(io_config1_sel_xc1_mdio_wm);
#endif
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptAsicCtrlArea->asIo_config[1].ulConfig = ulValue;

	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptAsicCtrlArea->ulPhy_ctrl0 = ulPhyCtrl;

/* This bit has been introduced with the netx 90 Rev. 1 
	 and should be set to 1. 
	 Note: This will not work on Rev.0.
	 On Rev0, bits 0-3 are the phy_address
	 Starting with Rev1, bits 0-2 are the phy_address, bit 3 is phy_clk_freq */
#define MSK_NX90_int_phy_cfg_phy_ctrl_phy_clk_freq              0x00000008U
#define SRT_NX90_int_phy_cfg_phy_ctrl_phy_clk_freq              3
#define DFLT_VAL_NX90_int_phy_cfg_phy_ctrl_phy_clk_freq         0x00000000U

	ulValue  = 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_address);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_clk_freq);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_mode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_fxmode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_automdix);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_np_msg_code);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_enable);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_mode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_fxmode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_automdix);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_np_msg_code);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_enable);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_reset);
	ptIntPhyCfgComArea->ulInt_phy_cfg_phy_ctrl = ulValue;
	
	systime_delay_ms(100);
	
	ulValue  = 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_address);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_clk_freq);
	ulValue |= 7U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_mode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_fxmode);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_automdix);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_np_msg_code);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_enable);
	ulValue |= 7U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_mode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_fxmode);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_automdix);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_np_msg_code);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_enable);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_reset);
	ptIntPhyCfgComArea->ulInt_phy_cfg_phy_ctrl = ulValue;
	
	systime_delay_ms(100);
	
	ulValue  = 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_address);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_clk_freq);
	ulValue |= 7U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_mode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_fxmode);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_automdix);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_np_msg_code);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy0_enable);
	ulValue |= 7U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_mode);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_fxmode);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_automdix);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_np_msg_code);
	ulValue |= 1U << HOSTSRT(int_phy_cfg_phy_ctrl_phy1_enable);
	ulValue |= 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_reset);
	ptIntPhyCfgComArea->ulInt_phy_cfg_phy_ctrl = ulValue;
	
	systime_delay_ms(100);
}



static unsigned long read_phy_register(unsigned long ulPhy, unsigned long ulRegister)
{
	HOSTDEF(ptXc0PhyCtrl0Area);
	unsigned long ulValue;


	/* Run a read operation. */
	ulValue  =         1U << HOSTSRT(int_phy_ctrl_miimu_snrdy);
	ulValue |=         1U << HOSTSRT(int_phy_ctrl_miimu_preamble);
	ulValue |=         0U << HOSTSRT(int_phy_ctrl_miimu_opmode);
	ulValue |=         0U << HOSTSRT(int_phy_ctrl_miimu_mdc_period);
	ulValue |=         1U << HOSTSRT(int_phy_ctrl_miimu_rta);
	ulValue |= ulRegister << HOSTSRT(int_phy_ctrl_miimu_regaddr);
	ulValue |=      ulPhy << HOSTSRT(int_phy_ctrl_miimu_phyaddr);
	ulValue |=         0U << HOSTSRT(int_phy_ctrl_miimu_data);
	ptXc0PhyCtrl0Area->ulInt_phy_ctrl_miimu = ulValue;

	/* Wait until the snrdy bit is 0. */
	do
	{
		ulValue  = ptXc0PhyCtrl0Area->ulInt_phy_ctrl_miimu;
	} while( (ulValue&HOSTMSK(int_phy_ctrl_miimu_snrdy))!=0 );

	/* Extract the data from the result. */
	ulValue = (ulValue & HOSTMSK(int_phy_ctrl_miimu_data)) >> HOSTSRT(int_phy_ctrl_miimu_data);

	return ulValue;
}


static unsigned long get_phy_revision(unsigned int fSetupPhy)
{
	int iResult;
	unsigned long ulValue;

	iResult = 0;
	ulValue = NETX90_PHY_VERSION_INVALID;
	
	if (fSetupPhy == 0)
	{
		uprintf("get_phy_revision: Skipping PHY setup.\n\n"); 
	}
	else
	{
		iResult = enableClocks();
		if (iResult == 0)
		{
			setup_phy_internal();
		}
	}
	
	if (iResult == 0)
	{
		/* Read the PHY version.
		* The version is split over 2 registers. Register 2 has the "high" part
		* and register 3 has the "low" part.
		*/
		ulValue  = read_phy_register(0, 2) << 16U;
		ulValue |= read_phy_register(0, 3);
	}

	return ulValue;
}
#endif

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

	/* Copy the unique ID. */
	ulValue = ptRAPSysctrlArea->aulRAP_SYSCTRL_OTP_CONFIG_[1];
	ptTestParams->sizUniqueIdInBits = 16;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));
	ptTestParams->tUniqueID.auc[0] = (unsigned char)((ulValue & MSK_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_122) >> SRT_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_122);
	ptTestParams->tUniqueID.auc[1] = (unsigned char)((ulValue & MSK_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_123) >> SRT_NX4000_RAP_SYSCTRL_OTP_CONFIG_1_FUSE_123);


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

	/* The netX500/100 has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));


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

	/* The netX90MPW has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));


#elif ASIC_TYP==ASIC_TYP_NETX90
	HOSTDEF(ptAsicCtrlComArea);
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptSampleAtPornStatArea);
	unsigned long ulValue;
	unsigned long ulRDY;
	unsigned long ulRUN;
	unsigned long ulStrappingOptions;
	unsigned long *pulId;
	CHIPID_T tChipID;
	unsigned long ulVersionRegister;


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

	/* Distinguish netX90, netX90B, netX90BPhyR3 and netX90C. */
	pulId = (unsigned long*)0x000000c0U;
	ulValue = *pulId;
	ulVersionRegister = ptAsicCtrlArea->ulNetx_version;
	tChipID = CHIPID_unknown;
	if( ulValue==0x0010a005 )
	{
		tChipID = CHIPID_netX90;
	}
	else if( ulValue==0x0010d005 )
	{
		/* Get the PHY revision. */
		
		ulValue = get_phy_revision(ptTestParams->fSetupPhy);
		if( ulValue==NETX90_PHY_VERSION_2 )
		{
			tChipID = CHIPID_netX90B;
		}
		else if( ulValue==NETX90_PHY_VERSION_3 )
		{
			if( ulVersionRegister==NETX90_VERSIONREG_REV2 )
			{
				tChipID = CHIPID_netX90C;
			}
			else if( ulVersionRegister==NETX90_VERSIONREG_REV1 )
			{
				tChipID = CHIPID_netX90BPhyR3;
			}
		}
	}
	ptTestParams->ulChipID = tChipID;

	/* Copy the unique ID. */
	ptTestParams->sizUniqueIdInBits = sizeof(ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id) * 8U;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));
	ptTestParams->tUniqueID.aul[0] = ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id[0];
	ptTestParams->tUniqueID.aul[1] = ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id[1];
	ptTestParams->tUniqueID.aul[2] = ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id[2];


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

	/* The netX50 has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));


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

	/* The netX56 has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));


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

	/* The netX10 has no unique chip ID. */
	ptTestParams->sizUniqueIdInBits = 0;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));
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
	uprintf("V" VERSION_ALL " (for CLI Flasher) \n\n");

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

