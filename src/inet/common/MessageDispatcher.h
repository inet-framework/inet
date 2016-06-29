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

#ifndef __INET_MESSAGEDISPATCHER_H
#define __INET_MESSAGEDISPATCHER_H

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

/**
 * This module connects multiple protocols to multiple other protocols.
 *
 * Configuring the dispatch mechanism:
 *  - protocols must register by calling registerProtocol
 *  - interfaces must register by calling registerInterface
 *  - sockets must register by sending socket commands
 *  - packets must have a proper control info attached that specifies the
 *    destination protocol, interface or socket
 */
class INET_API MessageDispatcher : public cSimpleModule, public IProtocolRegistrationListener, public IInterfaceRegistrationListener
{
    protected:
        std::map<int, int> socketIdToUpperLayerGateIndex;
        std::map<int, int> interfaceIdToLowerLayerGateIndex;
        std::map<int, int> protocolIdToUpperLayerGateIndex;
        std::map<int, int> protocolIdToLowerLayerGateIndex;

    protected:
        virtual void initialize() override;
        virtual void arrived(cMessage *message, cGate *inGate, simtime_t t) override;
        virtual cGate *handleUpperLayerPacket(cMessage *message, cGate *inGate);
        virtual cGate *handleLowerLayerPacket(cMessage *message, cGate *inGate);
        virtual cGate *handleUpperLayerCommand(cMessage *message, cGate *inGate);
        virtual cGate *handleLowerLayerCommand(cMessage *message, cGate *inGate);

        virtual int computeSocketReqSocketId(cMessage *message);
        virtual int computeSocketIndSocketId(cMessage *message);
        virtual int computeInterfaceId(cMessage *message);
        virtual const Protocol *computeProtocol(cMessage *message);

    public:
        MessageDispatcher();

        virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate) override;
        virtual void handleRegisterInterface(const InterfaceEntry &interface, cGate *gate) override;
};

} // namespace inet

#endif // ifndef __INET_MESSAGEDISPATCHER_H

