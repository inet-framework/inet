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

#ifndef __INET_ETHERNETFRAGMENTFCSINSERTER_H
#define __INET_ETHERNETFRAGMENTFCSINSERTER_H

#include "inet/protocol/checksum/base/FcsInserterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetFragmentFcsInserter : public FcsInserterBase
{
  protected:
    uint32_t lastFragmentCompleteFcs = 0;
    mutable uint32_t currentFragmentCompleteFcs = 0;

  protected:
    virtual uint32_t computeComputedFcs(const Packet *packet) const override;
    virtual uint32_t computeFcs(const Packet *packet, FcsMode fcsMode) const override;
    virtual void processPacket(Packet *packet) override;
    virtual void handlePacketProcessed(Packet *packet) override;
};

} // namespace inet

#endif // ifndef __INET_ETHERNETFRAGMENTFCSINSERTER_H

