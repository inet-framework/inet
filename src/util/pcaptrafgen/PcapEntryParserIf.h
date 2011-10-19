//
// Copyright (C) 2011 OpenSim Ltd
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
// @author Zoltan Bojthe
//

#ifndef __INET_UTIL_PCAPENTRYPARSERIF_H
#define __INET_UTIL_PCAPENTRYPARSERIF_H


#include "INETDefs.h"


/**
 * pcap entry parser interface class.
 *
 * Convert pcap entry to cPacket
 * @see InetPcapFileReader, PcapTrafficGenerator
 */
class PcapEntryParserIf : public cObject
{
  public:
    virtual ~PcapEntryParserIf() {};

    /** converts pcap entry from buf to a new cPacket or returns NULL */
    virtual cPacket* parse(const unsigned char *buf, uint32_t caplen, uint32_t totlen) = 0;
};

#endif //__INET_UTIL_PCAPENTRYPARSERIF_H
