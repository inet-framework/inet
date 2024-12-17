//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_HOTSTANDBY_H
#define __INET_HOTSTANDBY_H

#include <omnetpp/cmodule.h>
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee8021as/Gptp.h"

namespace inet {

class INET_API HotStandby : public ClockUserModuleBase, public cListener
{
  protected:
    std::map<uint8_t , clocktime_t> gptpSyncTime; // store each gptp sync time
  public:
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t simSignal, const SimTime& t, cObject *details) override;
    void handleGptpSyncSuccessfulSignal(const Gptp *gptp, const SimTime& t);
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
};

} // namespace inet

#endif

