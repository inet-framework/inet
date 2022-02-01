//
// Copyright (C) 2010 Alfonso Ariza
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_EXAMPLEQOSCLASSIFIER_H
#define __INET_EXAMPLEQOSCLASSIFIER_H

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

/**
 * An example packet classifier based on the UDP/TCP port number.
 */
class INET_API ExampleQosClassifier : public cSimpleModule, public DefaultProtocolRegistrationListener
{
  protected:
    virtual int getUserPriority(cMessage *msg);
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  public:
    ExampleQosClassifier() {}
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

