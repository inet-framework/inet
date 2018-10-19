//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//

#ifndef __INET_ETHERNETDISSECTOR_H_
#define __INET_ETHERNETDISSECTOR_H_

#include "inet/common/INETDefs.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {

class INET_API EthernetMacDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

class INET_API EthernetPhyDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace inet

#endif // __INET_ETHERNETDISSECTOR_H_
