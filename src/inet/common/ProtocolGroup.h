//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_PROTOCOLGROUP_H
#define __INET_PROTOCOLGROUP_H

#include "inet/common/Protocol.h"

namespace inet {

class INET_API ProtocolGroup
{
  protected:
    const char *name;
    std::vector<const Protocol *> protocols;
    std::map<int, const Protocol *> protocolNumberToProtocol;
    std::map<const Protocol *, int> protocolToProtocolNumber;

  public:
    ProtocolGroup(const char *name, std::map<int, const Protocol *> protocolNumberToProtocol);

    const char *getName() const { return name; }
    int getNumElements() const { return protocols.size(); }
    const Protocol *getElement(int index) const { return protocols.at(index); }

    const Protocol *findProtocol(int protocolNumber) const;
    const Protocol *getProtocol(int protocolNumber) const;
    int findProtocolNumber(const Protocol *protocol) const;
    int getProtocolNumber(const Protocol *protocol) const;
    void addProtocol(int protocolId, const Protocol *protocol);

    std::string str() const { return name; }

  public:
    // in alphanumeric order
    static ProtocolGroup ethertype;
    static ProtocolGroup ieee8022protocol;
    static ProtocolGroup ipprotocol;
    static ProtocolGroup pppprotocol;
    static ProtocolGroup snapOui;
    static ProtocolGroup tcpprotocol;
    static ProtocolGroup udpprotocol;
};

inline std::ostream& operator<<(std::ostream& o, const ProtocolGroup& t) { o << t.str(); return o; }

} // namespace inet

#endif

