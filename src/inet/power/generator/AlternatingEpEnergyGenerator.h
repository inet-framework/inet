//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ALTERNATINGEPENERGYGENERATOR_H
#define __INET_ALTERNATINGEPENERGYGENERATOR_H

#include "inet/power/contract/IEpEnergyGenerator.h"
#include "inet/power/contract/IEpEnergySink.h"

namespace inet {

namespace power {

/**
 * This class implements a simple power based alternating energy generator.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API AlternatingEpEnergyGenerator : public cSimpleModule, public IEpEnergyGenerator
{
  protected:
    // parameters
    IEpEnergySink *energySink = nullptr;
    cMessage *timer = nullptr;

    // state
    bool isSleeping = false;
    W powerGeneration = W(NaN);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void updatePowerGeneration();
    virtual void scheduleIntervalTimer();

  public:
    virtual ~AlternatingEpEnergyGenerator();

    virtual IEnergySink *getEnergySink() const override { return energySink; }
    virtual W getPowerGeneration() const override { return powerGeneration; }
};

} // namespace power

} // namespace inet

#endif

