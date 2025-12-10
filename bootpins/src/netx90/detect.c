#include "detect.h"

#include "netx_io_areas.h"
#include "systime.h"
#include "uprintf.h"

#include <string.h>


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

	ulValue  = 0U << HOSTSRT(int_phy_cfg_phy_ctrl_phy_address);
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


static unsigned long get_phy_revision(void)
{
	int iResult;
	unsigned long ulValue;


	iResult = enableClocks();
	if( iResult!=0 )
	{
		ulValue = NETX90_PHY_VERSION_INVALID;
	}
	else
	{
		setup_phy_internal();

		/* Read the PHY version.
		* The version is split over 2 registers. Register 2 has the "high" part
		* and register 3 has the "low" part.
		*/
		ulValue  = read_phy_register(0, 2) << 16U;
		ulValue |= read_phy_register(0, 3);
	}

	return ulValue;
}



void get_values(BOOTPINS_PARAMETER_T *ptTestParams)
{
#if ASIC_TYP==ASIC_TYP_NETX90_MPW
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
		if( ulVersionRegister==NETX90_VERSIONREG_REV1 )
		{
			/* Further examination can only be done by accessing the PHY.
			 * This will corrupt a running Ethernet communication. Check
			 * the flags if the PHY is critical.
			 */
			ulValue  = ptTestParams->ulFlags;
			ulValue &= BOOTPINS_FLAGS_PHY_IS_CRITICAL;
			if( ulValue!=0 )
			{
				uprintf("Not touching the PHY as it is marked as 'critical'.\n");
				tChipID = CHIPID_netX90BPhyR2or3;
			}
			else
			{
				/* Distinguish netX90B and netX90BPhyR3. */

				/* Get the PHY revision. */
				ulValue = get_phy_revision();
				if( ulValue==NETX90_PHY_VERSION_2 )
				{
					tChipID = CHIPID_netX90B;
				}
				else if( ulValue==NETX90_PHY_VERSION_3 )
				{
					tChipID = CHIPID_netX90BPhyR3;
				}
			}
		}
		else if( ulVersionRegister==NETX90_VERSIONREG_REV2 )
		{
			tChipID = CHIPID_netX90C;
		}
	}
	ptTestParams->ulChipID = tChipID;

	/* Copy the unique ID. */
	ptTestParams->sizUniqueIdInBits = sizeof(ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id) * 8U;
	memset(ptTestParams->tUniqueID.auc, 0, sizeof(ptTestParams->tUniqueID));
	ptTestParams->tUniqueID.aul[0] = ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id[0];
	ptTestParams->tUniqueID.aul[1] = ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id[1];
	ptTestParams->tUniqueID.aul[2] = ptAsicCtrlArea->aulAsic_ctrl_netx_unique_id[2];

#else
#       error "Unknown ASIC_TYP!"
#endif
}
