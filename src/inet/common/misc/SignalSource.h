//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALSOURCE_H
#define __INET_SIGNALSOURCE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API SignalSource : public cSimpleModule
{
  protected:
    simtime_t startTime, endTime;
    simsignal_t signal = -1;

  public:
    SignalSource() {}

  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
};

} // namespace inet

#endif

