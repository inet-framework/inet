#ifndef IEEE_802154_MAC_H
#define IEEE_802154_MAC_H

#include "RadioState.h"
#include "Ieee802154Const.h"
#include "Ieee802154Def.h"
#include "Ieee802154Enum.h"
#include "Ieee802154Field.h"
#include "Ieee802154Link.h"
#include "Ieee802154Frame_m.h"
#include "Ieee802154MacPhyPrimitives_m.h"
#include "Ieee802154NetworkCtrlInfo_m.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "IPassiveQueue.h"
#include "MACAddress.h"

/**
 * IEEE 802.15.4 Media Access Control (MAC) Layer
 * Refer to IEEE Std 802.15.4-2006
 *
 * @author Feng Chen
 * @ingroup macLayer
 */

struct taskPending
{
    taskPending()
    {
        init();
    }
    inline void init()
    {
        mcps_data_request = false;
        mcps_data_request_STEP = 0;
        /*mlme_associate_request = false;
        mlme_associate_request_STEP = 0;
        mlme_associate_response = false;
        mlme_associate_response_STEP = 0;
        mlme_disassociate_request = false;
        mlme_disassociate_request_STEP = 0;
        mlme_orphan_response = false;
        mlme_orphan_response_STEP = 0;
        mlme_reset_request = false;
        mlme_reset_request_STEP = 0;
        mlme_rx_enable_request = false;
        mlme_rx_enable_request_STEP = 0;
        mlme_scan_request = false;
        mlme_scan_request_STEP = 0;
        mlme_start_request = false;
        mlme_start_request_STEP = 0;
        mlme_sync_request = false;
        mlme_sync_request_STEP = 0;
        mlme_sync_request_tracking = false;
        mlme_poll_request = false;
        mlme_poll_request_STEP = 0;*/
        CCA_csmaca = false;
        CCA_csmaca_STEP = 0;
        RX_ON_csmaca = false;
        RX_ON_csmaca_STEP = 0;
    }

    bool &taskStatus(Ieee802154MacTaskType task)
    {
        switch (task)
        {
        case TP_MCPS_DATA_REQUEST:
            return mcps_data_request;
            /*case TP_MLME_ASSOCIATE_REQUEST:
                return mlme_associate_request;
            case TP_MLME_ASSOCIATE_RESPONSE:
                return mlme_associate_response;
            case TP_MLME_DISASSOCIATE_REQUEST:
                return mlme_disassociate_request;
            case TP_MLME_ORPHAN_RESPONSE:
                return mlme_orphan_response;
            case TP_MLME_RESET_REQUEST:
                return mlme_reset_request;
            case TP_MLME_RX_ENABLE_REQUEST:
                return mlme_rx_enable_request;
            case TP_MLME_SCAN_REQUEST:
                return mlme_scan_request;
            case TP_MLME_START_REQUEST:
                return mlme_start_request;
            case TP_MLME_SYNC_REQUEST:
                return mlme_sync_request;
            case TP_MLME_POLL_REQUEST:
                return mlme_poll_request;*/
        case TP_CCA_CSMACA:
            return CCA_csmaca;
        case TP_RX_ON_CSMACA:
            return RX_ON_csmaca;
        default:
            ASSERT(0);
            // shutup the compiler.
            return RX_ON_csmaca;
        }
    }

    UINT_8 &taskStep(Ieee802154MacTaskType task)
    {
        switch (task)
        {
        case TP_MCPS_DATA_REQUEST:
            return mcps_data_request_STEP;
            /*case TP_MLME_ASSOCIATE_REQUEST:
                return mlme_associate_request_STEP;
            case TP_MLME_ASSOCIATE_RESPONSE:
                return mlme_associate_response_STEP;
            case TP_MLME_DISASSOCIATE_REQUEST:
                return mlme_disassociate_request_STEP;
            case TP_MLME_ORPHAN_RESPONSE:
                return mlme_orphan_response_STEP;
            case TP_MLME_RESET_REQUEST:
                return mlme_reset_request_STEP;
            case TP_MLME_RX_ENABLE_REQUEST:
                return mlme_rx_enable_request_STEP;
            case TP_MLME_SCAN_REQUEST:
                return mlme_scan_request_STEP;
            case TP_MLME_START_REQUEST:
                return mlme_start_request_STEP;
            case TP_MLME_SYNC_REQUEST:
                return mlme_sync_request_STEP;
            case TP_MLME_POLL_REQUEST:
                return mlme_poll_request_STEP;*/
        case TP_CCA_CSMACA:
            return CCA_csmaca_STEP;
        case TP_RX_ON_CSMACA:
            return RX_ON_csmaca_STEP;
        default:
            ASSERT(0);
            // shutup compiler.
            return RX_ON_csmaca_STEP;
        }
    }

    char *taskFrFunc(Ieee802154MacTaskType task)
    {
        switch (task)
        {
        case TP_MCPS_DATA_REQUEST:
            return mcps_data_request_frFunc;
            /*case TP_MLME_ASSOCIATE_REQUEST:
                return mlme_associate_request_frFunc;
            case TP_MLME_ASSOCIATE_RESPONSE:
                return mlme_associate_response_frFunc;
            case TP_MLME_DISASSOCIATE_REQUEST:
                return mlme_disassociate_request_frFunc;
            case TP_MLME_ORPHAN_RESPONSE:
                return mlme_orphan_response_frFunc;
            case TP_MLME_RESET_REQUEST:
                return mlme_reset_request_frFunc;
            case TP_MLME_RX_ENABLE_REQUEST:
                return mlme_rx_enable_request_frFunc;
            case TP_MLME_SCAN_REQUEST:
                return mlme_scan_request_frFunc;
            case TP_MLME_START_REQUEST:
                return mlme_start_request_frFunc;
            case TP_MLME_SYNC_REQUEST:
                return mlme_sync_request_frFunc;
            case TP_MLME_POLL_REQUEST:
                return mlme_poll_request_frFunc;*/
        default:
            ASSERT(0);
            // shutup compiler.
            //return mlme_poll_request_frFunc;
        }
        return 0;
    }

    //----------------
    bool    mcps_data_request;
    UINT_8  mcps_data_request_STEP;
    char    mcps_data_request_frFunc[81];
    Ieee802154TxOption  mcps_data_request_TxOption;
    Ieee802154Frame*    mcps_data_request_pendPkt;
    /*----------------
    bool    mlme_associate_request;
    UINT_8  mlme_associate_request_STEP;
    char    mlme_associate_request_frFunc[81];
    bool    mlme_associate_request_SecurityEnable;
    UINT_8  mlme_associate_request_CoordAddrMode;
    Packet  *mlme_associate_request_pendPkt;
    //----------------
    bool    mlme_associate_response;
    UINT_8  mlme_associate_response_STEP;
    char    mlme_associate_response_frFunc[81];
    IE3ADDR mlme_associate_response_DeviceAddress;
    Packet  *mlme_associate_response_pendPkt;
    //----------------
    bool    mlme_disassociate_request;
    UINT_8  mlme_disassociate_request_STEP;
    char    mlme_disassociate_request_frFunc[81];
    bool    mlme_disassociate_request_toCoor;
    Packet  *mlme_disassociate_request_pendPkt;
    //----------------
    bool    mlme_orphan_response;
    UINT_8  mlme_orphan_response_STEP;
    char    mlme_orphan_response_frFunc[81];
    IE3ADDR mlme_orphan_response_OrphanAddress;
    //----------------
    bool    mlme_reset_request;
    UINT_8  mlme_reset_request_STEP;
    char    mlme_reset_request_frFunc[81];
    bool    mlme_reset_request_SetDefaultPIB;
    //----------------
    bool    mlme_rx_enable_request;
    UINT_8  mlme_rx_enable_request_STEP;
    char    mlme_rx_enable_request_frFunc[81];
    UINT_32 mlme_rx_enable_request_RxOnTime;
    UINT_32 mlme_rx_enable_request_RxOnDuration;
    double  mlme_rx_enable_request_currentTime;
    //----------------
    bool    mlme_scan_request;
    UINT_8  mlme_scan_request_STEP;
    char    mlme_scan_request_frFunc[81];
    UINT_8  mlme_scan_request_ScanType;
    UINT_8  mlme_scan_request_orig_macBeaconOrder;
    UINT_8  mlme_scan_request_orig_macBeaconOrder2;
    UINT_8  mlme_scan_request_orig_macBeaconOrder3;
    UINT_16 mlme_scan_request_orig_macPANId;
    UINT_32 mlme_scan_request_ScanChannels;
    UINT_8  mlme_scan_request_ScanDuration;
    UINT_8  mlme_scan_request_CurrentChannel;
    UINT_8  mlme_scan_request_ListNum;
    UINT_8  mlme_scan_request_EnergyDetectList[27];
    PAN_ELE mlme_scan_request_PANDescriptorList[27];
    //----------------
    bool    mlme_start_request;
    UINT_8  mlme_start_request_STEP;
    char    mlme_start_request_frFunc[81];
    UINT_8  mlme_start_request_BeaconOrder;
    UINT_8  mlme_start_request_SuperframeOrder;
    bool    mlme_start_request_BatteryLifeExtension;
    bool    mlme_start_request_SecurityEnable;
    bool    mlme_start_request_PANCoordinator;
    UINT_16 mlme_start_request_PANId;
    UINT_8  mlme_start_request_LogicalChannel;
    //----------------
    bool    mlme_sync_request;
    UINT_8  mlme_sync_request_STEP;
    char    mlme_sync_request_frFunc[81];
    UINT_8  mlme_sync_request_numSearchRetry;
    bool    mlme_sync_request_tracking;
    //----------------
    bool    mlme_poll_request;
    UINT_8  mlme_poll_request_STEP;
    char    mlme_poll_request_frFunc[81];
    UINT_8  mlme_poll_request_CoordAddrMode;
    UINT_16 mlme_poll_request_CoordPANId;
    IE3ADDR mlme_poll_request_CoordAddress;
    bool    mlme_poll_request_SecurityEnable;
    bool    mlme_poll_request_autoRequest;
    bool    mlme_poll_request_pending;
    //----------------*/
    bool    CCA_csmaca;
    UINT_8  CCA_csmaca_STEP;
    //----------------
    bool    RX_ON_csmaca;
    UINT_8  RX_ON_csmaca_STEP;
    //----------------
};

class INET_API Ieee802154Mac: public cSimpleModule, public INotifiable
{
  public:
    /**
    * @name Constructor and destructor
    */
    //@{
    Ieee802154Mac();
    ~Ieee802154Mac();
    //@}

    /** @brief called by other MAC modules in order to translate host name into MAC address */
    IE3ADDR getMacAddr() {return aExtendedAddress;}

  protected:
    /** @brief the bit rate at which we transmit */
    double bitrate;

    /**
    * @name Initializtion functions
    */
    //@{
    virtual void    initialize    (int);
    virtual void    initializeQueueModule    ();
    virtual int    numInitStages    () const { return 3; }
    virtual void registerInterface();
    //@}

    virtual void    finish    ();
    virtual void    receiveChangeNotification    (int, const cPolymorphic*);

    // Functions for starting a WPAN with star topology
    virtual void    startPANCoor    ();
    virtual void    startDevice    ();

    /**
    * @name Message handling functions
    */
    //@{
    virtual void    handleMessage    (cMessage*);
    virtual void    handleSelfMsg    (cMessage*);
    virtual void    handleUpperMsg    (cMessage*);
    virtual void    handleLowerMsg    (cMessage*);
    virtual void    handleMacPhyPrimitive    (int, cMessage*);
    virtual void    handleBeacon    (Ieee802154Frame*);
    virtual void    handleCommand    (Ieee802154Frame*);
    virtual void    handleData    (Ieee802154Frame*);
    virtual void    handleAck    (Ieee802154Frame*);
    /** @brief MAC frame filter, return true if frame is filtered out */
    virtual bool    frameFilter    (Ieee802154Frame*);
    virtual void    sendDown    (Ieee802154Frame*);
    virtual void    constructACK    (Ieee802154Frame*);
    /** @brief request a msg from IFQueue, if it exists */
    virtual void    reqtMsgFromIFq    ();
    //@}

    /**
    * @name Command handling functions
    *  for convenience, some MAC commands are implemeted in a tricky way in the model
    *  these functions are only called directly by ohter MAC modules, instead of via messages exchange like in real world
    */
    //@{
    /** This function is a part of simplified association process in the model.
     *  After receiving a beacon from the coordinator for the first time and associating with it,
     *  each device should call this function of its coordinator to register its MAC address and capability info at the coordinator and get the short MAC address from the return value */
    virtual UINT_16    associate_request_cmd    (IE3ADDR extAddr, const DevCapability& capaInfo);

    /** This function is a part of simplified GTS request process in the model.
     *  After association, each device should call this function of its PAN Coordinator
     *  to apply for GTS and get its GTS starting number from the return value */
    virtual UINT_8    gts_request_cmd    (UINT_16 macShortAddr, UINT_8 length, bool isReceive);

    /**
    * @name MAC-PHY primitives related functions
    */
    //@{
    virtual void    PLME_SET_TRX_STATE_request    (PHYenum state);
    virtual void    PLME_SET_request    (PHYPIBenum attribute);
    virtual void    PLME_CCA_request    ();
    virtual void    PLME_bitrate_request();
    virtual void    handle_PD_DATA_confirm    (PHYenum status);
    virtual void    handle_PLME_CCA_confirm    (PHYenum status);
    virtual void    handle_PLME_SET_TRX_STATE_confirm    (PHYenum status);
    //@}

    /**
    * @name SSCS-MAC primitives related functions
    */
    //@{
    virtual void    MCPS_DATA_request    (UINT_8 SrcAddrMode, UINT_16 SrcPANId, IE3ADDR SrcAddr,
                                          UINT_8 DstAddrMode, UINT_16 DstPANId, IE3ADDR DstAddr,
                                          cPacket* msdu,     Ieee802154TxOption TxOption);
    virtual void    MCPS_DATA_confirm    (MACenum status);
    virtual void    MCPS_DATA_indication    (Ieee802154Frame*);
    //@}

    /**
    * @name CSMA/CA related functions
    */
    //@{
    virtual void    csmacaEntry    (char pktType);// 'c': txBcnCmd; 'u': txBcnCmdUpper; 'd': txData
    virtual void    csmacaResume    ();
    virtual void    csmacaStart    (bool firsttime, Ieee802154Frame* frame = 0, bool ackReq = 0);
    virtual void    csmacaCancel    ();
    virtual void    csmacaCallBack    (PHYenum status); // CSMA-CA success or failure
    virtual void    csmacaReset    (bool bcnEnabled);
    virtual bool    csmacaCanProceed    (simtime_t wtime, bool afterCCA = false);
    virtual void    csmaca_handle_RX_ON_confirm    (PHYenum status);    // To be called by handle_PLME_SET_TRX_STATE_confirm
    virtual void    csmacaTrxBeacon    (char trx);  // To be called each time that a beacon received or transmitted
    virtual simtime_t    csmacaAdjustTime    (simtime_t wtime);
    virtual simtime_t    csmacaLocateBoundary    (bool toParent, simtime_t wtime);
    virtual simtime_t    getFinalCAP    (char trxType);
    //@}

    /**
    * @name State control and task management functions
    */
    //@{
    virtual void    dispatch    (PHYenum pStatus, const char *frFunc,
                                 PHYenum req_state = phy_SUCCESS,
                                 MACenum mStatus = mac_SUCCESS);
    virtual void    taskSuccess    (char type, bool csmacaRes = true);
    virtual void    taskFailed    (char type, MACenum status, bool csmacaRes = true);
    virtual void    checkTaskOverflow    (Ieee802154MacTaskType task);
    virtual void    FSM_MCPS_DATA_request    (PHYenum pStatus = phy_SUCCESS, MACenum mStatus = mac_SUCCESS);
    virtual void    resetTRX    ();
    //@}

    /**
    * @name Timer handling functions
    */
    //@{
    virtual void    handleBackoffTimer    ();
    virtual void    handleDeferCCATimer    ();
    virtual void    handleBcnRxTimer    ();
    virtual void    handleBcnTxTimer    ();
    virtual void    handleAckTimeoutTimer    ();
    virtual void    handleTxAckBoundTimer    (); // ACK is sent here
    virtual void    handleTxCmdDataBoundTimer    ();    // Cmd or data is sent here
    virtual void    handleIfsTimer    ();
    virtual void    handleSDTimer    ();    // shared by txSDTimer and rxSDTimer
    virtual void    handleFinalCapTimer    ();
    virtual void    handleGtsTimer    ();
    //@}

    /**
    * @name GTS related functions
    */
    //@{
    virtual bool    gtsCanProceed    ();
    virtual void    gtsScheduler    ();
    //@}

    /**
    * @name Timer starting functions
    */
    //@{
    virtual void    startBackoffTimer    (simtime_t);
    virtual void    startDeferCCATimer    (simtime_t);
    virtual void    startBcnRxTimer    ();
    virtual void    startBcnTxTimer    (bool txFirstBcn = false, simtime_t startTime = 0.0);
    virtual void    startAckTimeoutTimer ();
    virtual void    startTxAckBoundTimer    (simtime_t);
    virtual void    startTxCmdDataBoundTimer    (simtime_t);
    virtual void    startIfsTimer    (bool);
    virtual void    startTxSDTimer    ();
    virtual void    startRxSDTimer    ();
    virtual void    startGtsTimer    (simtime_t);
    virtual void    startFinalCapTimer    (simtime_t);
    //@}

    /**
    * @name Utility functions
    */
    //@{
    /** @brief check if the packet is sent to its parent or not */
    virtual bool    toParent    (Ieee802154Frame*);

    /** @brief calculate byte length of frame of certain type */
    virtual int    calFrmByteLength    (Ieee802154Frame*);

    /** @brief calculate byte length of frame header  */
    virtual int    calMHRByteLength    (UINT_8);

    /** @brief calculate duration of the frame transmitted over wireless channel  */
    virtual simtime_t    calDuration    (Ieee802154Frame*);

    /** @brief return current bit or symbol rate  at PHY*/
    virtual double    getRate    (char);
    //@}

    // Use to distinguish the radio module that send the event
    int radioModule;

    int getRadioModuleId() {return radioModule;}


// member variables
  public:

  protected:
    /**
    * @name User-adjustable MAC parameters
    */
    //@{
    /** @brief debug switch */
    bool m_debug;

    /** @brief enable or disable ACK for data transmission in CAP*/
    bool ack4Data;

    /** @brief enable or disable ACK for data transmission in CFP */
    bool ack4Gts;

    /** @brief enable or disable security for data frames (TBD) */
    bool secuData;

    /** @brief enable or disable security for beacon frames (TBD) */
    bool secuBeacon;

    /** @brief I'm the PAN coordinator or not */
    bool isPANCoor;

    /** @brief data transfer mode:  1: direct; 2: indirect; 3: GTS */
    int  dataTransMode;

    /** @brief when PAN starts */
    simtime_t panStartTime;

    /** @brief host name of the PAN coordinator in string */
    const char* panCoorName;

    /** @brief transmit or receive GTS, only used by devices */
    bool isRecvGTS;

    /** @brief payload of data frames transmitted in GTS, copied from traffic module */
    int gtsPayload;
    //@}

    /**
    * @name Static variables
    */
    //@{
    /** @brief global counter for generating unique MAC extended address */
    static IE3ADDR addrCount;

    /** @brief default MAC PIB attributes */
    static MAC_PIB MPIB;
    //@}

    /**
    * @name Module gate ID
    */
    //@{
    int mUppergateIn;
    int mUppergateOut;
    int mLowergateIn;
    int mLowergateOut;
    //@}

    /** @brief  pointer to the NotificationBoard module */
    NotificationBoard* mpNb;

    /** @brief pointer to the passive queue module */
    IPassiveQueue* queueModule;

    // for task processing
    taskPending taskP;

    /** @brief current requested radio state sent to PHY via PLME_SET_TRX_STATE_request */
    PHYenum trx_state_req;

    /**
    * @name Variables for PHY parameters
    */
    //@{
    /** @brief PHY PIB attributes copied from PHY via notificationboard */
    PHY_PIB ppib, tmp_ppib;

    /** @brief current bit rate at PHY */
    double phy_bitrate;

    /** @brief current symbol rate at PHY */
    double phy_symbolrate;
    //@}

    /**
    * @name Variables for MAC parameters
    */
    //@{
    /** @brief MAC extended address, in simulation use only 16 bit instead of 64 bit */
    IE3ADDR aExtendedAddress;
    MACAddress macaddress;//

    /** @brief MAC PIB attributes */
    MAC_PIB mpib;

    /** @brief device capability (TBD) */
    DevCapability capability;
    //@}

    /**
    * @name Beacon related variables
    */
    //@{
    /** @brief beacon order of incoming superframe */
    UINT_8 rxBO;

    /** @brief superframe order of incoming superframe */
    UINT_8 rxSO;

    /** @brief duration (in symbol) of a outgoing superframe slot (aBaseSlotDuration * 2^mpib.macSuperframeOrder) */
    UINT_32 txSfSlotDuration;

    /** @brief duration (in symbol) of a incoming superframe slot (aBaseSlotDuration * 2^rxSO) */
    UINT_32 rxSfSlotDuration;

    /** @brief duration (in backoff periods) of latest outgoing beacon */
    UINT_8 txBcnDuration;

    /** @brief duration (in backoff periods) of latest incoming beacon */
    UINT_8 rxBcnDuration;

    /** @brief length (in s) of a unit of backoff period, aUnitBackoffPeriod/phy_symbolrate */
    simtime_t bPeriod;

    /** @brief the time that the last beacon was received
     updated right after the beacon is received at MAC */
    simtime_t bcnRxTime;

    /** @brief the scheduled time that the latest beacon should have arrived
     sometimes bcnRxTime lags due to beacon loss (assumed impossible in the model) or in the middle of receiving a beacon,
     some calculations use this value to avoid incorrectness */
    simtime_t schedBcnRxTime;

    /** @brief outging superframe specification used by coordinators */
    SuperframeSpec txSfSpec;

    /** @brief incoming superframe specification used by devices */
    SuperframeSpec rxSfSpec;

    /** @brief outgoing PAN descriptor transmitted used by coordinators (TBD) */
    PAN_ELE txPanDescriptor;

    /** @brief incoming PAN descriptor transmitted used by devices (TBD) */
    PAN_ELE rxPanDescriptor;

    /** @brief flag for using beacon or not */
    bool beaconEnabled;

    /** @brief indicating a beacon frame waiting for transmission, suppress all other transmissions */
    bool beaconWaitingTx;

    /** @brief  indicating whether associated with a coordinator or not */
    bool notAssociated;

    /** @brief num of incoming beacons lost in a row */
    UINT_8 bcnLossCounter;
    //@}

    /**
    * @name CSMA/CA related variables
    */
    //@{
    UINT_8 NB;
    UINT_8 CW;
    UINT_8 BE;
    UINT_8 backoffStatus;   // 0: no backoff; 1: backoff successful; 2: backoff failed; 99: is backing off
    int bPeriodsLeft;   // backoff periods left
    bool csmacaAckReq;
    bool csmacaWaitNextBeacon;
    //@}

    /**
    * @name GTS related variables only for PAN coordinator
    */
    //@{
    /** @brief number of GTS descriptors being maintained */
    UINT_8 gtsCount;

    /** @brief list of GTS descriptors for all existing GTS being maintained */
    GTSDescriptor gtsList[7];

    /** @brief store new value of final superframe slot in the CAP after updating GTS settings
     and put into effect when next beacon is transmitted  */
    UINT_8 tmp_finalCap;

    /** @brief index of current running GTS, 99 means not in GTS */
    UINT_8 indexCurrGts;
    //@}


    /**
    * @name GTS related variables only for devices
    */
    //@{
    /** @brief number of superframe slots for the GTS, calculated in handleBeacon()  */
    UINT_8 gtsLength;

    /** @brief GTS starting slot, 0 means no GTS for me, allocated by the PAN coordinator when applying for GTS */
    UINT_8 gtsStartSlot;

    /** @brief duration of one complete data transaction in GTS, calculated in handleBeacon() */
    simtime_t gtsTransDuration;
    //@}

    /**
    * @name Indirect data transfer related variables
    */
    //@{
    //PendingAddrFields txPaFields; // pengding address fields transmitted as a coordinator
    //PendingAddrFields rxPaFields; // pengding address fields received as a device
    //@}

    /**
    * @name Frame buffers
    */
    //@{
    /** @brief buffer for frames currently being transmitted */
    Ieee802154Frame* txPkt;

    /** @brief buffer for beacon frames to be transmitted without CSMA/CA */
    Ieee802154Frame* txBeacon;

    /** @brief buffer for beacon or cmd frames coming from the upper layer,
     to be transmitted with CSMA/CA */
    Ieee802154Frame* txBcnCmdUpper;

    /** @brief buffer for beacon or cmd frames responding to receiving a packet,
     to be transmitted with CSMA/CA */
    Ieee802154Frame* txBcnCmd;

    /** @brief buffer for data frames to be transmitted */
    Ieee802154Frame* txData;

    /** @brief buffer for data frames to be transmitted in GTS */
    Ieee802154Frame* txGTS;

    /** @brief buffer for ack frames to be transmitted (no wait) */
    Ieee802154Frame* txAck;

    /** @brief buffer for frames currently being CSMA/CA performed,
     one of txBcnCmdUpper, txBcnCmd or txData */
    Ieee802154Frame* txCsmaca;

    /** @brief temp buffer for CSMA-CA, set when csmacaStart is called for first time,
     cleared when csmacaCallBack or csmacaCancel is called */
    Ieee802154Frame* tmpCsmaca;

    /** @brief buffer for received beacon frames,
     only used by mlme_scan_request and mlme_rx_enable_request (TBD) */
    Ieee802154Frame* rxBeacon;

    /** @brief buffer for received data frames */
    Ieee802154Frame* rxData;

    /** @brief buffer for received cmd frames */
    Ieee802154Frame* rxCmd;
    //@}

    /**
    * @name Timer messages
    */
    //@{
    /** @brief backoff timer for CSMA-CA */
    cMessage* backoffTimer;

    /** @brief timer for locating backoff boundary before sending a CCA request */
    cMessage* deferCCATimer;

    /** @brief timer for tracking beacons */
    cMessage* bcnRxTimer;

    /** @brief timer for transmitting beacon periodically */
    cMessage* bcnTxTimer;

    /** @brief timer for timer for ACK timeout */
    cMessage* ackTimeoutTimer;

    /** @brief timer for locating backoff boundary before txing ACK if beacon-enabled */
    cMessage* txAckBoundTimer;

    /** @brief timer for locating backoff boundary before txing Cmd or data if beacon-enabled */
    cMessage* txCmdDataBoundTimer;

    /** @brief timer for delay of IFS after receiving a data or cmd pkt */
    cMessage* ifsTimer;

    /** @brief timer for indicating being in the active period of outgoing (txing) superframe */
    cMessage* txSDTimer;

    /** @brief timer for indicating being in the active period of incoming (rxed) superframe */
    cMessage* rxSDTimer;

    /** @brief timer for indicating the end of CAP and the starting of CFP
     used only by devices to put radio into sleep at the end of CAP if my GTS is not the first one in the CFP */
    cMessage* finalCAPTimer;

    /** @brief timer for scheduling of GTS, shared by both PAN coordinator and devices */
    cMessage* gtsTimer;
    //@}

    /**
    * @name Variables for timers
    */
    //@{
    simtime_t lastTime_bcnRxTimer;
    bool txNow_bcnTxTimer;

    /** @brief true while in active period of the outgoing superframe  */
    bool inTxSD_txSDTimer;

    /** @brief true while in active period of the incoming superframe  */
    bool inRxSD_rxSDTimer;

    /** for PAN coordinator: index of GTS descriptor in the GTS list
     * that GTS timer is currently scheduling for its starting;
     * for devices: 99 indicating currently being in my GTS
    */
    UINT_8 index_gtsTimer;
    //@}

    /**
    * @name Data transmission related variables
    */
    //@{
    /** @brief true while a packet being transmitted at PHY */
    bool inTransmission;

    /** @brief true while a sent beacon or cmd frame in txBcnCmd is waiting for ACK */
    bool waitBcnCmdAck;

    /** @brief true while a sent beacon or cmd frame in txBcnCmdUpper is waiting for ACK */
    bool waitBcnCmdUpperAck;

    /** @brief true while a sent frame in txData is waiting for ACK */
    bool waitDataAck;

    /** @brief true while a sent frame in txGTS is waiting for ACK */
    bool waitGTSAck;

    /** @brief number of retries for txing a beacon or cmd frame in txBcnCmd */
    UINT_8 numBcnCmdRetry;

    /** @brief number of retries for txing a beacon or cmd frame in txBcnCmdUpper */
    UINT_8 numBcnCmdUpperRetry;

    /** @brief number of retries for txing a data frame in txData */
    UINT_8 numDataRetry;

    /** @brief number of retries for txing a data frame in txGTS */
    UINT_8 numGTSRetry;
    //@}

    /**
    * @name Link objects for beacon and data pkts duplication detection
    */
    //@{
    HLISTLINK* hlistBLink1;
    HLISTLINK* hlistBLink2;
    HLISTLINK* hlistDLink1;
    HLISTLINK* hlistDLink2;
    //@}

    /**
    * @name Container used by coordinator to store info of associated devices
    */
    typedef std::map<UINT_16, DevCapability> DeviceList;
    DeviceList deviceList;

    /**
    * @name Statistical variables
    */
    //@{
    /** @brief number of data pkts received from upper layer, counted in <handleUpperMsg()> */
    double numUpperPkt;

    /** @brief number of data pkts from upper layer dropped by MAC due to busy MAC or invalid size (e.g. oversize), counted in <handleUpperMsg()> */
    double numUpperPktLost;

    /** @brief number of collisions detected at PHY */
    double numCollision;

    /** @brief number of incoming beacons lost, counted in <handleBcnRxTimer()> */
    double numLostBcn;

    /** @brief number of transmitted beacons, counted in <taskSuccess('b')> */
    double numTxBcnPkt;

    /** @brief number of successfully transmitted data frames, counted in <taskSuccess('d')> */
    double numTxDataSucc;

    /** @brief number of data frames that MAC failed to transmit, counted in <taskFailed()> */
    double numTxDataFail;

    /** @brief number of successfully transmitted data frames in GTS, counted in <taskSuccess('g')> */
    double numTxGTSSucc;

    /** @brief number of data frames that MAC failed to transmit in GTS, counted in <taskFailed()> */
    double numTxGTSFail;

    /** @brief number of transmitted ack frames, counted in <taskSuccess()> */
    double numTxAckPkt;

    /** @brief number of received beacons, counted in <handleBeacon()> */
    double numRxBcnPkt;

    /** @brief number of received data frames, counted in <MCPS_DATA_indication()> */
    double numRxDataPkt;

    /** @brief number of received data frames in GTS, counted in <MCPS_DATA_indication()> */
    double numRxGTSPkt;

    /** @brief count only ACKs received before timeout, for both Cmd and Data  in <handleAck()> */
    double numRxAckPkt;
    //@}

    // tmp variables for debug
    double numTxAckInactive;
};
#endif
