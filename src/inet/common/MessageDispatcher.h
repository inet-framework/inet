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
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"

namespace inet {

/**
 * This class implements the corresponding module. See module documentation for more details.
 */
class INET_API MessageDispatcher : public cSimpleModule, public IProtocolRegistrationListener, public IInterfaceRegistrationListener
{
  public:
    class Key
    {
      protected:
        int protocolId;
        int servicePrimitive;

      public:
        Key(int protocolId, ServicePrimitive servicePrimitive) : protocolId(protocolId), servicePrimitive(servicePrimitive) {}

        bool operator<(const MessageDispatcher::Key& other) const {
            if (protocolId < other.protocolId)
                return true;
            else if (protocolId > other.protocolId)
                return false;
            else
                return servicePrimitive < other.servicePrimitive;
        }
        friend std::ostream& operator<<(std::ostream& out, const MessageDispatcher::Key& foo);
    };

  protected:
    std::map<int, int> socketIdToGateIndex;
    std::map<int, int> interfaceIdToGateIndex;
    std::map<Key, int> serviceToGateIndex;
    std::map<Key, int> protocolToGateIndex;

  protected:
    virtual void initialize() override;
    virtual void arrived(cMessage *message, cGate *inGate SENDOPTIONS_ARG, simtime_t t) override;
    virtual cGate *handlePacket(Packet *packet, cGate *inGate);
    virtual cGate *handleMessage(Message *request, cGate *inGate);

  public:
    virtual void handleRegisterInterface(const InterfaceEntry &interface, cGate *out, cGate *in) override;
    virtual void handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive) override;
};

std::ostream& operator<<(std::ostream& out, const MessageDispatcher::Key& foo) {
    out << "[" << foo.protocolId << ", " << omnetpp::cEnum::get("inet::ServicePrimitive")->getStringFor(foo.servicePrimitive) << "]";
    return out;
}

} // namespace inet

#endif // ifndef __INET_MESSAGEDISPATCHER_H

