//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::physicallayer;

//
// The bit numbers of the Radiotap fields in the present word, see
// https://www.radiotap.org/fields/defined.
//
enum Ieee80211RadiotapField
{
    RADIOTAP_TSFT = 0;
    RADIOTAP_FLAGS = 1;
    RADIOTAP_RATE = 2;
    RADIOTAP_CHANNEL = 3;
    RADIOTAP_DBM_ANTSIGNAL = 5;
    RADIOTAP_DBM_ANTNOISE = 6;
    RADIOTAP_DBM_TX_POWER = 10;
    RADIOTAP_MCS = 19;
    RADIOTAP_AMPDU_STATUS = 20;
    RADIOTAP_VHT = 21;
}

//
// The bits of the Radiotap Flags field.
//
enum Ieee80211RadiotapFlag
{
    RADIOTAP_FLAG_SHORT_PREAMBLE = 0x02;
    RADIOTAP_FLAG_FCS_AT_END = 0x10;
    RADIOTAP_FLAG_BAD_FCS = 0x40;
}

//
// The bits of the flags of the Radiotap Channel field.
//
enum Ieee80211RadiotapChannelFlag
{
    RADIOTAP_CHANNEL_CCK = 0x0020;
    RADIOTAP_CHANNEL_OFDM = 0x0040;
    RADIOTAP_CHANNEL_2GHZ = 0x0080;
    RADIOTAP_CHANNEL_5GHZ = 0x0100;
}

//
// The bits of the known and flags octets of the Radiotap MCS field.
//
enum Ieee80211RadiotapMcsBit
{
    RADIOTAP_MCS_KNOWN_BANDWIDTH = 0x01;
    RADIOTAP_MCS_KNOWN_INDEX = 0x02;
    RADIOTAP_MCS_KNOWN_GUARD_INTERVAL = 0x04;
    RADIOTAP_MCS_KNOWN_HT_FORMAT = 0x08;
    RADIOTAP_MCS_KNOWN_FEC = 0x10;
    RADIOTAP_MCS_KNOWN_STBC = 0x20;
    RADIOTAP_MCS_KNOWN_EXTENSION_STREAMS = 0x40;
    RADIOTAP_MCS_FLAG_BANDWIDTH_40 = 0x01;
    RADIOTAP_MCS_FLAG_SHORT_GUARD_INTERVAL = 0x04;
    RADIOTAP_MCS_FLAG_GREENFIELD = 0x08;
}

//
// The bits of the known word and the flags octet of the Radiotap VHT field.
//
enum Ieee80211RadiotapVhtBit
{
    RADIOTAP_VHT_KNOWN_STBC = 0x0001;
    RADIOTAP_VHT_KNOWN_GUARD_INTERVAL = 0x0004;
    RADIOTAP_VHT_KNOWN_BANDWIDTH = 0x0040;
    RADIOTAP_VHT_FLAG_SHORT_GUARD_INTERVAL = 0x04;
}

//
// The Radiotap header, which comes before the IEEE 802.11 frame in a capture
// record of link type 127 (LINKTYPE_IEEE802_11_RADIOTAP). The header contains
// only the fields whose bit is set in the present word. The chunk length
// depends on these fields and on their alignment; set it with
// computeIeee80211RadiotapHeaderLength() after the present word is set.
//
// The serializer writes the version, the padding, the length and one present
// word itself. All values are little-endian, as the format specifies.
//
class Ieee80211RadiotapHeader extends FieldsChunk
{
    chunkLength = B(8);
    uint32_t present = 0;           // one bit for each field below, see Ieee80211RadiotapField
    uint64_t tsft = 0;              // the time of the first bit of the frame, in microseconds
    uint8_t flags = 0;              // see Ieee80211RadiotapFlag
    uint8_t rate = 0;               // the legacy data rate, in units of 500 kb/s
    uint16_t channelFrequency = 0;  // the center frequency of the channel, in MHz
    uint16_t channelFlags = 0;      // see Ieee80211RadiotapChannelFlag
    int8_t antennaSignal = 0;       // the signal power at the antenna, in dBm
    int8_t antennaNoise = 0;        // the noise power at the antenna, in dBm
    int8_t txPower = 0;             // the transmit power, in dBm
    uint8_t mcsKnown = 0;           // see Ieee80211RadiotapMcsBit
    uint8_t mcsFlags = 0;           // see Ieee80211RadiotapMcsBit
    uint8_t mcsIndex = 0;           // the HT MCS index
    uint32_t ampduReference = 0;    // the same number in all frames of one A-MPDU
    uint16_t ampduFlags = 0;
    uint8_t ampduDelimiterCrc = 0;
    uint16_t vhtKnown = 0;          // see Ieee80211RadiotapVhtBit
    uint8_t vhtFlags = 0;           // see Ieee80211RadiotapVhtBit
    uint8_t vhtBandwidth = 0;       // the code of the bandwidth: 0 for 20 MHz, 1 for 40 MHz, 4 for 80 MHz, 11 for 160 MHz
    uint8_t vhtMcsNss[4];           // for each user: the MCS index in the high 4 bits, the number of spatial streams in the low 4 bits
    uint8_t vhtCoding = 0;          // one bit for each user: 0 for BCC, 1 for LDPC
    uint8_t vhtGroupId = 0;
    uint16_t vhtPartialAid = 0;
}
