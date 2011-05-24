#ifndef IEEE_802154_ENUM_H
#define IEEE_802154_ENUM_H

/********************************/
/**** PHY Layer Enumeration ****/
/******************************/

// Phy-Mac primitive type
enum Ieee802154MacPhyPrimitiveType
{
    //UNDEFINED                 = 0,
    PD_DATA_CONFIRM                 = 1,
    PLME_CCA_REQUEST                = 2,
    PLME_CCA_CONFIRM                = 3,
    PLME_ED_REQUEST                 = 4,
    PLME_ED_CONFIRM                 = 5,
    PLME_SET_TRX_STATE_REQUEST          = 6,
    PLME_SET_TRX_STATE_CONFIRM          = 7,
    PLME_SET_REQUEST                = 8,
    PLME_SET_CONFIRM                = 9,
    PLME_GET_BITRATE                = 10
};

//PHY enumerations description (Table 16)
typedef enum
{
    phy_BUSY = 1,
    phy_BUSY_RX = 2,
    phy_BUSY_TX = 3,
    phy_FORCE_TRX_OFF = 4,
    phy_IDLE = 5,
    phy_INVALID_PARAMETER = 6,
    phy_RX_ON = 7,
    phy_SUCCESS = 8,
    phy_TRX_OFF = 9,
    phy_TX_ON = 10,
    phy_UNSUPPORT_ATTRIBUTE = 11
} PHYenum;

// PHY PIB attributes
typedef enum
{
    PHY_CURRENT_CHANNEL = 0x01,
    PHY_CHANNELS_SUPPORTED,
    PHY_TRANSMIT_POWER,
    PHY_CCA_MODE
} PHYPIBenum;

// Phy timer type
enum Ieee802154PhyTimerType
{
    PHY_CCA_TIMER,
    PHY_ED_TIMER,
    PHY_TRX_TIMER,
    PHY_TX_OVER_TIMER,
    PHY_RX_OVER_TIMER               // dynamic timer
};

// packet error type
enum Ieee802154PktErrorType   // for msgKind set by PHY layer
{
    PACKETOK=0,
    COLLISION = 1,
    BITERROR,
    BITERROR_FORCE_TRX_OFF,
    RX_DURING_CCA                   // pkts received during CCA need to be discarded
};

typedef enum {
	TX_OVER = 0
} additionalData;
/********************************/
/****** MAC Layer Enumeration ******/
/*******************************/

//MAC enumerations description (Table 64)
typedef enum
{
    mac_SUCCESS = 0x00,

    //--- following from Table 68) ---
    // Association status
    mac_PAN_at_capacity,
    mac_PAN_access_denied,
    //--------------------------------

    mac_BEACON_LOSS = 0xe0,
    mac_CHANNEL_ACCESS_FAILURE,
    mac_DENIED,
    mac_DISABLE_TRX_FAILURE,
    mac_FAILED_SECURITY_CHECK,
    mac_FRAME_TOO_LONG,
    mac_INVALID_GTS,
    mac_INVALID_HANDLE,
    mac_INVALID_PARAMETER,
    mac_NO_ACK,
    mac_NO_BEACON,
    mac_NO_DATA,
    mac_NO_SHORT_ADDRESS,
    mac_OUT_OF_CAP,
    mac_PAN_ID_CONFLICT,
    mac_REALIGNMENT,
    mac_TRANSACTION_EXPIRED,
    mac_TRANSACTION_OVERFLOW,
    mac_TX_ACTIVE,
    mac_UNAVAILABLE_KEY,
    mac_UNSUPPORTED_ATTRIBUTE,
    mac_UNDEFINED           //we added this for handling any case not specified in the draft
} MACenum;

//MAC PIB attributes (Tables 71,72)
typedef enum
{
    //attributes from Table 71
    MAC_ACK_WAIT_DURATION,
    MAC_ASSOCIATION_PERMIT,
    MAC_AUTO_REQUEST,
    MAC_BATT_LIFE_EXT,
    MAC_BATT_LIFE_EXT_PERIODS,
    MAC_BEACON_PAYLOAD,
    MAC_BEACON_PAYLOAD_LENGTH,
    MAC_BEACON_ORDER,
    MAC_BEACON_TX_TIME,
    MAC_BSN,
    MAC_COORD_EXTENDED_ADDRESS,
    MAC_COORD_SHORT_ADDRESS,
    MAC_DSN,
    MAC_GTS_PERMIT,
    MAC_MAX_CSMA_BACKOFFS,
    MAC_MIN_BE,
    MAC_PAN_ID,
    MAC_PROMISCUOUS_MODE,
    MAC_RX_ON_WHEN_IDLE,
    MAC_SHORT_ADDRESS,
    MAC_SUPERFRAME_ORDER,
    MAC_TRANSACTION_PERSISTENCE_TIME
    //ATTRIBUTES FROM TABLE 72 (SECURITY ATTRIBUTES)
    /*
    MAC_ACL_ENTRY_DESCRIPTOR_SET,
    MAC_ACL_ENTRY_DESCRIPTOR_SET_SIZE,
    MAC_DEFAULT_SECURITY,
    MAC_ACL_DEFAULT_SECURITY_MATERIAL_LENGTH,
    MAC_DEFAULT_SECURITY_MATERIAL,
    MAC_DEFAULT_SECURITY_SUITE,
    MAC_SECURITY_MODE
    */
} MPIBAenum;

// MAC frame type - Table 65
typedef enum
{
    //Ieee802154_UNDEFINED_FRM  = 0,
    Ieee802154_BEACON       = 1,        // MAC Beacon
    Ieee802154_DATA         = 2,        // MAC Data
    Ieee802154_ACK          = 3,        // MAC ACK
    Ieee802154_CMD          = 4     // MAC command

} Ieee802154FrameType;

// MAC command type - Table 67
typedef enum
{
    //Ieee802154_UNDEFINED_CMD              = 0,
    Ieee802154_ASSOCIATION_REQUEST              = 1,
    Ieee802154_ASSOCIATION_RESPONSE             = 2,
    Ieee802154_DISASSOCIATION_NOTIFICATION          = 3,
    Ieee802154_DATA_REQUEST                 = 4,
    Ieee802154_PANID_CONFLICT_NOTIFICATION          = 5,
    Ieee802154_ORPHAN_NOTIFICATION              = 6,
    Ieee802154_BEACON_REQUEST               = 7,
    Ieee802154_COORDINATOR_REALIGNMENT          = 8,
    Ieee802154_GTS_REQUEST                  = 9

} Ieee802154MacCmdType;

// Pkt tx options
typedef enum
{
    //UNDEFINED         = 0,
    DIRECT_TRANS            = 1,
    INDIRECT_TRANS          = 2,
    GTS_TRANS           = 3

} Ieee802154TxOption;

// MAC timer type
enum Ieee802154MacTimerType
{
    START_PAN_COOR_TIMER,       // dynamic timer
    MAC_BACKOFF_TIMER,
    MAC_DEFER_CCA_TIMER,
    MAC_BCN_RX_TIMER,
    MAC_BCN_TX_TIMER,
    //MAC_TX_OVER_TIMER,
    MAC_ACK_TIMEOUT_TIMER,
    MAC_TX_ACK_BOUND_TIMER,
    MAC_TX_CMD_DATA_BOUND_TIMER,
    MAC_IFS_TIMER,
    MAC_TX_SD_TIMER,
    MAC_RX_SD_TIMER,
    MAC_FINAL_CAP_TIMER,
    MAC_GTS_TIMER
};

// Channel scan type - table 53
typedef enum
{
    //UNDEFINED         = 0,
    ED_SCAN             = 1,
    ACTIVE_SCAN         = 2,
    PASSIVE_SCAN            = 3,
    ORPHAN_SCAN         = 4

} Ieee802154ChannelScanType;

// MAC task pending type
typedef enum
{
    TP_MCPS_DATA_REQUEST = 1,
    TP_MLME_ASSOCIATE_REQUEST,
    TP_MLME_ASSOCIATE_RESPONSE,
    TP_MLME_DISASSOCIATE_REQUEST,
    TP_MLME_ORPHAN_RESPONSE,
    TP_MLME_RESET_REQUEST,
    TP_MLME_RX_ENABLE_REQUEST,
    TP_MLME_SCAN_REQUEST,
    TP_MLME_START_REQUEST,
    TP_MLME_SYNC_REQUEST,
    TP_MLME_POLL_REQUEST,
    TP_CCA_CSMACA,
    TP_RX_ON_CSMACA

} Ieee802154MacTaskType;


#endif
