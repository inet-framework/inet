//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ALTERNATINGEPENERGYCONSUMER_H
#define __INET_ALTERNATINGEPENERGYCONSUMER_H

#include "inet/power/contract/IEpEnergyConsumer.h"
#include "inet/power/contract/IEpEnergySource.h"

namespace inet {

namespace power {

/**
 * This class implements a simple power based alternating energy consumer.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API AlternatingEpEnergyConsumer : public cSimpleModule, public IEpEnergyConsumer
{
  protected:
    // parameters
    IEpEnergySource *energySource = nullptr;
    cMessage *timer = nullptr;

    // state
    bool isSleeping = false;
    W powerConsumption = W(NaN);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void updatePowerConsumption();
    virtual void scheduleIntervalTimer();

  public:
    virtual ~AlternatingEpEnergyConsumer();

    virtual IEnergySource *getEnergySource() const override { return energySource; }
    virtual W getPowerConsumption() const override { return powerConsumption; }
};

} // namespace power

} // namespace inet

#endif

