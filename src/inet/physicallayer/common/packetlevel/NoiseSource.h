//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_NOISESOURCE_H
#define __INET_NOISESOURCE_H

#include "inet/physicallayer/contract/packetlevel/IAntenna.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {
namespace physicallayer {

class INET_API NoiseSource : public cSimpleModule, public virtual IRadio
{
  protected:
    const int id = nextId++;
    cGate *radioIn = nullptr;
    int mediumModuleId = -1;
    IRadioMedium *medium = nullptr;
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

    virtual int getId() const override { return id; } // TODO:

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

#endif // ifndef __INET_NOISESOURCE_H

