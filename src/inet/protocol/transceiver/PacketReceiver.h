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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETRECEIVER_H
#define __INET_PACKETRECEIVER_H

#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/protocol/transceiver/base/PacketReceiverBase.h"

namespace inet {

using namespace inet::units::values;
using namespace inet::physicallayer;

class INET_API PacketReceiver : public PacketReceiverBase
{
  protected:
    bps datarate = bps(NaN);

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void receiveSignal(Signal *signal);
};

} // namespace inet

#endif // ifndef __INET_PACKETRECEIVER_H

