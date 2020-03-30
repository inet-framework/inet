//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
    virtual std::string str() const override;

    virtual unsigned int getSourcePort() const override { return getSrcPort(); }
    virtual void setSourcePort(unsigned int port) override { setSrcPort(port); }
    virtual unsigned int getDestinationPort() const override { return getDestPort(); }
    virtual void setDestinationPort(unsigned int port) override { setDestPort(port); }
}}

