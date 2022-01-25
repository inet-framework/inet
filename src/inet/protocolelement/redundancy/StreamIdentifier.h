//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __INET_STREAMIDENTIFIER_H
#define __INET_STREAMIDENTIFIER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamIdentifier : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    bool hasSequenceNumbering = false;
    cValueArray *mapping = nullptr;

    std::map<std::string, int> sequenceNumbers;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;
    virtual int incrementSequenceNumber(const char *stream);

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

