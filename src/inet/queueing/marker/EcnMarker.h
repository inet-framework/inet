//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_ECNMARKER_H_
#define __INET_ECNMARKER_H_

#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/queueing/base/PacketMarkerBase.h"

namespace inet {
namespace queueing {

class INET_API EcnMarker : public PacketMarkerBase
{
  protected:
    virtual void markPacket(Packet *packet) override;

  public:
    static void setEcn(Packet *packet, IpEcnCode ecn);
    static IpEcnCode getEcn(Packet *packet);
};

} // namespace queueing
} // namespace inet

#endif // #ifndef __INET_ECNMARKER_H_

