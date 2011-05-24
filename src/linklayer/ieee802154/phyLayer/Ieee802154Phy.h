//******************************
//LQI measurement not implemented
//

#ifndef IEEE_802154_PHY_H
#define IEEE_802154_PHY_H

#include "ChannelAccess.h"
#include "ChannelAccessExtended.h"
#include "RadioState.h"
#include "Ieee802154Const.h"
#include "Ieee802154Def.h"
#include "Ieee802154Enum.h"
#include "Ieee802154MacPhyPrimitives_m.h"
#include "AirFrame_m.h"
#include "IRadioModel.h"
#include "IReceptionModel.h"
#include "FWMath.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"


class INET_API Ieee802154Phy : public ChannelAccessExtended
{
  public:
    Ieee802154Phy           ();
    virtual ~Ieee802154Phy      ();

  protected:
    virtual void        initialize      (int);
    virtual int     numInitStages       () const { return 3; }
    virtual void        finish          ();

    // message handle functions
    void                handleMessage       (cMessage*);
    virtual void        handleUpperMsg      (AirFrame*);
    virtual void        handleSelfMsg       (cMessage*);
    virtual void        handlePrimitive     (int, cMessage*);
    virtual void        handleLowerMsgStart (AirFrame*);
    virtual void        handleLowerMsgEnd   (AirFrame*);
    virtual AirFrame*   encapsulatePacket   (cMessage*);
    void                bufferMsg       (AirFrame*);
    AirFrame*       unbufferMsg     (cMessage*);
    void                sendUp          (cMessage*);
    void                sendDown        (AirFrame*);

    virtual IReceptionModel *createReceptionModel() {return (IReceptionModel *)createOne(par("attenuationModel").stringValue());}
    virtual IRadioModel *createRadioModel() {return (IRadioModel *)createOne(par("radioModel").stringValue());}

    // primitives processing functions
    void                PD_DATA_confirm     (PHYenum status);
    void                PD_DATA_confirm     (PHYenum status,short);
    void                PLME_CCA_confirm    (PHYenum status);
    void                PLME_ED_confirm     (PHYenum status, UINT_8 energyLevel);
    void                handle_PLME_SET_TRX_STATE_request   (PHYenum setState);
    void                PLME_SET_TRX_STATE_confirm      (PHYenum status);
    void                handle_PLME_SET_request         (Ieee802154MacPhyPrimitives *primitive);
    void                PLME_SET_confirm            (PHYenum status, PHYPIBenum attribute);
    void                PLME_bitRate(double bitRate);

    void                setRadioState           (RadioState::State newState);
    int                 getChannelNumber        () const {return rs.getChannelNumber();}
    void                addNewSnr           ();
    void                changeChannel       (int newChannel);
    bool                channelSupported        (int channel);
    UINT_8              calculateEnergyLevel    ();
    double              getRate             (char dataOrSymbol);

    bool processAirFrame (AirFrame*);

  protected:
    bool                m_debug;        // debug switch
    IRadioModel*        radioModel;
    IReceptionModel*    receptionModel;

    int             uppergateOut;
    int             uppergateIn;

    double          transmitterPower;       // in mW
    double          noiseLevel;
    double          carrierFrequency;
    double          sensitivity;        // in mW
    double          thermalNoise;

    struct SnrStruct
    {
        AirFrame*   ptr;    ///< pointer to the message this information belongs to
        double      rcvdPower; ///< received power of the message
        SnrList     sList;    ///< stores SNR over time
    };
    SnrStruct snrInfo;  // stores the snrList and the the recvdPower for the
    // message currently being received, together with a pointer to the message.

    typedef         std::map<AirFrame*,double>  RecvBuff;
    RecvBuff            recvBuff; // A buffer to store a pointer to a message and the related receive power.

    AirFrame*       txPktCopy; // duplicated outgoing pkt, accessing encapsulated msg only when transmitter is forcibly turned off
    // set a error flag into the encapsulated msg to inform all nodes rxing this pkt that the transmition is terminated and the pkt is corrupted
    // use the new feature "Reference counting" of encapsulated messages
    //AirFrame *rxPkt;
    double          rxPower[27];    // accumulated received power in each channel, for ED measurement purpose
    double          rxPeakPower;    // peak power in current channle, for ED measurement purpose
    int             numCurrRx;

    RadioState      rs;             // four states model: idle, rxing, txing, sleep
    PHYenum         phyRadioState;  // three states model, according to spec: RX_ON, TX_ON, TRX_OFF
    PHYenum         newState;
    PHYenum         newState_turnaround;
    bool                isCCAStartIdle;     // indicating wheter channel is idle at the starting of CCA

    // timer
    cMessage*       CCA_timer; // timer for CCA, delay 8 symbols
    cMessage*       ED_timer;   // timer for ED measurement
    cMessage*       TRX_timer; // timer for Tx2Rx turnaround
    cMessage*       TxOver_timer;  // timer for tx over
    virtual void registerBattery();
    virtual void updateDisplayString();
    virtual double calcDistFreeSpace();
    bool drawCoverage;
    cMessage *updateString;
    simtime_t updateStringInterval;
};

#endif
