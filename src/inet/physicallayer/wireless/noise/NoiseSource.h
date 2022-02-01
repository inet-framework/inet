//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NOISESOURCE_H
#define __INET_NOISESOURCE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntenna.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {
namespace physicallayer {

class INET_API NoiseSource : public cSimpleModule, public virtual IRadio
{
  protected:
    const int id = nextId++;
    cGate *radioIn = nullptr;
    int mediumModuleId = -1;
    ModuleRefByPar<IRadioMedium> medium;
    const IAntenna *antenna = nullptr;
    const ITransmitter *transmitter = nullptr;

    cMessage *transmissionTimer = nullptr;
    cMessage *sleepTimer = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void startTransmission();
    virtual void endTransmission();

    virtual void scheduleSleepTimer();
    virtual void scheduleTransmissionTimer(const ITransmission *transmission);

  public:
    virtual ~NoiseSource();

    virtual const cGate *getRadioGate() const override { return radioIn; }

    virtual RadioMode getRadioMode() const override { return RADIO_MODE_TRANSMITTER; }
    virtual void setRadioMode(RadioMode radioMode) override { throw cRuntimeError("Invalid operation"); }

    virtual ReceptionState getReceptionState() const override { return RECEPTION_STATE_UNDEFINED; }
    virtual TransmissionState getTransmissionState() const override { return TRANSMISSION_STATE_TRANSMITTING; }

    virtual int getId() const override { return id; } // TODO

    virtual const IAntenna *getAntenna() const override { return antenna; }
    virtual const ITransmitter *getTransmitter() const override { return transmitter; }
    virtual const IReceiver *getReceiver() const override { return nullptr; }
    virtual const IRadioMedium *getMedium() const override { return medium; }

    virtual const ITransmission *getTransmissionInProgress() const override;
    virtual const ITransmission *getReceptionInProgress() const override { return nullptr; }

    virtual IRadioSignal::SignalPart getTransmittedSignalPart() const override { return IRadioSignal::SIGNAL_PART_WHOLE; }
    virtual IRadioSignal::SignalPart getReceivedSignalPart() const override { return IRadioSignal::SIGNAL_PART_NONE; }
};

} // namespace physicallayer
} // namespace inet

#endif

