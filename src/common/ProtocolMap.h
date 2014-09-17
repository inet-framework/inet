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

#ifndef __INET_PROTOCOLMAP_H
#define __INET_PROTOCOLMAP_H

#include <vector>
#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Maps protocol numbers to output gates
 */
class INET_API ProtocolMapping
{
  protected:
    struct Entry
    {
        int protocolNumber;
        int outGateIndex;

        Entry() {}
        Entry(int protocolNumber, int outGateIndex) { this->protocolNumber = protocolNumber; this->outGateIndex = outGateIndex; }
    };
    // we use vector because it's probably faster: we'll hit 1st or 2nd entry
    // (TCP or UDP) in 90% of cases
    typedef std::vector<Entry> Entries;
    Entries entries;

  public:
    ProtocolMapping() {}
    ~ProtocolMapping() {}
    void addProtocolMapping(int protocol, int gateIndex);
    void parseProtocolMapping(const char *s);

    /** find output gate index for protocol ID and returns it. Returns -2 if not found. */
    int findOutputGateForProtocol(int protocol) const;

    /** find output gate index for protocol ID and returns it. Throws an error if not found. */
    int getOutputGateForProtocol(int protocol) const;
};

} // namespace inet

#endif // ifndef __INET_PROTOCOLMAP_H

