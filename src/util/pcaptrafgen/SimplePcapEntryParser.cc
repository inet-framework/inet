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

#include <stdio.h>

#include "SimplePcapEntryParser.h"

#include "ByteArrayMessage.h"


Register_Class(SimplePcapEntryParser);

cPacket* SimplePcapEntryParser::parse(const unsigned char *buf, uint32 caplen, uint32 totlen)
{
    char str[50];
    sprintf(str, "PCAP:%u/%u bytes", caplen, totlen);
    ByteArrayMessage* ret = new ByteArrayMessage(str);
    ret->setDataFromBuffer(buf, caplen);
    ret->setByteLength(totlen);
    return ret;
}

