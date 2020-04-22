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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE8022LLCINSERTER_H
#define __INET_IEEE8022LLCINSERTER_H

#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8022LlcInserter : public PacketFlowBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    static const Protocol *getProtocol(const Ptr<const Ieee8022LlcHeader>& header);
};

} // namespace inet

#endif // ifndef __INET_IEEE8022LLCINSERTER_H

