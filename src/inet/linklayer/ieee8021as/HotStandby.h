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
#include "inet/clock/model/MultiClock.h"

namespace inet {

class INET_API HotStandby : public ClockUserModuleBase, public cListener
{
  protected:
    std::map<int, SyncState> syncStates; // store each gptp sync time
    MultiClock *multiClock;

  protected:
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t simSignal, intval_t t, cObject *details) override;
    void handleSyncStateChanged(const Gptp *gptp, SyncState syncState);
    virtual int numInitStages() const override { return NUM_INIT_STAGES; };
};

} // namespace inet

#endif
