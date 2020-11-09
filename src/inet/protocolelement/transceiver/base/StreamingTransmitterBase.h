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

#ifndef __INET_STREAMINGTRANSMITTERBASE_H
#define __INET_STREAMINGTRANSMITTERBASE_H

#include "inet/protocolelement/transceiver/base/PacketTransmitterBase.h"

namespace inet {

class INET_API StreamingTransmitterBase : public PacketTransmitterBase
{
  protected:
    cChannel *transmissionChannel = nullptr;

    simtime_t txStartTime = -1;
    clocktime_t txStartClockTime = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual void abortTx() = 0;

  public:
    virtual bool canPushSomePacket(cGate *gate) const override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

