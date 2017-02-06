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

#ifndef __INET_TUNSOCKET_H
#define __INET_TUNSOCKET_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"

namespace inet {

class INET_API TunSocket
{
  protected:
    int socketId = -1;
    int interfaceId = -1;
    cGate *outputGate = nullptr;

  protected:
    void sendToTun(cMessage *msg);

  public:
    TunSocket();
    ~TunSocket() {}

    void setOutputGate(cGate *outputGate) { this->outputGate = outputGate; }

    void open(int interfaceId);
    void send(Packet *packet);
    void close();

    /**
     * Returns the internal socket Id.
     */
    int getSocketId() const { return socketId; }
};

} // namespace inet

#endif // ifndef __INET_TUNSOCKET_H

