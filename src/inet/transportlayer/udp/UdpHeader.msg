//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.common.CrcMode;
import inet.transportlayer.contract.TransportHeaderBase;

cplusplus {{
#include "inet/common/INETUtils.h"
}}

namespace inet;

cplusplus {{
const B UDP_HEADER_LENGTH = B(8);
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
    uint16_t crc = 0 @toString(utils::hex($)) @fromString(utils::uhex($));
    CrcMode crcMode = CRC_MODE_UNDEFINED;
}

cplusplus(UdpHeader) {{
  public:
    virtual unsigned int getSourcePort() const override { return getSrcPort(); }
    virtual void setSourcePort(unsigned int port) override { setSrcPort(port); }
    virtual unsigned int getDestinationPort() const override { return getDestPort(); }
    virtual void setDestinationPort(unsigned int port) override { setDestPort(port); }
}}
