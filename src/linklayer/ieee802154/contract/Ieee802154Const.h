#ifndef IEEE_802154_CONST_H
#define IEEE_802154_CONST_H

#include "Ieee802154Def.h"

//---PHY layer constants (Table 18)---
const UINT_8 aMaxPHYPacketSize  = 127;      //max PSDU size (in bytes) the PHY shall be able to receive
const UINT_8 aTurnaroundTime    = 12;       //Rx-to-Tx or Tx-to-Rx max turnaround time (in symbol period)

//---Frequency bands and data rates (Table 1)---
const UINT_8 BR_868M    = 20;       //20 kb/s   -- ch 0
const UINT_8 BR_915M    = 40;       //40 kb/s   -- ch 1,2,3,...,10
const UINT_8 BR_2_4G    = 250;      //250 kb/s  -- ch 11,12,13,...,26
const UINT_8 SR_868M    = 20;       //20 ks/s
const UINT_8 SR_915M    = 40;       //40 ks/s
const double SR_2_4G    = 62.5;     //62.5 ks/s

const double max_pDelay = 100.0/200000000.0;    //maximum propagation delay
// PHY header
#define def_phyHeaderLength   6     // in byte

//---PHY_PIB default values---
//All the default values are not given in the draft.
//They are chosen for sake of a value
const UINT_8 def_phyCurrentChannel = 11;
const UINT_32 def_phyChannelsSupported = 0x07ffffff;
//const double def_phyTransmitPower = -3;       // in dBm
const UINT_8 def_phyCCAMode = 3;

// MAC Commands with fixed size
#define SIZE_OF_802154_ASSOCIATION_RESPONSE         29  // Fig 50: MHR (23) + Payload (4) + FCS (2)
#define SIZE_OF_802154_DISASSOCIATION_NOTIFICATION      21  // Fig 51: MHR (17) + Payload (2) + FCS (2)
#define SIZE_OF_802154_PANID_CONFLICT_NOTIFICATION      26  // Fig 53: MHR (23) + Payload (1) + FCS (2)
#define SIZE_OF_802154_ORPHAN_NOTIFICATION          20  // Fig 54: MHR (17) + Payload (1) + FCS (2)
#define SIZE_OF_802154_BEACON_REQUEST               10  // Fig 55: MHR (7) + Payload (1) + FCS (2)
#define SIZE_OF_802154_GTS_REQUEST              11  // Fig 57: MHR (7) + Payload (2) + FCS (2)
#define SIZE_OF_802154_ACK                  5       // Fig 46: MHR (3) + FCS (2)

//---MAC sublayer constants (Table 70)---
const UINT_8 aNumSuperframeSlots            = 16;       //# of slots contained in a superframe
const UINT_8 aBaseSlotDuration              = 60;       //# of symbols comprising a superframe slot of order 0
const UINT_16 aBaseSuperframeDuration
= aBaseSlotDuration * aNumSuperframeSlots;      //# of symbols comprising a superframe of order 0
//aExtendedAddress                  = ;     //64-bit (IEEE) address assigned to the device (device specific)
const UINT_8 aMaxBE                 = 5;        //max value of the backoff exponent in the CSMA-CA algorithm
const UINT_8 aMaxBeaconOverhead             = 75;       //max # of octets added by the MAC sublayer to the payload of its beacon frame
const UINT_8 aMaxBeaconPayloadLength
= aMaxPHYPacketSize - aMaxBeaconOverhead;       //max size, in octets, of a beacon payload
const UINT_8 aGTSDescPersistenceTime            = 4;        //# of superframes that a GTS descriptor exists in the beacon frame of a PAN coordinator
const UINT_8 aMaxFrameOverhead              = 25;       //max # of octets added by the MAC sublayer to its payload w/o security.
const UINT_16 aMaxFrameResponseTime         = 1220;     //max # of symbols (or CAP symbols) to wait for a response frame
const UINT_8 aMaxFrameRetries               = 3;        //max # of retries allowed after a transmission failures
const UINT_8 aMaxLostBeacons                = 4;        //max # of consecutive beacons the MAC sublayer can miss w/o declaring a loss of synchronization
const UINT_8 aMaxMACFrameSize
= aMaxPHYPacketSize - aMaxFrameOverhead;            //max # of octets that can be transmitted in the MAC frame payload field
const UINT_8 aMaxSIFSFrameSize              = 18;       //max size of a frame, in octets, that can be followed by a SIFS period
const UINT_16 aMinCAPLength             = 440;      //min # of symbols comprising the CAP
const UINT_8 aMinLIFSPeriod             = 40;       //min # of symbols comprising a LIFS period
const UINT_8 aMinSIFSPeriod             = 12;       //min # of symbols comprising a SIFS period
const UINT_16 aResponseWaitTime
= 32 * aBaseSuperframeDuration;             //max # of symbols a device shall wait for a response command following a request command
const UINT_8 aUnitBackoffPeriod             = 20;       //# of symbols comprising the basic time period used by the CSMA-CA algorithm

//---MAC_PIB default values (Tables 71,72)---
//attributes from Table 71
#define def_macAckWaitDuration          54          //22(ack) + 20(backoff slot) + 12(turnaround); propagation delay ignored?
#define def_macAssociationPermit        false
#define def_macAutoRequest          true
#define def_macBattLifeExt          false
#define def_macBattLifeExtPeriods       6
#define def_macBeaconPayload            ""
#define def_macBeaconPayloadLength      0
#define def_macBeaconOrder          15
#define def_macBeaconTxTime         0x000000
//#define def_macBSN                Random::random() % 0x100
//#define def_macCoordExtendedAddress       0xffffffffffffffffLL
#define def_macCoordExtendedAddress     0xffff
//not defined in draft
#define def_macCoordShortAddress        0xffff
//#define def_macDSN                Random::random() % 0x100
#define def_macGTSPermit            true
#define def_macMaxCSMABackoffs          4
#define def_macMinBE                3
#define def_macPANId                0xffff
#define def_macPromiscuousMode          false
#define def_macRxOnWhenIdle         true            // for non-beacon or supporting direct transmission in beacon-enabled
#define def_macShortAddress         0xffff
#define def_macSuperframeOrder          15
#define def_macTransactionPersistenceTime   0x01f4

//attributes from Table 72 (security attributes)
#define def_macACLEntryDescriptorSet        NULL
#define def_macACLEntryDescriptorSetSize    0x00
#define def_macDefaultSecurity          false
#define def_macACLDefaultSecurityMaterialLength 0x15
#define def_macDefaultSecurityMaterial      NULL
#define def_macDefaultSecuritySuite     0x00
#define def_macSecurityMode         0x00

#endif


