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

#ifndef __INET_SHORTCUTRADIO_H
#define __INET_SHORTCUTRADIO_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/physicallayer/base/packetlevel/PhysicalLayerBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

// TODO: emit signals required by IRadio interface
class INET_API ShortcutRadio : public PhysicalLayerBase, public virtual IRadio
{
  protected:
    static std::map<MacAddress, ShortcutRadio *> shortcutRadios;

  protected:
    RadioMode radioMode = RADIO_MODE_OFF;
    double bitrate = NaN;
    cPar *lengthOverhead = nullptr;
    cPar *durationOverhead = nullptr;
    cPar *propagationDelay = nullptr;
    cPar *packetLoss = nullptr;
    TransmissionState transmissionState = TRANSMISSION_STATE_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

    virtual ShortcutRadio *findPeer(MacAddress address);
    virtual void sendToPeer(Packet *packet, ShortcutRadio *peer);
    virtual void receiveFromPeer(Packet *packet);

  public:
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleUpperPacket(Packet *packet) override;

    virtual const cGate *getRadioGate() const override { return gate("radioIn"); }
    virtual RadioMode getRadioMode() const override { return radioMode; }
    virtual void setRadioMode(RadioMode radioMode) override { this->radioMode = radioMode; }
    virtual ReceptionState getReceptionState() const override { return RECEPTION_STATE_UNDEFINED; }
    virtual TransmissionState getTransmissionState() const override { return transmissionState; }
    virtual int getId() const override { return -1; }
    virtual const IAntenna *getAntenna() const override { return nullptr; }
    virtual const ITransmitter *getTransmitter() const override { return nullptr; }
    virtual const IReceiver *getReceiver() const override { return nullptr; }
    virtual const IRadioMedium *getMedium() const override { return nullptr; }
    virtual const ITransmission *getTransmissionInProgress() const override { return nullptr; }
    virtual const ITransmission *getReceptionInProgress() const override { return nullptr; }
    virtual IRadioSignal::SignalPart getTransmittedSignalPart() const override { return IRadioSignal::SIGNAL_PART_WHOLE; }
    virtual IRadioSignal::SignalPart getReceivedSignalPart() const override { return IRadioSignal::SIGNAL_PART_WHOLE; }

    // OperationalBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleStopOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}    //TODO implementation
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SHORTCUTRADIO_H

