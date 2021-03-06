//****************************************************************************
// epl_1588.c
// 
// Copyright (c) 2007-2008 National Semiconductor Corporation.
// All Rights Reserved
// 
// Contains sources for IEEE 1588 related functions.
//
// The following functions are implemented in this module:
//
//      PTPEnable
//      PTPSetTriggerConfig
//      PTPSetEventConfig
//      PTPSetTransmitConfig
//      PTPSetPhyStatusFrameConfig
//      PTPSetReceiveConfig
//      PTPCalcSourceIdHash
//      PTPSetTempRateDurationConfig
//      PTPSetClockConfig
//      PTPSetGpioInterruptConfig
//      PTPSetMiscConfig
//      PTPClockReadCurrent
//      PTPClockStepAdjustment
//      PTPClockSet
//      PTPClockSetRateAdjustment
//      PTPCheckForEvents
//      PTPGetTransmitTimestamp
//      PTPGetReceiveTimestamp
//      PTPGetTimestampFromFrame
//      PTPArmTrigger
//      PTPHasTriggerExpired
//      PTPCancelTrigger
//      PTPGetEvent
//      MonitorGpioSignals
//      IsPhyStatusFrame
//      GetNextPhyMessage
//****************************************************************************

#include "epl.h"
#include "ptpd.h"
#include "HL_mdio.h"
#include "NetworkInterface.h"
extern RX_CFG_ITEMS rxCfgItems;
extern uint32_t rxCfgOpts;

void EPLWriteReg(PEPL_PORT_HANDLE epl_port_handle, uint32_t reg, uint32_t data){
	OAIBeginRegCriticalSection(epl_port_handle->oaiDevHandle);
	dp83630_set_page(&(epl_port_handle->hdkif), reg);
	MDIOPhyRegWrite(epl_port_handle->hdkif.mdio_base, epl_port_handle->hdkif.phy_addr, reg&(~0xE0), data);
	OAIEndRegCriticalSection(epl_port_handle->oaiDevHandle);
}

uint32_t EPLReadReg(PEPL_PORT_HANDLE epl_port_handle, uint32_t reg){
	OAIBeginRegCriticalSection(epl_port_handle->oaiDevHandle);
	//TODO: return data to be consistent with original API
	dp83630_set_page(&(epl_port_handle->hdkif), reg);
	uint32_t data = 0;
	MDIOPhyRegRead(epl_port_handle->hdkif.mdio_base,
				   epl_port_handle->hdkif.phy_addr,
				   reg&(~0xE0),
				   &data);
	OAIEndRegCriticalSection(epl_port_handle->oaiDevHandle);
	return data;
}

void PTPEnable(PEPL_PORT_HANDLE epl_port_handle, boolean enableFlag)
//  Enables or disables the PHY's PTP 1588 Clock.
//
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  enableFlag
//      Set to TRUE to enable the Phy's 1588 hardware clock. Set to FALSE
//      to disable it.
//
//  Returns:
//      Nothing
//****************************************************************************
{
	EPLWriteReg(epl_port_handle, PHY_PG4_PTP_CTL, (enableFlag) ? P640_PTP_ENABLE : P640_PTP_DISABLE);
    return;
}
 
void PTPSetTriggerConfig(PEPL_PORT_HANDLE epl_port_handle,
        uint32_t trigger,
        uint32_t triggerBehavior,
        uint32_t gpioConnection)
//  Configures the operational behavior of an individual trigger.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  trigger
//      The trigger to configure, 0 - 7.
//  triggerBehavior
//      A bitmap of configuration options. Zero or more of the bits defined 
//      by TRGOPT_??? OR'ed together.
//  gpioConnection
//      The GPIO pin the trigger should be connected to. A value of 0 - 12. 
//      If 0 is specified no GPIO pin connection is made.
//  Returns
//      Nothing
//****************************************************************************
{
	uint32_t reg;

    reg = 0;
    if ( triggerBehavior & TRGOPT_PULSE)        reg |= P640_TRIG_PULSE;
    if ( triggerBehavior & TRGOPT_PERIODIC)     reg |= P640_TRIG_PER;
    if ( triggerBehavior & TRGOPT_TRG_IF_LATE)  reg |= P640_TRIG_IF_LATE;
    if ( triggerBehavior & TRGOPT_NOTIFY_EN)    reg |= P640_TRIG_NOTIFY;
    if ( triggerBehavior & TRGOPT_TOGGLE_EN)    reg |= P640_TRIG_TOGGLE;
    
    reg |= gpioConnection << P640_TRIG_GPIO_SHIFT;
    reg |= trigger << P640_TRIG_CSEL_SHIFT;
    reg |= P640_TRIG_WR;
    
//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_TRIG, reg);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_TRIG, reg);
    return;
}
 
void PTPSetEventConfig (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t event,
        boolean eventRiseFlag,
        boolean eventFallFlag,
        boolean eventSingle,
        uint32_t gpioConnection)
//  Configures the operational behavior of an individual event.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  event
//      The event to configure, 0 - 7.
//  eventRiseFlag
//      If set to TRUE, enables detection of rising edge on Event input.
//  eventFallFlag
//      If set to TRUE, enables detection of falling edge on Event input.
//  eventSingle
//      If set to TRUE, enables single event capture operation
//  gpioConnection
//      The GPIO pin the event should be connected to. A value of 0 - 12. 
//      If 0 is specified no GPIO pin connection is made.
//  Returns
//      Nothing
//****************************************************************************
{
	uint32_t reg;

    reg = 0;
    
    reg |= gpioConnection << P640_EVNT_GPIO_SHIFT;
    reg |= event << P640_EVNT_SEL_SHIFT;
    reg |= P640_EVNT_WR;
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_EVNT, reg);
    
    if ( eventRiseFlag) reg |= P640_EVNT_RISE;
    if ( eventFallFlag) reg |= P640_EVNT_FALL;
    if ( eventSingle) reg |= P640_EVNT_SINGLE;
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_EVNT, reg);
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
    return;
}

void PTPSetTransmitConfig (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t txConfigOptions,
        uint32_t ptpVersion,
        uint32_t ptpFirstByteMask,
        uint32_t ptpFirstByteData)

//  Configures the device's 1588 transmit operation.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  txConfigOptions
//      A bitmap of configuration options. Zero or more of the bits defined 
//      by TXOPT_??? OR'ed together.
//  ptpVersion
//      Enable Timestamp capture for a specific version of the IEEE 1588 
//      specification. This value may be set to any value between 1 and 15 
//      and allows support for future versions of the IEEE 1588 specification. 
//      A value of 0 will disable version checking (not recommended).
//  ptpFirstByteMask
//      Bit mask to be used for matching Byte0 of the PTP Message. A one in 
//      any bit enables matching for the associated data bit. If no matching 
//      is required, all bits of the mask should be set to 0.
//  ptpFirstByteData
//      Data to be used for matching Byte0 of the PTP Message. This parameter 
//      is ignored if ptpFirstByteMask is 0x00.
//  Returns
//      Nothing
//****************************************************************************
{
	uint32_t reg;

    reg = 0;
    if ( txConfigOptions & TXOPT_SYNC_1STEP)    reg |= P640_SYNC_1STEP;
    if ( txConfigOptions & TXOPT_DR_INSERT)     reg |= P640_DR_INSERT;
    if ( txConfigOptions & TXOPT_NTP_TS_EN)     reg |= P640_NTP_TS_EN;
    if ( txConfigOptions & TXOPT_IGNORE_2STEP)  reg |= P640_IGNORE_2STEP;
    if ( txConfigOptions & TXOPT_CRC_1STEP)     reg |= P640_CRC_1STEP;
    if ( txConfigOptions & TXOPT_CHK_1STEP)     reg |= P640_CHK_1STEP;
    if ( txConfigOptions & TXOPT_IP1588_EN)     reg |= P640_IP1588_EN;
    if ( txConfigOptions & TXOPT_L2_EN)         reg |= P640_TX_L2_EN;
    if ( txConfigOptions & TXOPT_IPV6_EN)       reg |= P640_TX_IPV6_EN;
    if ( txConfigOptions & TXOPT_IPV4_EN)       reg |= P640_TX_IPV4_EN;
    if ( txConfigOptions & TXOPT_TS_EN)         reg |= P640_TX_TS_EN;

    reg |= ptpVersion << P640_TX_PTP_VER_SHIFT;
//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_TXCFG0, reg);
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_TXCFG0, reg);

    reg = (ptpFirstByteMask << P640_BYTE0_MASK_SHIFT) | (ptpFirstByteData << P640_BYTE0_DATA_SHIFT);
//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_TXCFG1, reg);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_TXCFG1, reg);
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
    return;
}

uint32_t PTPReadTransmitConfig (PEPL_PORT_HANDLE epl_port_handle)
{
	uint32_t reg = 0;
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
	reg = EPLReadReg(epl_port_handle, PHY_PG5_PTP_TXCFG0);
	reg = EPLReadReg(epl_port_handle, PHY_PG5_PTP_TXCFG1) << 16;
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
    return reg;
}

void PTPSetPhyStatusFrameConfig (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t statusConfigOptions,
        MAC_SRC_ADDRESS_ENUM srcAddrToUse,
        uint32_t minPreamble,
        uint32_t ptpReserved,
        uint32_t ptpVersion,
        uint32_t transportSpecific,
        uint8_t messageType,
        uint32_t sourceIpAddress)
//  Configures the device's Phy Status Frame (PSF) operational configuration.
//
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort
//      function.
//  statusConfigOptions
//      A bitmap of configuration options. Zero or more of the bits defined
//      the STSOPT_??? bit definitions - OR'ed together.
//  srcAddrToUse
//      Specifies the MAC source address to use in Phy Status Frames (PSF).
//      Must be one of the following values.
//          STS_SRC_ADDR_1 - [00 00 00 00 00 00]
//          STS_SRC_ADDR_2 - [08 00 17 0B 6B 0F]
//          STS_SRC_ADDR_3 - [08 00 17 00 00 00]
//          STS_SRC_ADDR_USE_MC - Use MAC multicast destination address
//  minPreamble
//      Determines the minimum number of preamble bytes required for sending
//      Phy Status Frames (PSF) on the MII interface. It is recommended that
//      this be set to the smallest value the MAC will tolerate.
//  ptpReserved
//      PTP v2 reserved field: This field contains the reserved 4-bit field
//      (at offset 1) to be sent in status packets from the Phy to the local
//      MAC using the MII receive data interface.
//  ptpVersion
//      The PTP version number to set in the PTP version field of the status
//      frame. Typically this is set to a value of 0x02.
//  transportSpecific
//      The value to use for the transportSpecific field for status frames
//      from the Phy to the local MAC using the MII receive data interface.
//      A value of 0x0F ensures the frame will not be interpreted as a valid
//      PTP message.
//  messageType
//      The value to use for the messageType field for status frames from
//      the Phy to the local MAC using the MII receive data interface.
//      The default value of 0x0F ensures the frame will not be interpreted
//      as a valid PTP message.
//  sourceIpAddress
//      4-byte source IP address to use in frames created by the Phy.
//
//  Returns
//      Nothing
//****************************************************************************
{
	uint32_t reg;
	uint8_t *ptr;
	static uint8_t srcAddrs[4][6] = \
							{ {0x08,0x00,0x17,0x0B,0x6B,0x0F},
							  {0x08,0x00,0x17,0x00,0x00,0x00},
							  {0x01,0x00,0x00,0x00,0x00,0x00},
							  {0x00,0x00,0x00,0x00,0x00,0x00} };

	// NOTE: ocList is packed in little endian order for proper calculation
	uint16_t ocList[6] = { 0x4500, 0x0111, 0xE000, 0x0181, 0x0000, 0x0000 }; // Little Endian
	//uint16_t ocList[6] = { 0x0045, 0x1101, 0x00E0, 0x8101, 0x0000, 0x0000 }; // Big Endian

	uint8_t  i;
	uint32_t ipChecksum, rollover;

    ptr = srcAddrs[ srcAddrToUse ];
	*(uint32_t*)&epl_port_handle->psfSrcMacAddr[0] = *(uint32_t*)&ptr[0];
	*(uint16_t*)&epl_port_handle->psfSrcMacAddr[4] = *(uint16_t*)&ptr[4];

    reg = 0;
    if( statusConfigOptions & STSOPT_LITTLE_ENDIAN ) reg |= P640_PKT_ENDIAN;
    if( statusConfigOptions & STSOPT_IPV4 )          reg |= P640_PKT_IPV4;
    if( statusConfigOptions & STSOPT_TXTS_EN )       reg |= P640_PKT_TXTS_EN;
    if( statusConfigOptions & STSOPT_RXTS_EN )       reg |= P640_PKT_RXTS_EN;
    if( statusConfigOptions & STSOPT_TRIG_EN )       reg |= P640_PKT_TRIG_EN;
    if( statusConfigOptions & STSOPT_EVENT_EN )      reg |= P640_PKT_EVNT_EN;
    if( statusConfigOptions & STSOPT_ERR_EN )        reg |= P640_PKT_ERR_EN;

    // Ensure that PCF read responses are enabled if that is our only mechanism
    // to interact with the PHY, otherwise everything will stop working
    if ( statusConfigOptions & STSOPT_PCFR_EN /*|| portHandle->oaiDevHandle->pcfDefault*/ ) {
        reg |= P640_PKT_PCFR_EN;
    }

    reg |= srcAddrToUse << P640_MAC_SRC_ADD_SHIFT;
    reg |= minPreamble << P640_MIN_PRE_SHIFT;
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
    EPLWriteReg( epl_port_handle, PHY_PG5_PSF_CFG0, reg );

    reg = ptpReserved << P640_PTP_RESERVED_SHIFT;
    reg |= ptpVersion << P640_VERSION_PTP_SHIFT;
    reg |= transportSpecific << P640_TRANSP_SPEC_SHIFT;
    reg |= messageType << P640_MESSAGE_TYPE_SHIFT;
    EPLWriteReg( epl_port_handle, PHY_PG6_PSF_CFG1, reg );

	// sourceIpAddress is given in network (big endian) order
    reg  =  (sourceIpAddress & 0x000000FF)        << P640_IP_SA_BYTE0_SHIFT;
	reg |= ((sourceIpAddress & 0x0000FF00) >>  8) << P640_IP_SA_BYTE1_SHIFT;
    EPLWriteReg( epl_port_handle, PHY_PG6_PSF_CFG2, reg );

    reg  = ((sourceIpAddress & 0x00FF0000) >> 16) << P640_IP_SA_BYTE2_SHIFT;
    reg |= ((sourceIpAddress & 0xFF000000) >> 24) << P640_IP_SA_BYTE3_SHIFT;
    EPLWriteReg( epl_port_handle, PHY_PG6_PSF_CFG3, reg );

    // Calculate the checksum
#if 1
    // Little Endian
    ocList[4] = (uint16_t)((sourceIpAddress & 0x000000FF)<<8 | (sourceIpAddress & 0x0000FF00)>> 8);
    ocList[5] = (uint16_t)((sourceIpAddress & 0x00FF0000)>>8 | (sourceIpAddress & 0xFF000000)>>24);
#else
    // Big Endian
    ocList[4] = (uint16_t)((sourceIpAddress & 0x0000FFFF)       );
    ocList[5] = (uint16_t)((sourceIpAddress & 0xFFFF0000) >> 16 );
#endif

    ipChecksum = 0;
    for( i=0; i<6; i++ )
        ipChecksum += ocList[i];
    while( ipChecksum > 0xFFFF ) {
        rollover = (ipChecksum >> 16);
        ipChecksum = (ipChecksum & 0xFFFF) + rollover;
    }

    EPLWriteReg( epl_port_handle, PHY_PG6_PSF_CFG4, ipChecksum );
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);

    epl_port_handle->psfConfigOptions = statusConfigOptions;
    return;
}

void PTPSetReceiveConfig (PEPL_PORT_HANDLE epl_port_handle,
			uint32_t rxConfigOptions,
			RX_CFG_ITEMS *rxConfigItems)
//  Configures the device's receive operation.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  rxConfigOptions
//      A bitmap of configuration options. Zero or more of the bits defined 
//      by the RXOPT_??? bit definitions OR'ed together.
//  rxConfigItems
//      This structure of configuration values must be filled out prior to 
//      making this call. See the RX_CFG_ITEMS structure definition.
//  Returns
//      Nothing
//****************************************************************************
{
	uint32_t reg;

    reg = 0;
    if ( rxConfigOptions & RXOPT_DOMAIN_EN)      reg |= P640_DOMAIN_EN;
    if ( rxConfigOptions & RXOPT_ALT_MAST_DIS)   reg |= P640_ALT_MAST_DIS;
    if ( rxConfigOptions & RXOPT_USER_IP_EN)     reg |= P640_USER_IP_EN;
    if ( rxConfigOptions & RXOPT_RX_SLAVE)       reg |= P640_RX_SLAVE;
    if ( rxConfigOptions & RXOPT_IP1588_EN0)     reg |= P640_IP1588_EN0;
    if ( rxConfigOptions & RXOPT_IP1588_EN1)     reg |= P640_IP1588_EN1;
    if ( rxConfigOptions & RXOPT_IP1588_EN2)     reg |= P640_IP1588_EN2;
    if ( rxConfigOptions & RXOPT_RX_L2_EN)       reg |= P640_RX_L2_EN;
    if ( rxConfigOptions & RXOPT_RX_IPV6_EN)     reg |= P640_RX_IPV6_EN;
    if ( rxConfigOptions & RXOPT_RX_IPV4_EN)     reg |= P640_RX_IPV4_EN;
    if ( rxConfigOptions & RXOPT_RX_TS_EN)       reg |= P640_RX_TS_EN;
   
    reg |= rxConfigItems->ptpVersion << P640_RX_PTP_VER_SHIFT;

//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG0, reg);
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG0, reg);

//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG2, rxConfigItems->ipAddrData >> 16);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG2, rxConfigItems->ipAddrData >> 16);

//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG0, reg | P640_USER_IP_SEL);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG0, reg | P640_USER_IP_SEL);

//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG2, rxConfigItems->ipAddrData & 0x0000FFFF);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG2, rxConfigItems->ipAddrData & 0x0000FFFF);

//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG0, reg);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG0, reg);

    reg = rxConfigItems->ptpFirstByteMask << P640_BYTE0_MASK_SHIFT;
    reg |= rxConfigItems->ptpFirstByteData << P640_BYTE0_DATA_SHIFT;
//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG1, reg);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG1, reg);
    epl_port_handle->rxConfigOptions = rxConfigOptions;
    reg = 0;    
    if ( rxConfigOptions & RXOPT_ACC_UDP)        reg |= P640_ACC_UDP;
    if ( rxConfigOptions & RXOPT_ACC_CRC)        reg |= P640_ACC_CRC;
    if ( rxConfigOptions & RXOPT_TS_APPEND)      reg |= P640_TS_APPEND;
    if ( rxConfigOptions & RXOPT_TS_INSERT)      reg |= P640_TS_INSERT;
    
    reg |= rxConfigItems->tsMinIFG << P640_TS_MIN_IFG_SHIFT;
    reg |= rxConfigItems->ptpDomain << P640_PTP_DOMAIN_SHIFT;
//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG3, reg);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG3, reg);
    
    reg = 0;
    if ( rxConfigOptions & RXOPT_IPV4_UDP_MOD) reg |= P640_IPV4_UDP_MOD;
    if ( rxConfigOptions & RXOPT_TS_SEC_EN)    reg |= P640_TS_SEC_EN;

    epl_port_handle->tsSecondsLen = rxConfigItems->tsSecLen;
    epl_port_handle->rxTsNanoSecOffset = rxConfigItems->rxTsNanoSecOffset;
    epl_port_handle->rxTsSecondsOffset = rxConfigItems->rxTsSecondsOffset;
    
    reg |= rxConfigItems->tsSecLen << P640_TS_SEC_LEN_SHIFT;
    reg |= rxConfigItems->rxTsNanoSecOffset << P640_RXTS_NS_OFF_SHIFT;
    reg |= rxConfigItems->rxTsSecondsOffset << P640_RXTS_SEC_OFF_SHIFT;
//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG5_PTP_RXCFG4, reg);
	EPLWriteReg(epl_port_handle, PHY_PG5_PTP_RXCFG4, reg);

    reg = rxConfigItems->srcIdHash << P640_PTP_RX_HASH_SHIFT;
    if ( rxConfigOptions & RXOPT_SRC_ID_HASH_EN) reg |= P640_RX_HASH_EN;
//    EPLWriteReg( &(epl_port_handle->hdkif), PHY_PG6_PTP_RXHASH, reg);
	EPLWriteReg(epl_port_handle, PHY_PG6_PTP_RXHASH, reg);
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
    return;
}

uint32_t PTPReadReceiveConfig (PEPL_PORT_HANDLE epl_port_handle)
{
	uint32_t reg = 0;
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
	reg = EPLReadReg(epl_port_handle, PHY_PG5_PTP_RXCFG0);
	reg = EPLReadReg(epl_port_handle, PHY_PG5_PTP_RXCFG1);
	reg = EPLReadReg(epl_port_handle, PHY_PG5_PTP_RXCFG2);
	reg = EPLReadReg(epl_port_handle, PHY_PG5_PTP_RXCFG3);
	reg = EPLReadReg(epl_port_handle, PHY_PG5_PTP_RXCFG4);
	reg = EPLReadReg(epl_port_handle, PHY_PG6_PTP_RXHASH);
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
    return reg;
}

uint32_t PTPCalcSourceIdHash (uint8_t *tenBytesData)
//  Utility function that will calculate a 12-bit CRC-32 (IEEE 802.3, no 
//  complement) value used to program the RX_CFG_ITEMS.srcIdHash field.
//  
//  tenBytesData
//      This is a pointer to the desired fixed values found in bytes 20 - 29 
//      of the PTP event message that receive source identification filtering 
//      should occur.
//  Returns
//      Value representing the CRC-32 (IEEE 802.3, no complement) generated 
//      from the specified 10-byte buffer.
//
//  This function would only be used if RXOPT_SRC_ID_HASH_EN was specified 
//  in a call to PTPSetReceiveConfig.
//****************************************************************************
{
uint32_t crc, x, i, val;
uint8_t data;

    crc = 0xFFFFFFFF;
    for ( x = 0; x < 10; x++)
    {
        data = tenBytesData[x];
        
        for ( i = 0; i < 8; i++)
        {
            val = crc << 1;
            if ((data & 0x01) ^ ((crc >> 31) & 0x01))
                crc = val ^ 0x04C11DB7;
            else
                crc = val ^ 0;
            data = data >> 1;
        }
    }

    return (crc & 0xFFFFFFFF) >> 20;
}

void PTPSetTempRateDurationConfig (PEPL_PORT_HANDLE epl_port_handle, uint32_t duration)
//  Configures the PTP Temporary Duration.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  duration
//      PTP Temporary Rate Duration: This sets the duration for the Temporary 
//      Rate in number of clock cycles. The actual Time duration is dependent 
//      on the value of the configured Temporary Rate. This value has a range 
//      of up to 2^26 (26-bits may be defined). 
//  Returns
//      Nothing
//
//  This value is remembered by the hardware, therefore the setting can be 
//  used for multiple temporary clock adjustments, if the desired duration 
//  remains constant.
//****************************************************************************
{
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
    EPLWriteReg(epl_port_handle, PHY_PG5_PTP_TRDH, duration >> P640_PTP_RATE_HI_SHIFT);
    EPLWriteReg(epl_port_handle, PHY_PG5_PTP_TRDL, duration & 0xFFFF);
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
    return;
}

void PTPSetClockConfig (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t clockConfigOptions,
        uint32_t ptpClockDivideByValue,
        uint32_t ptpClockSource,
        uint32_t ptpClockSourcePeriod)
//  Configures the general PTP clock configuration options.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  clockConfigOptions
//      A bitmap of configuration options. Zero or more of the bits defined in 
//      the CLKOPT_??? bit definitions OR'ed together.
//  ptpClockDivideByValue
//      PTP Clock Divide-by Value: This parameter defines the divide-by value 
//      for the output clock. The output clock is divided from an internal 
//      250MHz clock. Valid values range from 2 to 255 (0x02 to 0xFF), giving 
//      a nominal output frequency range of 125MHz down to 980.4kHz. Divide-by 
//      values of 0 and 1 are not valid and will stop the output clock.
//  ptpClockSource
//      PTP Clock Source Select: Selects among three possible sources for the 
//      PTP reference clock, one of the following values may be specified:
//          0x00 - 125MHz from internal PGM (default)
//          0x01 - Divide-by-N from 125MHz internal PGM
//          0x02 - External reference clock
//  ptpClockSourcePeriod
//      PTP Clock Source Period: Configures the PTP clock source period in 
//      nanoseconds. Values less than 8 are invalid. This parameter is used as 
//      follows by the different clock source modes:
//          0x00 - 125MHz - Ignored
//          0x01 - Divide-by-N - Bits 6:3 are used to divide the 125MHz PGM 
//                 clock by a value between 1 and 15. Bits 2:0 are ignored. 
//          0x02 - External - Bits 6:0 indicate the nominal period in 
//                 nanoseconds of the external reference clock.
//
//  Returns
//      Nothing
//****************************************************************************
{
	uint32_t reg;
    reg = 0;
    if ( clockConfigOptions & CLKOPT_CLK_OUT_EN) reg |= P640_PTP_CLKOUT_EN;
    if ( clockConfigOptions & CLKOPT_CLK_OUT_SEL) reg |= P640_PTP_CLKOUT_SEL;
    if ( clockConfigOptions & CLKOPT_CLK_OUT_SPEED_SEL) reg |= P640_PTP_CLKOUT_SPSEL;
    
    reg |= ptpClockDivideByValue << P640_PTP_CLKDIV_SHIFT;
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
    EPLWriteReg( epl_port_handle, PHY_PG6_PTP_COC, reg);

    reg = (ptpClockSource << P640_CLK_SRC_SHIFT) | (ptpClockSourcePeriod << P640_CLK_SRC_PER_SHIFT);
    EPLWriteReg( epl_port_handle, PHY_PG6_PTP_CLKSRC, reg);
    
    // Enable the output clock if requested
	reg = EPLReadReg(epl_port_handle, PHY_PG0_PHYCR2);
    if ( clockConfigOptions & CLKOPT_CLK_OUT_EN)
        reg &= ~PHYCR2_CLK_OUT_DIS;
    else
        reg |= PHYCR2_CLK_OUT_DIS;
    EPLWriteReg( epl_port_handle, PHY_PG0_PHYCR2, reg);
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
}

void PTPSetGpioInterruptConfig (PEPL_PORT_HANDLE epl_port_handle, uint32_t gpioInt)
//  Configures the GPIO pin to be used for the PTP Interrupt function.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  gpioInt
//      Specifies the number of the GPIO to assign to the PTP Interrupt. 
//      This is a value from 1 - 12. Specifying 0 disables interrupt / GPIO 
//      assignment.
//
//  Returns
//      Nothing
//****************************************************************************
{
    EPLWriteReg( epl_port_handle, PHY_PG6_PTP_INTCTL, gpioInt);
}

void PTPSetMiscConfig (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t ptpEtherType,
        uint32_t ptpOffset,
        uint32_t txSfdGpio,
        uint32_t rxSfdGpio)
//  Sets various miscellaneous PTP configuration options.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  ptpEtherType
//      This parameter defines the Ethernet Type field used to detect PTP 
//      messages transported over Ethernet layer 2. Normally this would be set 
//      to 0xF788.
//  ptpOffset
//      This parameter defines the offset in bytes to the PTP Message from 
//      the preceding header. For Layer2, this is the offset from the Ethernet 
//      Type Field. For IP/UDP, it is the offset from the end of the UDP 
//      Header. Values are from 0x00 - 0xFF.
//  txSfdGpio
//      Tx Start of Frame GPIO Select: This parameter specifies the GPIO 
//      output to which the Tx SFD signal is assigned. Valid values are 0 
//      (disabled) or 1-12.
//  rxSfdGpio
//      Rx Start of Frame GPIO Select: This parameter specifies the GPIO 
//      output to which the Rx SFD signal is assigned. Valid values are 0 
//      (disabled) or 1-12.
//
//  Returns
//      Nothing
//****************************************************************************
{
    OAIBeginMultiCriticalSection(epl_port_handle->oaiDevHandle);
    EPLWriteReg( epl_port_handle, PHY_PG6_PTP_ETR, ptpEtherType);
    EPLWriteReg( epl_port_handle, PHY_PG6_PTP_OFF, ptpOffset);
    EPLWriteReg( epl_port_handle, PHY_PG6_PTP_SFDCFG,
                 (txSfdGpio << P640_TX_SFD_GPIO_SHIFT) | 
                 (rxSfdGpio << P640_RX_SFD_GPIO_SHIFT));
    OAIEndMultiCriticalSection(epl_port_handle->oaiDevHandle);
}

void PTPClockReadCurrent (PEPL_PORT_HANDLE epl_port_handle,
        int32_t *retNumberOfSeconds,
        int32_t *retNumberOfNanoSeconds)
//  Returns a snapshot of the current IEEE 1588 clock value.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  retNumberOfSeconds
//      Will be set on return to the number of seconds comprising the IEEE 
//      1588 hardware clock.
//  retNumberOfNanoSeconds
//      Will be set on return to the number of nanoseconds comprising the 
//      IEEE 1588 hardware clock. This value cannot be larger then 
//      1e9 (1 second).
//
//  Returns
//      Nothing
{
    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
//	uint32_t ptp_control_reg = EPLReadReg(epl_port_handle, PHY_PG4_PTP_CTL);
//	ptp_control_reg = ptp_control_reg >> 16;
//	ptp_control_reg |= P640_PTP_RD_CLK;
//	EPLWriteReg(epl_port_handle, PHY_PG4_PTP_CTL, ptp_control_reg);
	EPLWriteReg(epl_port_handle, PHY_PG4_PTP_CTL, P640_PTP_RD_CLK);
    
    *retNumberOfNanoSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_TDR) >> 16;

    *retNumberOfNanoSeconds |= EPLReadReg( epl_port_handle, PHY_PG4_PTP_TDR);

    *retNumberOfSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_TDR) >> 16;

    *retNumberOfSeconds |= EPLReadReg( epl_port_handle, PHY_PG4_PTP_TDR);

    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
}

void PTPClockStepAdjustment (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t numberOfSeconds,
        uint32_t numberOfNanoSeconds,
        boolean negativeAdj)
//  A step adjustment value is added to the current IEEE 1588 hardware clock 
//  value. Note that the adjustment value can be positive or negative.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  numberOfSeconds
//      The number of seconds to add to the current clock value. This should 
//      always be a positive value, use the negativeAdj flag to indicate a 
//      negative overall value.
//  numberOfNanoSeconds
//      The number of nanoseconds to add to the current clock value. This 
//      should always be a positive value, use the negativeAdj flag to 
//      indicate a negative overall value. This value must NOT be larger then 
//      10^9 (1 second).
//  negativeAdj
//      If TRUE, the numberOfSeconds and numberOfNanoSeconds values will be 
//      subtracted from the current clock value, otherwise the values will be 
//      an added to the clock. 
//
//  Returns
//      Nothing
//
//  This function does not account for the underlying time required within the 
//  hardware to make the adjustment (2 clock periods - 16ns). The caller should
//  adjust the adjustment value by subtracting 16ns from the passed in
//  value.
//****************************************************************************
{
    if ( negativeAdj)
    {
        // Convert to 2's complement
        numberOfSeconds = (uint32_t)((0x100000000 - (long long)numberOfSeconds) & 0xFFFFFFFF);
        numberOfNanoSeconds = (uint32_t)((0x100000000 - numberOfNanoSeconds) & 0xFFFFFFFF);
    }
        
    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfNanoSeconds & 0xFFFF);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfNanoSeconds >> 16);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfSeconds & 0xFFFF);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfSeconds >> 16);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_CTL, P640_PTP_STEP_CLK);
    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
}

void PTPClockSet (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t numberOfSeconds,
        uint32_t numberOfNanoSeconds)
//  Sets the IEEE 1588 hardware clock equal to the specified time value.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  numberOfSeconds
//      The number of seconds to set the current clock value too.
//  numberOfNanoSeconds
//      The number of nanoseconds to set the current clock value too. This 
//      value must NOT be larger then 10^9 (1 second).
//
//  Returns
//      Nothing
//****************************************************************************
{
    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfNanoSeconds & 0xFFFF);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfNanoSeconds >> 16);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfSeconds & 0xFFFF);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, numberOfSeconds >> 16);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_CTL, P640_PTP_LOAD_CLK);

//	uint32_t ptp_control_reg = EPLReadReg(epl_port_handle, PHY_PG4_PTP_CTL);
//	ptp_control_reg = ptp_control_reg >> 16;
//	ptp_control_reg |= P640_PTP_LOAD_CLK;
//	EPLWriteReg(epl_port_handle, PHY_PG4_PTP_CTL, ptp_control_reg);
    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
}

void PTPClockSetRateAdjustment (
		PEPL_PORT_HANDLE epl_port_handle,
        uint32_t rateAdjValue,
        boolean tempAdjFlag,
        boolean adjDirectionFlag)
//  The clock can be programmed to operate at an adjusted frequency value by 
//  programming a rate adjustment value. The rate adjustment allows for 
//  correction on the order of 2-32ns per reference clock cycle. The frequency 
//  adjustment will allow the clock to correct the offset over time, avoiding 
//  any potential side-effects caused by a step adjustment in the time value. 
//  The rate adjustment can be the normal adjustment rate that will always be 
//  used, and a temporary rate adjustment can be specified that will only occur 
//  for a preprogrammed amount of time.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  rateAdjValue
//      This 26-bit magnitude is the number of 2^-32 ns units that should be 
//      added to or subtracted from the IEEE 1588 hardware clock per reference 
//      cycle (8ns).
//  tempAdjFlag
//      If set to TRUE the set rate will override any previously set normal 
//      adjustment value for the amount of time specified using the 
//      PTPSetTempRateDurationConfig() function.  If this is FALSE, this 
//      function sets the normal clock adjustment time.
//  adjDirectionFlag
//      If set to TRUE, this will cause the PTP Clock to operate at a lower
//      frequency than the reference. The rateAdjValue value will be 
//      subtracted from the clock on every cycle. If set to FALSE, the 
//      rateAdjValue will be added to the clock on every cycle thus causing 
//      the IEEE 1588 hardware clock to operate at a higher frequency.
//
//  Returns
//      Nothing
//  
//  This function allows setting two different rate adjustment values, a 
//  Normal Rate and a Temporary Rate. The Temporary Rate allows the 1588 
//  Clock to operate at a modified rate for a programmed amount of time, 
//  as defined with PTPSetTempRateDurationConfig(). The Normal Rate will 
//  be selected and used if a Temporary Rate is not currently active.
//  
//  When setting a rate, the tempAdjFlag parameter indicates the rate is to
//  be temporary. Following completion of the time duration the rate will 
//  revert back to the Normal Rate. Note that the Normal Rate may be changed 
//  while a Temporary Rate is active. This will have not effect on the 
//  Temporary Rate, but the new Normal Rate will be used when the Temporary 
//  Rate Duration completes.
//  
//  To adjust the time value, software uses the 1588 protocol to determine 
//  the time correction required. The time correction may be spread over 
//  multiple clock cycles by programming a Temporary Rate value. To determine 
//  the rate setting, software should compute the rate difference as the time 
//  correction divided by the time duration in number of 8ns clock cycles. 
//  This value should be multiplied by 2^32 to convert to the correct units. 
//  This rate difference should then be added to the current PTP Rate setting 
//  to provide the Temporary Rate.
//  
//  The Temporary Rate value is a 26-bit value plus a sign bit (adjDirectionFlag), 
//  providing a range of -(2^26-1) to +(2^26-1) in units of 2^-32ns/cycle. 
//  Since each reference clock cycle is 8ns, this allows for a rate adjustment 
//  maximum of approximately +/-1950ppm.
//  
//  The Temporary Rate duration is a 26-bit value providing a duration of up 
//  to 536ms.
//  
//  Example:
//      Conditions:
//          Current Rate = +10 ppm, which gives PTP_Rate = 343597 (2^-32ns/cycle)
//          Time_Error = 20ns, gives Time_Corr = -20ns
//          Assume the correction will be done over 1ms, gives
//          Temp_Rate_duration = 1ms/8ns = 125,000
//  
//      Calculation:
//          Temp_Rate_delta = (Time_Corr/Temp_Rate_duration) * 2^32
//                  = (-20/125000) * 2^32 = -687194
//          Temp_Rate = Current_Rate + Temp_Rate_delta = 343597 + -687194 = -343597
//****************************************************************************
{
	int32_t reg = 0;
	rateAdjValue *= 100;
    reg = (rateAdjValue >> P640_PTP_RATE_HI_SHIFT) & P640_PTP_RATE_HI_MASK;
    if ( tempAdjFlag) reg |= P640_PTP_TMP_RATE;
    if (adjDirectionFlag) reg |= P640_PTP_RATE_DIR;
    
    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_RATEH, reg);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_RATEL, rateAdjValue & 0xFFFF);
    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
}

uint32_t PTPCheckForEvents (PEPL_PORT_HANDLE epl_port_handle)
//  Checks to determine if any of the following hardware events are 
//  outstanding: Transmit timestamp, receive timestamp, trigger done and event 
//  timestamp.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//
//  Returns
//      A bit map of zero or more bits set indicating the types of events that 
//      are available from the hardware. The defined bits are:
//          PTPEVT_TRANSMIT_TIMESTAMP_BIT
//          PTPEVT_RECEIVE_TIMESTAMP_BIT
//          PTPEVT_EVENT_TIMESTAMP_BIT
//          PTPEVT_TRIGGER_DONE_BIT
//
//  This must be called prior to retrieving individual events from the 
//  hardware. The bit map must be used to determine which "Get" functions 
//  need to be called. The applicable "Get" functions are:
//
//      PTPGetTransmitTimestamp
//      PTPGetReceiveTimestamp
//      PTPGetEvent
//      PTPHasTriggerExpired
//
//  It is NOT necessary to call this function prior to calling 
//  PTPHasTriggerExpired, although this function can be useful in a main 
//  polling loop to quickly determine if there is an expired trigger as well 
//  as the other types of events with a single call.
//****************************************************************************
{
	uint32_t reg, eventFlags;

	reg = EPLReadReg( epl_port_handle, PHY_PG4_PTP_STS);
    eventFlags = 0;
    if ( reg & P640_TXTS_RDY) eventFlags |= PTPEVT_TRANSMIT_TIMESTAMP_BIT;
    if ( reg & P640_RXTS_RDY) eventFlags |= PTPEVT_RECEIVE_TIMESTAMP_BIT;
    if ( reg & P640_TRIG_DONE) eventFlags |= PTPEVT_TRIGGER_DONE_BIT;
    if ( reg & P640_EVENT_RDY) eventFlags |= PTPEVT_EVENT_TIMESTAMP_BIT;
    return eventFlags;
}

void PTPGetTransmitTimestamp (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t *retNumberOfSeconds,
        uint32_t *retNumberOfNanoSeconds,
        uint32_t *overflowCount)
//  Returns the next available transmit timestamp.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function. 
//  retNumberOfSeconds
//      Will be set on return to the next transmit's seconds timestamp value.
//  retNumberOfNanoSeconds
//      Will be set on return to the next transmit's nanoseconds timestamp 
//      value. This value will never be larger then 10^9 (1 second).
//  overflowCount
//      Will be set on return and indicates if timestamps were dropped due to 
//      an overflow of the Transmit Timestamp queue. The overflow counter 
//      will stick at a value of three if more then three timestamps were 
//      missed. Normally this value should be 0.
//
//  Returns
//      Nothing
//  
//  The caller must have previously called PTPCheckForEvents() and determined 
//  that the PTPEVT_TRANSMIT_TIMESTAMP_BIT bit was set prior to invoking this 
//  function. This function does NOT check to determine if an outstanding 
//  transmit event is available.
//  
//  The hardware can queue up to four transmit timestamps. If more then four 
//  transmits occur without first reading the transmit timeout, then the 
//  overflowCount will indicate (up to 3) the number of timestamps that were 
//  dropped due to overflow.
//  
//  This function does not make the necessary adjustment to the timestamp to 
//  account for delay from the timestamp point to the wire (approximately 
//  6 bit times). The caller should adjust the returned value by adding 
//  10ns to the timestamp value.
//****************************************************************************
{
	uint32_t reg;

    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    *retNumberOfNanoSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_TXTS);

    reg = EPLReadReg( epl_port_handle, PHY_PG4_PTP_TXTS);
    *overflowCount = (reg & 0xC000) >> 14;
    *retNumberOfNanoSeconds |= (reg & 0x3FFF) << 16;

    *retNumberOfSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_TXTS);

    *retNumberOfSeconds |= EPLReadReg( epl_port_handle, PHY_PG4_PTP_TXTS) << 16;
    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
}

void PTPGetReceiveTimestamp (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t *retNumberOfSeconds,
        uint32_t *retNumberOfNanoSeconds,
        uint32_t *overflowCount,
        uint32_t *sequenceId,
        uint8_t *messageType,
        uint32_t *hashValue)
//  Returns the next available receive timestamp from the device's receive 
//  timestamp queue.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  retNumberOfSeconds
//      Will be set on return to the next receive's seconds timestamp value.
//  retNumberOfNanoSeconds
//      Will be set on return to the next receive's nanoseconds timestamp 
//      value. This value will never be larger then 1e9 (1 second).
//  overflowCount
//      Will be set on return and indicates if timestamps were dropped due to 
//      an overflow of the Receive Timestamp queue. The overflow counter 
//      will stick at a value of three if more then three timestamps were 
//      missed. Normally this value should be 0.
//  sequenceId
//      Will be set on return to the 16-bit SequenceId field from the incoming 
//      PTP frame.
//  messageType
//      Will be set on return to the messageType field from the incoming PTP 
//      frame. For version 1 of the IEEE 1588 specification the Timestamp unit 
//      will return the lower four bits of the control field (octet 32 in the 
//      message). Otherwise, the Timestamp unit will record the version 2 
//      messageType field which is the least significant bits of the first 
//      octet in the PTP message.
//  hashValue
//      Will be set on return to a 12-bit hash value on octets 20-29 of the 
//      PTP event message. For version 1 of the IEEE 1588 specification, 
//      this corresponds to the messageType, sourceCommunicationTechnology, 
//      sourceUuid, and sourcePortId fields. For version 2 of the IEEE 1588 
//      specification, this corresponds to the 10-octet sourcePortIdentity 
//      field. The combination of hash value and sequenceId, allows software 
//      to correctly match a Timestamp with the correct receive event message.
//
//      The hash algorithm used is the CRC function as defined in section 
//      3.2.8 the IEEE 802.3 specification. The Timestamp unit returns the 
//      12 most significant bits as the CRC computation (the resultant bits 
//      are not complemented as done in the 802.3 CRC generation).
//
//      To minimize unnecessary timestamp capture, the device may 
//      be configured to filter based on the source identification hash value.
//      This value may be programmed using the PTPSetMiscConfig() call with 
//      relevance to the ptpRxHash parameter. If the source hash value for 
//      the incoming PTP event message does not match the programmed hash 
//      value, then the message will not be timestamped.
//
//  Returns
//      Nothing
//  
//  The caller must have previously called PTPCheckForEvents() and determined 
//  that the PTPEVT_RECEIVE_TIMESTAMP_BIT bit was set prior to invoking this 
//  function. This function does NOT check to determine if an outstanding 
//  receive event is available.
//  
//  The hardware can queue up to four receive timestamps.
//  
//  This function does not adjust the timestamp to account for delay from
//  the wire. The caller should make the necessary adjustment to the timestamp to 
//  account for delay from the wire to the timestamp point (approximately 
//  26 bit times) by subtracting 210ns from the returned value.
//****************************************************************************
{
	uint32_t reg = 0;

    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);

    *retNumberOfNanoSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_RXTS);

    reg = EPLReadReg( epl_port_handle, PHY_PG4_PTP_RXTS);
    *overflowCount = (reg & 0xC000) >> 14;
    *retNumberOfNanoSeconds |= (reg & 0x3FFF) << 16;

    *retNumberOfSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_RXTS);

    *retNumberOfSeconds |= EPLReadReg( epl_port_handle, PHY_PG4_PTP_RXTS) << 16;

    *sequenceId = EPLReadReg( epl_port_handle, PHY_PG4_PTP_RXTS);
    reg = EPLReadReg( epl_port_handle, PHY_PG4_PTP_RXTS);

    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);

    *messageType = reg >> 12;
    *hashValue = reg & 0x0FFF;
}

void PTPGetTimestampFromFrame (uint8_t *receiveFrameData,
        uint32_t *retNumberOfSeconds,
        uint32_t *retNumberOfNanoSeconds)
//  Extracts the embedded receive timestamp from a received PTP frame.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  receiveFrameData
//      Points to the start of the PTP header.
//  retNumberOfSeconds
//      Will be set on return to the next receive's seconds timestamp value. 
//      The number of significant bits of magnitude returned is determined by 
//      the RXOPT_TS_SEC_EN option bit used during the call to 
//      PSPSetReceiveConfig().
//  retNumberOfNanoSeconds
//      Will be set on return to the next receive's nanoseconds timestamp 
//      value. This value will never be larger then 10^9 (1 second).
//
//  Returns
//      Nothing
//  
//  This function can be used when receive timestamp insertion has been 
//  enabled using the PSPSetReceiveConfig() function with the RXOPT_TS_INSERT 
//  option set. It extracts the embedded timestamp from the frame, either from 
//  the end of the frame if RXOPT_TS_APPEND is enabled, or from the configured 
//  locations within the PTP frame itself. 
//  
//  If RXOPT_TS_APPEND is disabled, the relevant reserved fields are set to a 
//  value of 0.
//  
//  This function does not adjust the timestamp to account for delay from
//  the wire. The caller should make the necessary adjustment to the timestamp to 
//  account for delay from the wire to the timestamp point (approximately 
//  26 bit times) by subtracting 210ns from the returned value.
//****************************************************************************
{
	uint8_t *nanoField, *secField;
	uint32_t opts;

    opts = rxCfgOpts;
    if ( !(opts & RXOPT_TS_INSERT))
        return;

    if ( opts & RXOPT_TS_APPEND)
    {
        // Timestamp is located at the end of the PTP packet at the configured
        // offsets from the end of the PTP message.
        nanoField = &receiveFrameData[ PTP_EVENT_PACKET_LENGTH + rxCfgItems.rxTsNanoSecOffset ];
        secField = &nanoField[ rxCfgItems.rxTsSecondsOffset ];
    }
    else
    {
        // Timestamp is located at the configured offsets from the start of the
        // PTP message.
        nanoField = &receiveFrameData[ rxCfgItems.rxTsNanoSecOffset ];
        secField = &receiveFrameData[ rxCfgItems.rxTsSecondsOffset ];
    }

    *retNumberOfNanoSeconds = *(uint32_t*)nanoField;
    *(uint32_t*)nanoField = 0L;

    if ( rxCfgItems.tsSecLen == 0)
    {
        *retNumberOfSeconds = (uint32_t)*secField;
        *secField = 0;
    }
    else if ( rxCfgItems.tsSecLen == 1)
    {
        *retNumberOfSeconds = (uint32_t)*(uint16_t*)secField;
        *(uint16_t*)secField = 0;
    }
    else if ( rxCfgItems.tsSecLen == 2)
    {
        *retNumberOfSeconds = 0x00FFFFFF & *(uint32_t*)secField;
        *(uint16_t*)secField = 0;
        secField[2] = 0;
    }
    else if ( rxCfgItems.tsSecLen == 3)
    {
        *retNumberOfSeconds = *(uint32_t*)secField;
        *(uint32_t*)secField = 0;
    }
}

void PTPArmTrigger (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t trigger,
        uint32_t expireTimeSeconds,
        uint32_t expireTimeNanoSeconds,
        boolean initialStateFlag,
        boolean waitForRolloverFlag,
        uint32_t pulseWidth,
        uint32_t pulseWidth2)
//  Establishes a trigger's expiration time and trigger pulse width behavior.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  trigger
//      The trigger to arm, 0 - 7.
//  expireTimeSeconds
//      The seconds portion of the desired expiration time relative to the 
//      IEEE 1588 hardware clock. 
//  expireTimeNanoSeconds
//      The nanoseconds portion of the desired expiration time relative to 
//      the IEEE 1588 hardware clock. This value may be not be larger then 
//      1e9 (1 second).
//  initialStateFlag
//      Indicates initial state of signal to be set when trigger is armed 
//      (FALSE will cause a signal rise at trigger time, TRUE will cause a 
//      signal fall at trigger time). This parameter is ignored in toggle 
//      mode.
//  waitForRolloverFlag
//      If set to TRUE the device should not arm the trigger until after 
//      the seconds field of the clock time has rolled over from 
//      0xFFFF_FFFF to 0.
//  pulseWidth
//      Sets the 50% duty cycle time for triggers 2 - 7. Its format is 
//      bits [31:30] = seconds, [29:0] = nanoseconds. For triggers 0 and 1 
//      sets the width of the first part of the pulse, the width of the 
//      second part of the pulse is set by the pulseWidth2 parameter.
//  pulseWidth2
//      Ignored for triggers 2 - 7. For Triggers 0 and 1, in a single or 
//      periodic pulse type signal, a second pulse width value controls the 
//      2nd pulse width (period is pulseWidth + pulseWidth2). 
//      Its format is bits [31:30] = seconds, [29:0] = nanoseconds.
//
//      For Edge type signals, pulseWidth2 is interpreted as a 16-bit seconds 
//      field and pulseWidth is a 30-bit nanoseconds field. 
//
//  Returns
//      Nothing
//
//  Once this function has been called, the trigger will be armed. 
//****************************************************************************
{
	uint32_t reg;

    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    reg = (trigger << P640_TRIG_SEL_SHIFT) | P640_TRIG_LOAD;
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_CTL, reg);
    
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, expireTimeNanoSeconds & 0xFFFF);
    
    reg = (expireTimeNanoSeconds >> 16) | (initialStateFlag ? 0x80000000 : 0) |
          (waitForRolloverFlag ? 0x40000000 : 0);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, reg);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, expireTimeSeconds & 0xFFFF);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, expireTimeSeconds >> 16);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, pulseWidth & 0xFFFF);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, pulseWidth >> 16);
    
    if ( trigger <= 1)
    {
        EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, pulseWidth2 & 0xFFFF);
        EPLWriteReg( epl_port_handle, PHY_PG4_PTP_TDR, pulseWidth2 >> 16);
    }
    
    reg = (trigger << P640_TRIG_SEL_SHIFT) | P640_TRIG_EN;
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_CTL, reg);
    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
}
 
boolean PTPHasTriggerExpired (PEPL_PORT_HANDLE epl_port_handle, uint32_t trigger)
//  Determines if a particular trigger has occurred or if an error has 
//  occurred. An interrupt mechanism may also be used to detect trigger 
//  expiration.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  trigger
//      The trigger to check, 0 - 7.
//
//  Returns
//      NS_STATUS_SUCCESS
//          The specified trigger has expired successfully.
//      NS_STATUS_FAILURE
//          The specified trigger has not yet expired.
//      NS_STATUS_INVALID_PARM
//          The specified trigger was armed late and expired. For a periodic 
//          signal, if the Trigger-if-late control is set, this function will 
//          return NS_STATUS_SUCCESS. If the Trigger-if-late is not set, this 
//          status return code will be returned.
//
//  It is NOT necessary to call PTPCheckForEvents() prior to invocation.
//****************************************************************************
{
	uint32_t reg, trgBit;

    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    reg = EPLReadReg( epl_port_handle, PHY_PG4_PTP_TSTS);
    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
    
    trgBit = 1 << (trigger * 2);
    if ( reg & trgBit)
        return NS_STATUS_FAILURE;
    if ( reg & (trgBit << 1))
        return NS_STATUS_INVALID_PARM;
    return NS_STATUS_SUCCESS;
}

void PTPCancelTrigger (PEPL_PORT_HANDLE epl_port_handle, uint32_t trigger)
//  Cancels a previously schedule trigger event (if active).
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  trigger
//      The trigger to cancel, 0 - 7.
//
//  Returns
//      Nothing
//  
//  It is possible that a trigger will expire before this function can cancel 
//  it, depending on how close the expiration time is to the current IEEE 
//  1588 time.
//****************************************************************************
{
	uint32_t reg;
    reg = (trigger << P640_TRIG_SEL_SHIFT) | P640_TRIG_DIS;
    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    EPLWriteReg( epl_port_handle, PHY_PG4_PTP_CTL, reg);
    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
}

boolean PTPGetEvent (PEPL_PORT_HANDLE epl_port_handle,
        uint32_t *eventBits,
        uint32_t *riseFlags,
        uint32_t *eventTimeSeconds,
        uint32_t *eventTimeNanoSeconds,
        uint32_t *eventsMissed)
//  Returns the available asynchronous event timestamps.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//  eventBits
//      Set on return to a bit map of events that have occurred. Bit 0 is 
//      event 0, etc. 0 or more bits may be set.
//  riseFlags
//      Set on return to a bit map indicating if an event occurred on the 
//      rising edge (set to 1), or on the falling edge (set to 0). Bit 0 is
//      is the rising edge flag for event 0, etc.
//  eventTimeSeconds
//      The seconds portion of the IEEE 1588 clock timestamp when this event 
//      occurred. 
//  eventTimeNanoSeconds
//      The nanosecond portion of the IEEE 1588 clock timestamp when this 
//      event occurred. This value will not be larger then 1e9 (1 second).
//  eventsMissed
//      Set on return to indicate the number of events that have been missed 
//      prior to this event due to internal event queue overflow. The maximum 
//      value is 7.
//
//  Returns
//      TRUE if an event was returned, FALSE otherwise.
//  
//  The caller must have previously called PTPCheckForEvents() and determined 
//  that the PTPEVT_EVET_TIMESTAMP_BIT bit was set prior to invoking this 
//  function. This function does NOT check to determine if an outstanding 
//  event is available.
//  
//  This function properly tracks and handles events that occur at the same 
//  exact time. It also adjusts the timestamp values to compensate for input 
//  path and synchronization delays.
//****************************************************************************
{
	uint32_t reg, exSts, x;

    *eventBits = 0;
    *riseFlags = 0;

    OAIBeginMultiCriticalSection( epl_port_handle->oaiDevHandle);
    reg = EPLReadReg( epl_port_handle, PHY_PG4_PTP_ESTS);
    
    *eventsMissed = (reg & P640_EVNTS_MISSED_MASK) >> P640_EVNTS_MISSED_SHIFT;
    
    if ( !(reg & P640_EVENT_DET))
    {
        OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
        return FALSE;
    }
        
    if ( reg & P640_MULT_EVENT)
    {
        exSts = EPLReadReg( epl_port_handle, PHY_PG4_PTP_EDATA);
        
        for ( x = 8; x; x--)
        {
            if ( exSts & 0x40)
                *eventBits |= 1 << (x-1);
                if ( exSts & 0x80)
                    *riseFlags |= 1 << (x-1);    
            exSts <<= 2;
        }
    }
    else    
    {
        *eventBits |= 1 << ((reg & P640_EVNT_NUM_MASK) >> P640_EVNT_NUM_SHIFT);
        *riseFlags |= ((reg & P640_EVNT_RF) ? 1 : 0) << ((reg & P640_EVNT_NUM_MASK) >> P640_EVNT_NUM_SHIFT);
    }
    
    *eventTimeNanoSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_EDATA);

    *eventTimeNanoSeconds |= EPLReadReg( epl_port_handle, PHY_PG4_PTP_EDATA) << 16;

    *eventTimeSeconds = EPLReadReg( epl_port_handle, PHY_PG4_PTP_EDATA);

    *eventTimeSeconds |= EPLReadReg( epl_port_handle, PHY_PG4_PTP_EDATA) << 16;

    OAIEndMultiCriticalSection( epl_port_handle->oaiDevHandle);
    
    // Adj for pin input delay and edge detection time
	if( *eventTimeNanoSeconds < PIN_INPUT_DELAY )
	{
		if( *eventTimeSeconds > 0 )
		{
	        *eventTimeSeconds -= 1;
			*eventTimeNanoSeconds += ((NS_UINT)1e9 - PIN_INPUT_DELAY);
		}
		else
			*eventTimeSeconds = *eventTimeNanoSeconds = 0;
	}
	else
		*eventTimeNanoSeconds -= PIN_INPUT_DELAY;

	return TRUE;
}

uint32_t MonitorGpioSignals (PEPL_PORT_HANDLE epl_port_handle)
//  Provides a function to read the current status of the device's GPIO 
//  signals.
//  
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort 
//      function.
//
//  Returns
//      Returns bits [11:0] reflecting the current values on the 12 GPIOs. 
//      GPIO 12 is bit 11, etc.
//****************************************************************************
{
    return EPLReadReg( epl_port_handle, PHY_PG6_PTP_GPIOMON);
}

uint8_t * IsPhyStatusFrame (PEPL_PORT_HANDLE epl_port_handle,
        uint8_t *frameBuffer,
        uint16_t frameLength)
//  Determines if the specified receive frame is a specially formatted IEEE
//  1588 Phy Status Frame (PSF).
//
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort
//      function.
//  frameBuffer
//      Pointer to a byte array containing a pointer to the incoming IEEE
//      1588 frame. Where in the frame this points is defined by the
//      offsetLocation parameter. This should point to the start of the
//      Ethernet frame.
//  frameLength
//      The number of bytes pointed to by frameBuffer.
//
//  Returns
//      If a valid message is found a pointer to the Phy message that
//      should be processed will be returned.  Otherwise NULL will be
//      returned.
//
//  This function used the previously configured values for the IEEE 1588
//  header transportSpecific, messageType and versionPTP to determine if the
//  frame is a Phy Status Frame. Refer to PTPSetPhyStatusFrameConfig().
//
//  If the frame is a PSF message, the caller should use the
//  GetNextPhyMessage() to obtain the message's information.
//****************************************************************************
{
	static uint8_t ipDestAddr[6] = {0x01,0x00,0x5E,0x00,0x01,0x81};
	static uint8_t macDestAddr[6] = {0x01,0x1B,0x19,0x00,0x00,0x00};

    if ( epl_port_handle->psfConfigOptions & STSOPT_IPV4)
    {
//         IPv4/UDP Frame
        if ( frameLength < 50)
            return epl_port_handle->psfList;

        if ( *(uint32_t*)&frameBuffer[0] != *(uint32_t*)&ipDestAddr[0] ||
             *(uint16_t*)&frameBuffer[4] != *(uint16_t*)&ipDestAddr[4])
            return epl_port_handle->psfList;

        if ( *(uint32_t*)&frameBuffer[6] != *(uint32_t*)&epl_port_handle->psfSrcMacAddr[0] ||
             *(uint16_t*)&frameBuffer[10] != *(uint16_t*)&epl_port_handle->psfSrcMacAddr[4])
            return epl_port_handle->psfList;

        if ( frameBuffer[12] != 0x08 || frameBuffer[13] != 0x00)
            return epl_port_handle->psfList;

        return &frameBuffer[44];
    }
    else
    {
        // Layer 2 Ethernet Frame
        if ( frameLength < 22)
            return epl_port_handle->psfList;

        if ( *(uint32_t*)&frameBuffer[0] != *(uint32_t*)&macDestAddr[0] ||
             *(uint16_t*)&frameBuffer[4] != *(uint16_t*)&macDestAddr[4])
            return epl_port_handle->psfList;

        if ( *(uint32_t*)&frameBuffer[6]  != *(uint32_t*)&epl_port_handle->psfSrcMacAddr[0] ||
             *(uint16_t*)&frameBuffer[10] != *(uint16_t*)&epl_port_handle->psfSrcMacAddr[4])
            return epl_port_handle->psfList;

        if ( frameBuffer[12] != 0x88 || frameBuffer[13] != 0xF7)
            return epl_port_handle->psfList;

        return &frameBuffer[16];
    }
}

uint16_t ByteSwap16(uint8_t  lEndian, uint16_t wordIn)
// Helper Functions to handle big vs little endian
//
// lEndian
//      Flag that tells whether or not bytes need to be swapped
//      This is derived from the PTP_ENDIAN setting in the part
// wordIn
//      16bit value that needs to be swapped
//
// Returns
//      Swapped 16bit value
//****************************************************************************
{
    if( lEndian )
        return wordIn;
    else
        return (((wordIn) >> 8) & 0x00ff) | (((wordIn) << 8) & 0xff00);
}

uint32_t ByteSwap32(uint8_t  lEndian, uint32_t dwordIn)
// Helper Functions to handle big vs little endian
//
// lEndian
//      Flag that tells whether or not bytes need to be swapped
//      This is derived from the PTP_ENDIAN setting in the part
// dwordIn
//      32bit value that needs to be swapped
//
// Returns
//      Swapped 32bit value
//****************************************************************************
{
    if( lEndian )
        return dwordIn;
    else
        return  (((dwordIn) >> 24) & 0x000000ff) | (((dwordIn) >> 8 ) & 0x0000ff00) |
                (((dwordIn) << 8 ) & 0x00ff0000) | (((dwordIn) << 24) & 0xff000000);
}

uint8_t * intGetNextPhyMessage (PEPL_PORT_HANDLE epl_port_handle,
        uint8_t *msgLocation,
        PHYMSG_MESSAGE_TYPE_ENUM *messageType,
        PHYMSG_MESSAGE *phyMessageOut,
        boolean usePSFList)
//  See below for details of the call.  This is the same as the exported call
//  except it adds an additional flag usePSFList which controls whether or not
//  the PSFList is cleared.  This helps in the creation/parsing of the messages
//  list.
//****************************************************************************
{
	uint16_t val, stsType;
	uint16_t *ptr;
	uint8_t *msg = msgLocation;
	PHYMSG_MESSAGE *message = (PHYMSG_MESSAGE *)phyMessageOut;
	uint8_t nextMsgOffset = 0;
	uint8_t lEndian = epl_port_handle->psfConfigOptions & STSOPT_LITTLE_ENDIAN;
	PHYMSG_LIST *psfList;

    // Clear out the PSF List first
    if( usePSFList && epl_port_handle->psfList ) {
        psfList = (PHYMSG_LIST *)epl_port_handle->psfList;
        *messageType = psfList->msgType;
        memcpy( phyMessageOut, &psfList->phyMsg, sizeof( PHYMSG_MESSAGE ) );
        epl_port_handle->psfList = psfList->nxtMsg;
        OAIFree( psfList );
        free( psfList );
        return msgLocation; // Didn't do anything with it, send it back around
    } // if( portHdl->psfList )

    *messageType = 0xFF;

    // First check for termination field (4 zero octets)
    if ( *(uint32_t*)msg == 0x00000000)
        return NULL;

    stsType = ByteSwap16( lEndian, *(uint16_t*)msg);

    switch ( stsType & 0xF000)
    {
        case 0x1000:
            // Tx timestamp message
            *messageType = PHYMSG_STATUS_TX;

            message->TxStatus.txTimestampSecs  = ByteSwap16( lEndian, *(uint16_t*)&msg[6]);
            message->TxStatus.txTimestampSecs |= ByteSwap16( lEndian, *(uint16_t*)&msg[8]) << 16;
            val = ByteSwap16( lEndian, *(uint16_t*)&msg[4]);
            message->TxStatus.txTimestampNanoSecs = ByteSwap16( lEndian, *(uint16_t*)&msg[2]);
            message->TxStatus.txTimestampNanoSecs |= (val & ~0xC000) << 16;
            message->TxStatus.txOverflowCount = val >> 14;
            nextMsgOffset = 5 * sizeof( uint16_t);
            break;

        case 0x2000:
            // Rx timestamp message
            *messageType = PHYMSG_STATUS_RX;

            message->RxStatus.rxTimestampSecs  = ByteSwap16( lEndian, *(uint16_t*)&msg[6]);
            message->RxStatus.rxTimestampSecs |= ByteSwap16( lEndian, *(uint16_t*)&msg[8]) << 16;
            val = ByteSwap16( lEndian, *(uint16_t*)&msg[4]);
            message->RxStatus.rxTimestampNanoSecs = ByteSwap16( lEndian, *(uint16_t*)&msg[2]);
            message->RxStatus.rxTimestampNanoSecs |= (val & ~0xC000) << 16;
            message->RxStatus.rxOverflowCount = val >> 14;

            message->RxStatus.sequenceId = ByteSwap16( lEndian, *(uint16_t*)&msg[10]);
            val = ByteSwap16( lEndian, *(uint16_t*)&msg[12]);
            message->RxStatus.messageType = val >> 12;
            message->RxStatus.sourceHash = val & 0x0FFF;
            nextMsgOffset = 7 * sizeof( uint16_t);
            break;

        case 0x3000:
            // Trigger message
            *messageType = PHYMSG_STATUS_TRIGGER;
            message->TriggerStatus.triggerStatus = ByteSwap16( lEndian, *(uint16_t*)&msg[2]);
            nextMsgOffset = 2 * sizeof( uint16_t);
            break;

        case 0x4000:
            // Event timestamp message
            *messageType = PHYMSG_STATUS_EVENT;

            message->EventStatus.ptpEstsRegBits = stsType & 0x0FFF;
            message->EventStatus.extendedEventStatusFlag = FALSE;

            val = (message->EventStatus.ptpEstsRegBits & P640_EVNTS_TS_LEN_MASK) >> P640_EVNTS_TS_LEN_SHIFT;

            if ( message->EventStatus.ptpEstsRegBits & P640_MULT_EVENT)
            {
                ptr = (uint16_t*)&msg[4];
                message->EventStatus.extendedEventStatusFlag = TRUE;
                message->EventStatus.extendedEventInfo = ByteSwap16( lEndian, *(uint16_t*)&msg[2]);
            }
            else
                ptr = (uint16_t*)&msg[2];

            message->EventStatus.evtTimestampSecs = 0;
            message->EventStatus.evtTimestampNanoSecs = 0;

            message->EventStatus.evtTimestampNanoSecs = ByteSwap16( lEndian, *ptr++);
            if ( val == 1) {
                message->EventStatus.evtTimestampNanoSecs |= ByteSwap16( lEndian, *ptr++) << 16;
            } else if ( val == 2) {
                message->EventStatus.evtTimestampNanoSecs |= ByteSwap16( lEndian, *ptr++) << 16;
                message->EventStatus.evtTimestampSecs = ByteSwap16( lEndian, *ptr++);
            } else if ( val == 3) {
                message->EventStatus.evtTimestampNanoSecs |= ByteSwap16( lEndian, *ptr++) << 16;
                message->EventStatus.evtTimestampSecs = ByteSwap16( lEndian, *ptr++);
                message->EventStatus.evtTimestampSecs |= ByteSwap16( lEndian, *ptr++) << 16;
            }

            nextMsgOffset = (NS_UINT8)((uint8_t *)ptr - msg);    // Calculate offset from beginning
            break;

        case 0x5000:
            // Status frame error status
            *messageType = PHYMSG_STATUS_ERROR;

            val = ByteSwap16( lEndian, *(uint16_t*)&msg[0]);
            message->ErrorStatus.frameBufOverflowFlag     = (val & 0x0FFF) == 0x00 ? TRUE : FALSE;
            message->ErrorStatus.frameCounterOverflowFlag = (val & 0x0FFF) == 0x01 ? TRUE : FALSE;
            nextMsgOffset = 1 * sizeof( uint16_t);
            break;

        case 0x6000:
            // Register read response
            *messageType = PHYMSG_STATUS_REG_READ;

            val = ByteSwap16( lEndian, *(uint16_t*)&msg[0]);

            message->RegReadStatus.regIndex = val & 0x001F;         // bits 4:0
            message->RegReadStatus.regPage  = (val & 0x00E0) >> 5;  // bits 7:5
            message->RegReadStatus.readRegisterValue = ByteSwap16( lEndian, *(uint16_t*)&msg[2]);
            //message->RegReadStatus.readRegisterValue = *(uint16_t*)&msg[2];
            nextMsgOffset = 2 * sizeof( uint16_t);
            break;

        default:
            // Do nothing will return NULL
            break;
    }

    if( !nextMsgOffset ) {
        // Next message offset still 0 no message processed and nothing
        // more to process.
        return NULL;
    }
    else {
        // Return address of the next message to check/process.
        return &msg[nextMsgOffset];
    }
}

uint8_t * GetNextPhyMessage (PEPL_PORT_HANDLE epl_port_handle,
        uint8_t *msgLocation,
        PHYMSG_MESSAGE_TYPE_ENUM *messageType,
        PHYMSG_MESSAGE *phyMessageOut)
//  Returns information about the next Phy message contained in a list of one
//  or more Phy message structures.
//
//  portHandle
//      Handle that represents a port. This is obtained using the EPLEnumPort
//      function.
//  msgLocation
//      Pointer to the PHY message to check/process.  This is initially
//      determined by calling IsPhyStatusFrame().  The caller should treat
//      this field opaquely.
//  messageType
//      Set on return to one of the PHYMSG_MESSAGE_TYPE_ENUM values indicating
//      the type of message structure returned in the message union buffer.
//  phyMessageOut
//      Caller allocated union/structure that will be filled out on return to
//      contain the relevant message information fields.
//
//  Returns
//      Returns a pointer to the PHY message location to be checked/processed.
//      Returns NULL if there are no further messages to process.
//****************************************************************************
{
    return intGetNextPhyMessage( epl_port_handle, msgLocation, messageType, phyMessageOut, 1 );
}
