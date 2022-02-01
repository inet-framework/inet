//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATEBASEDCCENERGYCONSUMER_H
#define __INET_STATEBASEDCCENERGYCONSUMER_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/power/contract/ICcEnergyConsumer.h"
#include "inet/power/contract/ICcEnergySource.h"

namespace inet {

namespace physicallayer {

/**
 * This is a simple radio energy consumer model. The current consumption is
 * determined by the radio mode, the transmitter state and the receiver state
 * using constant parameters.
 *
 */
class INET_API StateBasedCcEnergyConsumer : public cSimpleModule, public virtual power::ICcEnergyConsumer, public cListener
{
  protected:
    // parameters
    V minSupplyVoltage = V(NaN);
    V maxSupplyVoltage = V(NaN);
    A offCurrentConsumption = A(NaN);
    A sleepCurrentConsumption = A(NaN);
    A switchingCurrentConsumption = A(NaN);
    A receiverIdleCurrentConsumption = A(NaN);
    A receiverBusyCurrentConsumption = A(NaN);
    A receiverReceivingCurrentConsumption = A(NaN);
    A receiverReceivingPreambleCurrentConsumption = A(NaN);
    A receiverReceivingHeaderCurrentConsumption = A(NaN);
    A receiverReceivingDataCurrentConsumption = A(NaN);
    A transmitterIdleCurrentConsumption = A(NaN);
    A transmitterTransmittingCurrentConsumption = A(NaN);
    A transmitterTransmittingPreambleCurrentConsumption = A(NaN);
    A transmitterTransmittingHeaderCurrentConsumption = A(NaN);
    A transmitterTransmittingDataCurrentConsumption = A(NaN);

    // environment
    IRadio *radio = nullptr;
    power::ICcEnergySource *energySource = nullptr;

    // state
    A currentConsumption = A(NaN);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual A computeCurrentConsumption() const;

  public:
    virtual power::IEnergySource *getEnergySource() const override { return energySource; }
    virtual A getCurrentConsumption() const override { return currentConsumption; }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace physicallayer

} // namespace inet

#endif

