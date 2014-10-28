//
// Copyright (C) 2013 Andras Varga
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

#ifndef __INET_IPSOCKET_H_
#define __INET_IPSOCKET_H_

#include "INETDefs.h"

#define IP_C_REGISTER_PROTOCOL 1199

/**
 * TODO
 */
class IPSocket {
  private:
    cGate * gateToIP;

  protected:
    void sendToIP(cMessage * message);

  public:
    IPSocket(cGate * gateToIP = NULL) { this->gateToIP = gateToIP; }
    virtual ~IPSocket() { }

    /**
     * Sets the gate on which to send to IP. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("ipOut"));</tt>
     */
    void setOutputGate(cGate * gateToIP) { this->gateToIP = gateToIP;}

    /**
     * Registers the given IP protocol to the connected gate.
     */
    void registerProtocol(int protocol);
};

#endif
