//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPROTOCOLREGISTRATIONLISTENER_H
#define __INET_IPROTOCOLREGISTRATIONLISTENER_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

extern OPP_THREAD_LOCAL std::deque<std::string> registrationPath;

extern std::string printRegistrationPath();

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
INET_API void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut, cGate *responseIn = nullptr, cGate *confirmOut = nullptr);

INET_API void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *requestIn, cGate *indicationOut, cGate *responseIn = nullptr, cGate *confirmOut = nullptr);

INET_API void registerAnyService(cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerAnyService(cGate *requestIn, cGate *indicationOut, cGate *responseIn = nullptr, cGate *confirmOut = nullptr);

/**
 * Registers a protocol primitive (PDU processing) at the given gate.
 *
 * For example, IP receives protocol indications on lowerLayerIn and sends IP
 * protocol packets (e.g. Ethernet service requests) on lowerLayerOut.
 *
 * For another example, Ethernet receives protocol indications on lowerLayerIn
 * and sends protocol packets (e.g. transmission service requests) on lowerLayerOut.
 */
INET_API void registerProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn, cGate *responseOut = nullptr, cGate *confirmIn = nullptr);

INET_API void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *requestOut, cGate *indicationIn, cGate *responseOut = nullptr, cGate *confirmIn = nullptr);

INET_API void registerAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerAnyProtocol(cGate *requestOut, cGate *indicationIn, cGate *responseOut = nullptr, cGate *confirmIn = nullptr);

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
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override {}
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override {}

    virtual void handleRegisterServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override {}
    virtual void handleRegisterProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override {}

    virtual void handleRegisterAnyService(cGate *gate, ServicePrimitive servicePrimitive) override {}
    virtual void handleRegisterAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) override {}
};

#define Enter_Method2    cMethodCallContextSwitcher __ctx(check_and_cast<cComponent *>(this)); __ctx.methodCall

class INET_API TransparentProtocolRegistrationListener : public virtual IProtocolRegistrationListener
{
  public:
    virtual cGate *getRegistrationForwardingGate(cGate *gate) { return nullptr; }
    virtual void mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f) {
        auto forwardingGate = getRegistrationForwardingGate(gate);
        if (forwardingGate != nullptr)
            f(forwardingGate);
    }

    virtual bool isForwardingProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) const { return isForwardingProtocol(gate, servicePrimitive); }
    virtual bool isForwardingProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) const { return isForwardingProtocol(gate, servicePrimitive); }
    virtual bool isForwardingAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) const { return isForwardingProtocol(gate, servicePrimitive); }
    virtual bool isForwardingProtocol(cGate *gate, ServicePrimitive servicePrimitive) const {
        if (gate->getType() == cGate::INPUT)
            return servicePrimitive == SP_REQUEST || servicePrimitive == SP_RESPONSE;
        else if (gate->getType() == cGate::OUTPUT)
            return servicePrimitive == SP_INDICATION || servicePrimitive == SP_CONFIRM;
        else
            throw cRuntimeError("Unknown gate type");
    }

    virtual bool isForwardingService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) const { return isForwardingService(gate, servicePrimitive); }
    virtual bool isForwardingServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) const { return isForwardingService(gate, servicePrimitive); }
    virtual bool isForwardingAnyService(cGate *gate, ServicePrimitive servicePrimitive) const { return isForwardingService(gate, servicePrimitive); }
    virtual bool isForwardingService(cGate *gate, ServicePrimitive servicePrimitive) const {
        if (gate->getType() == cGate::INPUT)
            return servicePrimitive == SP_INDICATION || servicePrimitive == SP_CONFIRM;
        else if (gate->getType() == cGate::OUTPUT)
            return servicePrimitive == SP_REQUEST || servicePrimitive == SP_RESPONSE;
        else
            throw cRuntimeError("Unknown gate type");
    }

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterService");
        if (isForwardingService(protocol, gate, servicePrimitive)) {
            EV_INFO << "Forwarding service registration" << EV_FIELD(protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
            mapRegistrationForwardingGates(gate, [&] (cGate *gate) {
                registerService(protocol, gate, servicePrimitive);
            });
        }
    }

    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterProtocol");
        if ( isForwardingProtocol(protocol, gate, servicePrimitive)) {
            EV_INFO << "Forwarding protocol registration" << EV_FIELD(protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
            mapRegistrationForwardingGates(gate, [&] (cGate *gate) {
                registerProtocol(protocol, gate, servicePrimitive);
            });
        }
    }

    virtual void handleRegisterServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterServiceGroup");
        if (isForwardingServiceGroup(protocolGroup, gate, servicePrimitive)) {
            EV_INFO << "Forwarding service group registration" << EV_FIELD(protocolGroup) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
            mapRegistrationForwardingGates(gate, [&] (cGate *gate) {
                registerServiceGroup(protocolGroup, gate, servicePrimitive);
            });
        }
    }

    virtual void handleRegisterProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterProtocolGroup");
        if (isForwardingProtocolGroup(protocolGroup, gate, servicePrimitive)) {
            EV_INFO << "Forwarding protocol group registration" << EV_FIELD(protocolGroup) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
            mapRegistrationForwardingGates(gate, [&] (cGate *gate) {
                registerProtocolGroup(protocolGroup, gate, servicePrimitive);
            });
        }
    }

    virtual void handleRegisterAnyService(cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterAnyService");
        if (isForwardingAnyService(gate, servicePrimitive)) {
            EV_INFO << "Forwarding any service registration" << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
            mapRegistrationForwardingGates(gate, [&] (cGate *gate) {
                registerAnyService(gate, servicePrimitive);
            });
        }
    }

    virtual void handleRegisterAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) override {
        Enter_Method2("handleRegisterAnyProtocol");
        if (isForwardingAnyProtocol(gate, servicePrimitive)) {
            EV_INFO << "Forwarding any protocol registration" << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
            mapRegistrationForwardingGates(gate, [&] (cGate *gate) {
                registerAnyProtocol(gate, servicePrimitive);
            });
        }
    }
};

} // namespace inet

#endif

