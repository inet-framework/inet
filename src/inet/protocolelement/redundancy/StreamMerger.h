//
// Copyright (C) 2020 OpenSim Ltd.
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

#ifndef __INET_STREAMMERGER_H
#define __INET_STREAMMERGER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamMerger : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    cValueMap *streamMapping = nullptr;
    int bufferSize = -1;

    std::map<std::string, std::vector<int>> sequenceNumbers;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;
    virtual bool matchesPacket(const Packet *packet) const override;

    virtual bool matchesInputStream(const char *streamName) const;
    virtual bool matchesSequenceNumber(const char *streamName, int sequenceNumber) const;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif
