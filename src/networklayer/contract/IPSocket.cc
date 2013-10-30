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

#include "IPSocket.h"
#include "IPProtocolId_m.h"

void IPSocket::registerProtocol(int protocol)
{
    if (gateToIP && gateToIP->isConnected()) {
        IPRegisterProtocolCommand * controlInfo = new IPRegisterProtocolCommand();
        controlInfo->setProtocol(protocol);
        cMessage * message = new cMessage("RegisterProtocol", IP_C_REGISTER_PROTOCOL);
        message->setControlInfo(controlInfo);
        sendToIP(message);
    }
}

void IPSocket::sendToIP(cMessage * message)
{
    if (!gateToIP)
        throw cRuntimeError("IPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<InetSimpleModule *>(gateToIP->getOwnerModule())->sendSync(message, gateToIP);
}
