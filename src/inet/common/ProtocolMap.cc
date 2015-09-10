//
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

#include "inet/common/INETDefs.h"

#include "inet/common/ProtocolMap.h"

namespace inet {

void ProtocolMapping::parseProtocolMapping(const char *s)
{
    while (isspace(*s))
        s++;

    while (*s) {
        Entry entry(-1, -1);

        if (!isdigit(*s))
            throw cRuntimeError("Syntax error: protocol number expected");
        entry.protocolNumber = atoi(s);
        while (isdigit(*s))
            s++;

        if (*s++ != ':')
            throw cRuntimeError("Syntax error: colon expected");

        while (isspace(*s))
            s++;
        if (!isdigit(*s))
            throw cRuntimeError("Syntax error in script: output gate index expected");
        entry.outGateIndex = atoi(s);
        while (isdigit(*s))
            s++;

        // add
        entries.push_back(entry);

        // skip delimiter
        while (isspace(*s))
            s++;
        if (!*s)
            break;
        if (*s++ != ',')
            throw cRuntimeError("Syntax error: comma expected");
        while (isspace(*s))
            s++;
    }
}

int ProtocolMapping::findOutputGateForProtocol(int protocol) const
{
    for (const auto & elem : entries)
        if (elem.protocolNumber == protocol)
            return elem.outGateIndex;


    return -2;    // illegal gateindex
}

int ProtocolMapping::getOutputGateForProtocol(int protocol) const
{
    int ret = findOutputGateForProtocol(protocol);
    if (ret >= -1)
        return ret;
    throw cRuntimeError("No output gate defined in protocolMapping for protocol number %d", protocol);
}

void ProtocolMapping::addProtocolMapping(int protocol, int gateIndex)
{
    int registered = findOutputGateForProtocol(protocol);
    if (registered == -2)
        entries.push_back(Entry(protocol, gateIndex));
    else if (registered == gateIndex)
        EV_WARN << "The protocol " << protocol << " already registered to gate index=" << gateIndex << endl;
    else
        throw cRuntimeError("The protocol %d should not register to gate index=%d because it already registered to gate index=%d.", protocol, gateIndex, registered);
}

} // namespace inet

