// SPDX-License-Identifier: LGPL-3.0-or-later
import inet.common.INETDefs;
namespace inet::ieee80211;

// IEEE Std 802.11-2024, 9.4.2.54: modeled fields in the fixed 26-octet HT Capabilities body;
// only the reserved bits are left out, except for those of the A-MPDU Parameters field, which
// some devices do set.
struct Ieee80211HtCapabilitiesElement
{
    bool ldpc;
    bool supportedChannelWidth40Mhz;
    short smPowerSave = 3; // SM Power Save: 0 static, 1 dynamic, 3 disabled
    bool greenfield;
    bool shortGi20;
    bool shortGi40;
    bool txStbc;
    short rxStbc; // number of spatial streams Rx STBC is supported for (0..3)
    bool htDelayedBlockAck;
    bool maxAmsduLength7935; // Maximum A-MSDU Length: 7935 octets if set, 3839 otherwise
    bool dsssCckModeIn40Mhz;
    bool fortyMhzIntolerant;
    bool lsigTxopProtectionSupport;
    short maxAmpduLengthExponent;
    short minimumMpduStartSpacing; // A-MPDU Parameters: Minimum MPDU Start Spacing (0..7)
    uint8_t ampduParametersReserved; // A-MPDU Parameters: the reserved bits B5-B7, as a value (0..7)
    bool rxMcsSupported[77];
    short rxHighestSupportedDataRate; // in Mb/s (0..1023); 0: not specified
    bool txMcsSetDefined;
    bool txRxMcsSetNotEqual;
    short txMaxNss;
    bool txUnequalModulation;
    uint16_t htExtendedCapabilities; // HT Extended Capabilities field, subfields not modeled
    uint32_t transmitBeamformingCapabilities; // Transmit Beamforming Capabilities field, subfields not modeled
    uint8_t aselCapabilities; // ASEL Capabilities field, subfields not modeled
}

// IEEE Std 802.11-2024, 9.4.2.55: modeled fields in the fixed 22-octet HT Operation body;
// only the reserved bits are left out.
struct Ieee80211HtOperationElement
{
    short primaryChannel;
    short secondaryChannelOffset;
    bool staChannelWidth40Mhz;
    bool rifsMode;
    short protectionMode;
    bool nongreenfieldHtStasPresent;
    bool obssNonHtStasPresent;
    short channelCenterFrequencySegment2; // 0..255
    bool dualBeacon;
    bool dualCtsProtection;
    bool stbcBeacon;
    bool basicMcsSupported[77];
}

// IEEE 802.11-2024,9.4.2.157: fixed12-octet VHT Capabilities body.
struct Ieee80211VhtCapabilitiesElement
{
    uint32_t capabilitiesInfo = 0;
    uint16_t rxMcsMap = 65535;
    uint16_t rxHighestSupportedDataRate = 0;
    uint16_t txMcsMap = 65535;
    uint16_t txHighestSupportedDataRate = 0;
}

// 9.4.2.158: fixed5-octet VHT Operation body.
struct Ieee80211VhtOperationElement
{
    uint8_t channelWidth = 0;
    uint8_t centerFrequencySegment0 = 0;
    uint8_t centerFrequencySegment1 = 0;
    uint16_t basicMcsMap = 65532;
}
