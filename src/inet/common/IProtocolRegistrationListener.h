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

} // namespace inet

#endif

