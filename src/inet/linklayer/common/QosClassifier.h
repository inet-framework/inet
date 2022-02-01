//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSCLASSIFIER_H
#define __INET_QOSCLASSIFIER_H

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

/**
 * This module classifies and assigns User Priority to packets.
 */
class INET_API QosClassifier : public cSimpleModule, public DefaultProtocolRegistrationListener
{
  protected:
    int defaultUp;
    std::map<int, int> ipProtocolUpMap;
    std::map<int, int> udpPortUpMap;
    std::map<int, int> tcpPortUpMap;

    virtual int parseUserPriority(const char *text);
    virtual void parseUserPriorityMap(const char *text, std::map<int, int>& upMap);

    virtual int getUserPriority(cMessage *msg);

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  public:
    QosClassifier() {}

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

