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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_STREAMINGTRANSMITTER_H
#define __INET_STREAMINGTRANSMITTER_H

#include "inet/protocol/transceiver/base/StreamingTransmitterBase.h"

namespace inet {

class INET_API StreamingTransmitter : public StreamingTransmitterBase
{
  protected:
    clocktime_t txStartTime = -1;

  protected:
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void startTx(Packet *packet);
    virtual void endTx();
    virtual void abortTx() override;

    virtual void scheduleTxEndTimer(Signal *signal);

  public:
    virtual bool supportsPacketStreaming(cGate *gate) const override { return gate == outputGate; }

    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

