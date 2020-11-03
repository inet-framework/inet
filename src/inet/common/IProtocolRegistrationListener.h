//
// Copyright (C) 2012 OpenSim Ltd.
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

#ifndef __INET_IPROTOCOLREGISTRATIONLISTENER_H
#define __INET_IPROTOCOLREGISTRATIONLISTENER_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

/**
 * Registers a service primitive (SDU processing) at the given gate.
 *
 * For example, IP receives service requests on upperLayerIn from transport
 * protocols and sends service indications (e.g. TCP/UDP protocol indiciations)
 * on upperLayerOut.
 *
 * For another example, Ethernet receives service requests on upperLayerIn from
 * network protocols and sends service indications (e.g. IP protocol indications)
 * on upperLayerOut.
 */
INET_API void registerService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut);
INET_API void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut);

INET_API void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut);
INET_API void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *requestIn, cGate *indicationOut);

INET_API void registerAnyService(cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerAnyService(cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut);
INET_API void registerAnyService(cGate *requestIn, cGate *indicationOut);

/**
 * Registers a protocol primitive (PDU processing) at the given gate.
 *
 * For example, IP receives protocol indiciations on lowerLayerIn and sends IP
 * protocol packets (e.g. Ethernet service requests) on lowerLayerOut.
 *
 * For another example, Ethernet receives protocol indications on lowerLayerIn
 * and sends protocol packets (e.g. transmission service requests) on lowerLayerOut.
 */
INET_API void registerProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn);
INET_API void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn);

INET_API void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn);
INET_API void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *requestOut, cGate *indicationIn);

INET_API void registerAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerAnyProtocol(cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn);
INET_API void registerAnyProtocol(cGate *requestOut, cGate *indicationIn);

/**
 * This interface defines methods that are called during protocol service
 * primitve registration.
 */
class INET_API IProtocolRegistrationListener
{
  public:
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) = 0;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) = 0;

    virtual void handleRegisterServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) = 0;
    virtual void handleRegisterProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) = 0;

    virtual void handleRegisterAnyService(cGate *gate, ServicePrimitive servicePrimitive) = 0;
    virtual void handleRegisterAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) = 0;
};

class INET_API DefaultProtocolRegistrationListener : public virtual IProtocolRegistrationListener
{
  public:
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override { }
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override { }

    virtual void handleRegisterServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override { }
    virtual void handleRegisterProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override { }

    virtual void handleRegisterAnyService(cGate *gate, ServicePrimitive servicePrimitive) override { }
    virtual void handleRegisterAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) override { }
};

#define Enter_Method2 cMethodCallContextSwitcher __ctx(check_and_cast<cComponent *>(this)); __ctx.methodCall

class INET_API TransparentProtocolRegistrationListener : public virtual IProtocolRegistrationListener
{
  public:
    virtual cGate *getRegistrationForwardingGate(cGate *gate) = 0;

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterService");
        registerService(protocol, getRegistrationForwardingGate(gate), servicePrimitive);
    }

    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterProtocol");
        registerProtocol(protocol, getRegistrationForwardingGate(gate), servicePrimitive);
    }

    virtual void handleRegisterServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterServiceGroup");
        registerServiceGroup(protocolGroup, getRegistrationForwardingGate(gate), servicePrimitive);
    }

    virtual void handleRegisterProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterProtocolGroup");
        registerProtocolGroup(protocolGroup, getRegistrationForwardingGate(gate), servicePrimitive);
    }

    virtual void handleRegisterAnyService(cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterAnyService");
        registerAnyService(getRegistrationForwardingGate(gate), servicePrimitive);
    }

    virtual void handleRegisterAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterAnyProtocol");
        registerAnyProtocol(getRegistrationForwardingGate(gate), servicePrimitive);
    }
};

} // namespace inet

#endif

