//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


import inet.common.INETDefs;
import inet.transportlayer.common.CrcMode;
import inet.transportlayer.contract.TransportHeaderBase;

namespace inet;


cplusplus {{
const B UDP_HEADER_LENGTH = B(10);
}}

//
// Represents an Udp header, to be used with the ~Udp module.
//
class UdpHeader extends TransportHeaderBase
{
    unsigned short srcPort;
    unsigned short destPort;
    chunkLength = UDP_HEADER_LENGTH;
    B totalLengthField = B(-1);   // UDP header + payload in bytes
    uint16_t crc = 0;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
}

cplusplus(UdpHeader) {{
  public:
    virtual std::string str() const override;

    virtual unsigned int getSourcePort() const override { return getSrcPort(); }
    virtual void setSourcePort(unsigned int port) override { setSrcPort(port); }
    virtual unsigned int getDestinationPort() const override { return getDestPort(); }
    virtual void setDestinationPort(unsigned int port) override { setDestPort(port); }
}}

