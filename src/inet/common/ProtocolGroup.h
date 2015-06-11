//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_PROTOCOLGROUP_H
#define __INET_PROTOCOLGROUP_H

#include "inet/common/Protocol.h"

namespace inet {

class INET_API ProtocolGroup
{
  protected:
    const char *name;
    std::map<int, const Protocol *> protocolNumberToProtocol;
    std::map<const Protocol *, int> protocolToProtocolNumber;

  public:
    ProtocolGroup(const char *name, std::map<int, const Protocol *> protocolNumberToProtocol);

    const char *getName() const { return name; }

    const Protocol *findProtocol(int protocolNumber) const;
    const Protocol *getProtocol(int protocolNumber) const;
    int findProtocolNumber(const Protocol *protocol) const;
    int getProtocolNumber(const Protocol *protocol) const;

  public:
    // in alphanumeric order
    static const ProtocolGroup ethertype;
    static const ProtocolGroup ipprotocol;
};

} // namespace inet

#endif // ifndef __INET_PROTOCOLGROUP_H

