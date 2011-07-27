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
    uint8_t CoordAddrMode;
    uint16_t CoordPANId;

    uint16_t CoordAddress_16_or_64; // shared by both 16 bit short address or 64 bit extended address

    uint8_t LogicalChannel;
    //uint16_t           SuperframeSpec;     // ignored, store in txSfSpec or rxSfSpec instead
    bool                GTSPermit;
    uint8_t LinkQuality;
    //simtime_t     TimeStamp;              // ignored, use bcnRxTime instead
    bool                SecurityUse;
    uint8_t ACLEntry;
    bool                SecurityFailure;
    //add one field for cluster tree
    //uint16_t   clusTreeDepth;
};

struct PHY_PIB
{
    int             phyCurrentChannel;
    //uint32_t phyChannelsSupported;
    //double            phyTransmitPower;
    //uint8_t phyCCAMode;
};

struct MAC_PIB
{
    //attributes from Table 71
    uint8_t macAckWaitDuration;
    bool                macAssociationPermit;
    bool                macAutoRequest;
    bool                macBattLifeExt;
    uint8_t macBattLifeExtPeriods;
    /*
    uint8_t  macBeaconPayload[aMaxPHYPacketSize-(6+9+2+1)+1];    //beacon length in octets (w/o payload):
                                    //  max: 6(phy) + 15(mac) + 23 (GTSs) + 57 (pending addresses)
                                    //  min: 6(phy) + 9(mac) + 2 (GTSs) + 1 (pending addresses)
    */
    uint8_t macBeaconPayload[aMaxBeaconPayloadLength + 1];
    uint8_t macBeaconPayloadLength;
    uint8_t macBeaconOrder;
    double      macBeaconTxTime;            // we use actual time in double instead of integer in spec
    uint8_t macBSN; // sequence number for beacon pkt
    IE3ADDR macCoordExtendedAddress;
    uint16_t macCoordShortAddress;
    uint8_t macDSN; // sequence number for data or cmd pkt
    bool                macGTSPermit;
    uint8_t macMaxCSMABackoffs;
    uint8_t macMinBE;
    uint16_t macPANId;
    bool                macPromiscuousMode;
    bool                macRxOnWhenIdle;
    uint16_t macShortAddress;
    uint8_t macSuperframeOrder;
    uint16_t macTransactionPersistenceTime;
    //attributes from Table 72 (security attributes)

    /*
    MAC_ACL*        macACLEntryDescriptorSet;
    uint8_t macACLEntryDescriptorSetSize;
    bool                macDefaultSecurity;
    uint8_t macACLDefaultSecurityMaterialLength;
    uint8_t* macDefaultSecurityMaterial;
    uint8_t macDefaultSecuritySuite;
    uint8_t macSecurityMode;
    */
};

//Elements of ACL entry descriptor (Table 73)
struct MAC_ACL
{
    IE3ADDR         ACLExtendedAddress;
    uint16_t ACLShortAddress;
    uint16_t ACLPANId;
    uint8_t ACLSecurityMaterialLength;
    uint8_t* ACLSecurityMaterial;
    uint8_t ACLSecuritySuite;
};

// Frame Control field in MHR (Figure 35)
struct FrameCtrl
{
    Ieee802154FrameType         frmType;
    bool                secu;
    bool                frmPending;
    bool                ackReq;
    bool                intraPan;
    uint8_t dstAddrMode;
    uint8_t srcAddrMode;
};

// Superframe specification (SS) (16 bits) - Fig 40
struct SuperframeSpec
{
    uint8_t BO; // beacon order
    uint32_t BI; // becaon interval
    uint8_t SO; // superframe order
    uint32_t SD; // superframe duration
    uint8_t finalCap; // final superframe slot utilized by the CAP
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
    uint16_t devShortAddr;       // device short address
    uint8_t startSlot;           // starting slot
    uint8_t length;          // length in slots
    bool isRecvGTS;     // transmit or receive in GTS, not defined here is spec, but we put it here just for convenience
    bool isTxPending;       // there is a data pending for txing
};

struct PendingAddrFields
{
    uint8_t numShortAddr;   //num of short addresses pending
    uint8_t numExtendedAddr;    //num of extended addresses pending
    IE3ADDR             addrList[7];    //pending address list (shared by short/extended addresses)
};



#endif

