//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RANDOMQOSCLASSIFIER_H
#define __INET_RANDOMQOSCLASSIFIER_H

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

/**
 * A QoS classifier that assigns a random User Priority. This is useful
 * for testing purposes.
 */
class INET_API RandomQosClassifier : public cSimpleModule, public DefaultProtocolRegistrationListener
{
  protected:
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  public:
    void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

