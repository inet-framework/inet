//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004, 2009 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TlvOptions;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.contract.NetworkHeaderBase;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.transportlayer.common.CrcMode;

cplusplus {{
#include "inet/common/ProtocolGroup.h"
}}

namespace inet;

cplusplus {{
// default Ipv4 header length: 20 bytes
const B IPv4_MIN_HEADER_LENGTH = B(20);

// maximum Ipv4 header length (base+options): 60 = 4 * 15 bytes
const B IPv4_MAX_HEADER_LENGTH = B(60);

// option type fields
const unsigned char IPOPTION_COPY_MASK   = 0x80;
const unsigned char IPOPTION_CLASS_MASK  = 0x60;
const unsigned char IPOPTION_NUMBER_MASK = 0x1F;

// option entry number
const unsigned int MAX_IPADDR_OPTION_ENTRIES = 9;
const unsigned int MAX_TIMESTAMP_OPTION_ENTRIES = 4;

}}

//
// Ipv4 options class
//
enum Ipv4OptionClass
{
    IPOPTION_CLASS_CONTROL = 0;
    IPOPTION_CLASS_RESERVED = 1;
    IPOPTION_CLASS_DEBUGGING = 2;
    IPOPTION_CLASS_RESERVED2 = 3;
};

//
// Ipv4 option types
//
enum Ipv4OptionType
{
    IPOPTION_END_OF_OPTIONS = 0;
    IPOPTION_NO_OPTION = 1;
    IPOPTION_STREAM_ID = 136;

    IPOPTION_TIMESTAMP = 68;

    IPOPTION_SECURITY = 130;
    IPOPTION_LOOSE_SOURCE_ROUTING = 131;
    IPOPTION_RECORD_ROUTE = 7;
    IPOPTION_STRICT_SOURCE_ROUTING = 137;
    IPOPTION_ROUTER_ALERT = 148;

    IPOPTION_TLV_GPSR = 47;
};

//
// The timestamp flag uses the same numeric values as the Ipv4 Protocol
//
enum TimestampFlag
{
    IP_TIMESTAMP_TIMESTAMP_ONLY = 0;
    IP_TIMESTAMP_WITH_ADDRESS = 1;
    IP_TIMESTAMP_SENDER_INIT_ADDRESS = 3;
}

class Ipv4Option extends TlvOptionBase
{
}

class Ipv4OptionNop extends Ipv4Option
{
    type = IPOPTION_NO_OPTION;
    length = 1;
}

class Ipv4OptionEnd extends Ipv4Option
{
    type = IPOPTION_END_OF_OPTIONS;
    length = 1;
}

//
// Option structure: Record Route
//
class Ipv4OptionRecordRoute extends Ipv4Option
{
    // type = IPOPTION_RECORD_ROUTE;
    // type = IPOPTION_LOOSE_SOURCE_ROUTING
    // type = IPOPTION_STRICT_SOURCE_ROUTING
    // length = 3 + 4 * getRecordAddressArraySize();
    short nextAddressIdx;
    // pointer = 4 + 4 * nextAddressIdx
    Ipv4Address recordAddress[];    // max size is 9
}

//
// Option structure: Timestamp
//
class Ipv4OptionTimestamp extends Ipv4Option
{
    type = IPOPTION_TIMESTAMP;
    // length = 4 + 4 * getRecordAddressArraySize() + 4 * getRecordTimestampArraySize();
    TimestampFlag flag;
    short overflow;
    short nextIdx;
    // pointer = 5 + (flag == IP_TIMESTAMP_TIMESTAMP_ONLY ? 4 : 8) * nextIdx

    // use either up to 4 addresses with timestamps or
    // only up to 9 timestamps, according to the flag
    Ipv4Address recordAddress[];
    simtime_t recordTimestamp[];
}

//
// Option Structure: Stream ID
//
class Ipv4OptionStreamId extends Ipv4Option
{
    type = IPOPTION_STREAM_ID;
    length = 4;
    short streamId;
}

//
// Option Structure: Stream ID
//
class Ipv4OptionRouterAlert extends Ipv4Option
{
    type = IPOPTION_ROUTER_ALERT;
    length = 4;
    uint16_t routerAlert = 0;
}

class Ipv4OptionUnknown extends Ipv4Option
{
    // type = any
    // length = 2 + getBytesArraySize()
    uint8_t bytes[];
}

//
// Represents an Ipv4 datagram.
//
// Uses the following cPacket fields:
//    - getByteLength() / setByteLength() to represent total datagram length
//    - hasBitError() / setBitError() to represent datagram correctness
//    - getTimestamp() / setTimestamp (simtime) used in timestamp option
//
// Additional length fields defined in this class are in bytes.
//
// Only one of the option fields can exist at a time.
//
// TODO cleanup order of fields
class Ipv4Header extends NetworkHeaderBase
{
    chunkLength = IPv4_MIN_HEADER_LENGTH;

    short version = 4;      // @bit(4)
    B headerLength = IPv4_MIN_HEADER_LENGTH;         // @bit(4), bytes, must be multiple of 4
    short typeOfService;    // @bit(8)
    // OMNeT++ 6.0:
    // short __dscp @custom @getter(getDscp) @setter(setDscp);  // @bit(6), maps to bits 0-5 of ToS, stored in typeOfService
    // short __ecn @custom @getter(getEcn) @setter(setEcn);  // @bit(2),  maps to bits 6-7 of ToS, stored in typeOfService

    B totalLengthField;    // @byte(2)

    uint16_t identification;    // @byte(2)
    bool reservedBit = false;    // @bit(1)
    bool moreFragments;    // @bit(1)
    bool dontFragment;    // @bit(1)
    uint16_t fragmentOffset;         // @bit(13), must be multiple of 8

    short timeToLive;    // @byte(1)
    IpProtocolId protocolId = IP_PROT_NONE;    // @byte(1)
    uint16_t crc = 0;    // @byte(2)
    CrcMode crcMode = CRC_MODE_UNDEFINED;

    Ipv4Address srcAddress;

    Ipv4Address destAddress;

    TlvOptions options; // array of option pointers, option pointers owned by datagram
}

cplusplus(Ipv4Header) {{
  public:
    virtual short getDscp() const;
    virtual void setDscp(short dscp);
    virtual short getEcn() const;
    virtual void setEcn(short ecn);

    /**
     * Returns the number of extension headers in this datagram
     */
    virtual unsigned int getOptionArraySize() const { return options.getTlvOptionArraySize(); }

    /**
     * Returns the kth extension header in this datagram
     */
    virtual TlvOptionBase& getOptionForUpdate(unsigned int k) { handleChange(); return *CHK((options.getTlvOptionForUpdate(k))); }
    virtual const TlvOptionBase& getOption(unsigned int k) const { return *CHK((options.getTlvOption(k))); }

    /**
     * Returns the TlvOptionBase of the specified type,
     * or nullptr. If index is 0, then the first, if 1 then the
     * second option is returned.
     */
    virtual TlvOptionBase *findMutableOptionByType(short int optionType, int index = 0);
    virtual const TlvOptionBase *findOptionByType(short int optionType, int index = 0) const;

    /**
     * Adds an TlvOptionBase to the datagram.
     */
    virtual void addOption(TlvOptionBase *opt);
    virtual void addOption(TlvOptionBase *opt, int atPos);

    /**
     * Calculates the length of the Ipv6 header plus the extension
     * headers.
     */
    virtual B calculateHeaderByteLength() const;

    // implements NetworkHeaderBase:
    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddress()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddress(address.toIpv4()); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddress()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddress(address.toIpv4()); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::getIpProtocolGroup()->findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return getMoreFragments() || (getFragmentOffset() != 0); }

    // utility: calculate and set CRC based on crcMode
    virtual void updateCrc();

    // utility: verify CRC
    virtual bool verifyCrc() const;
}}
