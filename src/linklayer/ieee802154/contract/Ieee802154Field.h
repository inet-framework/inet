#ifndef IEEE_802154_FIELD_H
#define IEEE_802154_FIELD_H

#include "Ieee802154Def.h"
#include "Ieee802154Enum.h"

// Addressing mode
#define             defFrmCtrl_AddrModeNone     0x00
#define             defFrmCtrl_AddrMode16       0x02
#define             defFrmCtrl_AddrMode64       0x03

// Transmission option
#define             defTxOp_Acked   0x01
#define             defTxOp_GTS 0x02
#define             defTxOp_Indirect    0x04
#define             defTxOp_SecEnabled  0x08

//Elements of PAN descriptor (Table 41)
struct PAN_ELE
{
    UINT_8          CoordAddrMode;
    UINT_16         CoordPANId;

    UINT_16         CoordAddress_16_or_64;      // shared by both 16 bit short address or 64 bit extended address

    UINT_8          LogicalChannel;
    //UINT_16           SuperframeSpec;     // ignored, store in txSfSpec or rxSfSpec instead
    bool                GTSPermit;
    UINT_8          LinkQuality;
    //simtime_t     TimeStamp;              // ignored, use bcnRxTime instead
    bool                SecurityUse;
    UINT_8          ACLEntry;
    bool                SecurityFailure;
    //add one field for cluster tree
    //UINT_16   clusTreeDepth;
};

struct PHY_PIB
{
    int             phyCurrentChannel;
    //UINT_32       phyChannelsSupported;
    //double            phyTransmitPower;
    //UINT_8            phyCCAMode;
};

struct MAC_PIB
{
    //attributes from Table 71
    UINT_8          macAckWaitDuration;
    bool                macAssociationPermit;
    bool                macAutoRequest;
    bool                macBattLifeExt;
    UINT_8          macBattLifeExtPeriods;
    /*
    UINT_8  macBeaconPayload[aMaxPHYPacketSize-(6+9+2+1)+1];    //beacon length in octets (w/o payload):
                                    //  max: 6(phy) + 15(mac) + 23 (GTSs) + 57 (pending addresses)
                                    //  min: 6(phy) + 9(mac) + 2 (GTSs) + 1 (pending addresses)
    */
    UINT_8          macBeaconPayload[aMaxBeaconPayloadLength+1];
    UINT_8          macBeaconPayloadLength;
    UINT_8          macBeaconOrder;
    double      macBeaconTxTime;            // we use actual time in double instead of integer in spec
    UINT_8          macBSN;                             // sequence number for beacon pkt
    IE3ADDR         macCoordExtendedAddress;
    UINT_16         macCoordShortAddress;
    UINT_8          macDSN;                             // sequence number for data or cmd pkt
    bool                macGTSPermit;
    UINT_8          macMaxCSMABackoffs;
    UINT_8          macMinBE;
    UINT_16         macPANId;
    bool                macPromiscuousMode;
    bool                macRxOnWhenIdle;
    UINT_16         macShortAddress;
    UINT_8          macSuperframeOrder;
    UINT_16         macTransactionPersistenceTime;
    //attributes from Table 72 (security attributes)

    /*
    MAC_ACL*        macACLEntryDescriptorSet;
    UINT_8          macACLEntryDescriptorSetSize;
    bool                macDefaultSecurity;
    UINT_8          macACLDefaultSecurityMaterialLength;
    UINT_8*         macDefaultSecurityMaterial;
    UINT_8          macDefaultSecuritySuite;
    UINT_8          macSecurityMode;
    */
};

//Elements of ACL entry descriptor (Table 73)
struct MAC_ACL
{
    IE3ADDR         ACLExtendedAddress;
    UINT_16         ACLShortAddress;
    UINT_16         ACLPANId;
    UINT_8          ACLSecurityMaterialLength;
    UINT_8*         ACLSecurityMaterial;
    UINT_8          ACLSecuritySuite;
};

// Frame Control field in MHR (Figure 35)
struct FrameCtrl
{
    Ieee802154FrameType         frmType;
    bool                secu;
    bool                frmPending;
    bool                ackReq;
    bool                intraPan;
    UINT_8          dstAddrMode;
    UINT_8          srcAddrMode;
};

// Superframe specification (SS) (16 bits) - Fig 40
struct SuperframeSpec
{
    UINT_8          BO;         // beacon order
    UINT_32             BI;         // becaon interval
    UINT_8          SO;         // superframe order
    UINT_32             SD;         // superframe duration
    UINT_8          finalCap;       // final superframe slot utilized by the CAP
    bool                battLifeExt;    // battery life extention
    bool                panCoor;        // PAN coordinator
    bool                assoPmt;        // association permit
};

// Device capability information field - Fig 49
struct DevCapability
{
    bool alterPANCoor;
    bool FFD;
    bool recvOnWhenIdle;
    bool secuCapable;
    bool alloShortAddr;
    const char* hostName;       // only for convenience
};

struct GTSDescriptor
{
    UINT_16 devShortAddr;       // device short address
    UINT_8 startSlot;           // starting slot
    UINT_8 length;          // length in slots
    bool isRecvGTS;     // transmit or receive in GTS, not defined here is spec, but we put it here just for convenience
    bool isTxPending;       // there is a data pending for txing
};

struct PendingAddrFields
{
    UINT_8          numShortAddr;   //num of short addresses pending
    UINT_8          numExtendedAddr;    //num of extended addresses pending
    IE3ADDR             addrList[7];    //pending address list (shared by short/extended addresses)
};



#endif

