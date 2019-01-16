//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_RADIO_H
#define __INET_RADIO_H

#include "inet/physicallayer/base/packetlevel/PhysicalLayerBase.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/physicallayer/contract/packetlevel/IAntenna.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {
namespace physicallayer {

/**
 * This class is the default implementation of the IRadio interface.
 *
 * The transmission process starts when the radio module receives a packet
 * from the higher layer. The radio must be in transmitter or transceiver mode
 * before receiving a packet, otherwise it throws an exception. The radio changes
 * its transmitter state to transmitting, and emits a transmitter state changed
 * signal. Finally, it schedules a timer to the end of the transmission.
 *
 * The transmission process ends when the above timer expires. The radio changes
 * its transmitter state back to idle, and emits a transmitter state changed
 * signal.
 *
 * The reception process starts when the radio module receives a packet.
 * The radio must be in receiver or transceiver mode before the packet arrives,
 * otherwise it just ignores the packet. The radio changes its receiver state
 * to the appropriate value, and emits a receiver state changed signal. Finally,
 * it schedules a timer to the end of the reception.
 *
 * The reception process ends when one of the above timer expires. If the timer
 * corresponds to an attempted reception, then the radio determines the
 * reception decision. Independently of whether the reception is successful or
 * not, the encapsulated packet is sent up to the higher layer. Finally, the radio
 * changes its receiver state to the appropriate value, and emits a receiver
 * state changed signal.
 */
// TODO: support capturing a stronger transmission
class INET_API Radio : public PhysicalLayerBase, public virtual IRadio
{
  protected:
    /**
     * An identifier which is globally unique for the whole lifetime of the
     * simulation among all radios.
     */
    const int id = nextId++;

    /** @name Parameters that determine the behavior of the radio. */
    //@{
    /**
     * The radio antenna model is never nullptr.
     */
    const IAntenna *antenna = nullptr;
    /**
     * The transmitter model is never nullptr.
     */
    const ITransmitter *transmitter = nullptr;
    /**
     * The receiver model is never nullptr.
     */
    const IReceiver *receiver = nullptr;
    /**
     * The radio medium model is never nullptr.
     */
    IRadioMedium *medium = nullptr;
    /**
     * The module id of the medim model.
     */
    int mediumModuleId = -1;
    /**
     * Simulation time required to switch from one radio mode to another.
     */
    simtime_t switchingTimes[RADIO_MODE_SWITCHING][RADIO_MODE_SWITCHING];
    /**
     * When true packets are serialized into a sequence of bytes before sending out.
     */
    bool sendRawBytes = false;
    /**
     * Determines whether the transmission of the preamble, header and data part
     * are simulated separately or not.
     */
    bool separateTransmissionParts = false;
    /**
     * Determines whether the reception of the preamble, header and data part
     * are simulated separately or not.
     */
    bool separateReceptionParts = false;
    //@}

    /** Gates */
    //@{
    cGate *upperLayerOut = nullptr;
    cGate *upperLayerIn = nullptr;
    cGate *radioIn = nullptr;
    //@}

    /** State */
    //@{
    /**
     * The current radio mode.
     */
    RadioMode radioMode = RADIO_MODE_OFF;
    /**
     * The radio is switching to this radio radio mode if a switch is in
     * progress, otherwise this is the same as the current radio mode.
     */
    RadioMode nextRadioMode = RADIO_MODE_OFF;
    /**
     * The radio is switching from this radio mode to another if a switch is
     * in progress, otherwise this is the same as the current radio mode.
     */
    RadioMode previousRadioMode = RADIO_MODE_OFF;
    /**
     * The current reception state.
     */
    ReceptionState receptionState = RECEPTION_STATE_UNDEFINED;
    /**
     * The current transmission state.
     */
    TransmissionState transmissionState = TRANSMISSION_STATE_UNDEFINED;
    /**
     * The current received signal part.
     */
    IRadioSignal::SignalPart receivedSignalPart = IRadioSignal::SIGNAL_PART_NONE;
    /**
     * The current transmitted signal part.
     */
    IRadioSignal::SignalPart transmittedSignalPart = IRadioSignal::SIGNAL_PART_NONE;
    //@}

    /** @name Timer */
    //@{
    /**
     * The timer that is scheduled to the end of the current transmission.
     * If this timer is not scheduled then no transmission is in progress.
     */
    cMessage *transmissionTimer = nullptr;
    /**
     * The timer that is scheduled to the end of the current reception.
     * If this timer is nullptr then no attempted reception is in progress but
     * there still may be incoming receptions which are not attempted.
     */
    cMessage *receptionTimer = nullptr;
    /**
     * The timer that is scheduled to the end of the radio mode switch.
     */
    cMessage *switchTimer = nullptr;
    //@}

  private:
    void parseRadioModeSwitchingTimes();
    void startRadioModeSwitch(RadioMode newRadioMode, simtime_t switchingTime);
    void completeRadioModeSwitch(RadioMode newRadioMode);

  protected:
    virtual void initialize(int stage) override;
    virtual void initializeRadioMode();

    virtual void handleMessageWhenDown(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message) override;
    virtual void handleSwitchTimer(cMessage *message);
    virtual void handleTransmissionTimer(cMessage *message);
    virtual void handleReceptionTimer(cMessage *message);
    virtual void handleUpperCommand(cMessage *command) override;
    virtual void handleLowerCommand(cMessage *command) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleSignal(Signal *signal) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void startTransmission(Packet *macFrame, IRadioSignal::SignalPart part);
    virtual void continueTransmission();
    virtual void endTransmission();
    virtual void abortTransmission();

    virtual Signal *createSignal(Packet *packet) const;

    virtual void startReception(cMessage *timer, IRadioSignal::SignalPart part);
    virtual void continueReception(cMessage *timer);
    virtual void endReception(cMessage *timer);
    virtual void abortReception(cMessage *timer);
    virtual void captureReception(cMessage *timer);

    virtual void sendUp(Packet *macFrame);
    virtual cMessage *createReceptionTimer(Signal *signal) const;
    virtual bool isReceptionTimer(const cMessage *message) const;

    virtual bool isReceiverMode(IRadio::RadioMode radioMode) const;
    virtual bool isTransmitterMode(IRadio::RadioMode radioMode) const;
    virtual bool isListeningPossible() const;

    virtual void updateTransceiverState();
    virtual void updateTransceiverPart();

  public:
    Radio() { }
    virtual ~Radio();

    virtual int getId() const override { return id; }

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IAntenna *getAntenna() const override { return antenna; }
    virtual const ITransmitter *getTransmitter() const override { return transmitter; }
    virtual const IReceiver *getReceiver() const override { return receiver; }
    virtual const IRadioMedium *getMedium() const override { return medium; }

    virtual const cGate *getRadioGate() const override { return radioIn; }

    virtual RadioMode getRadioMode() const override { return radioMode; }
    virtual void setRadioMode(RadioMode newRadioMode) override;

    virtual ReceptionState getReceptionState() const override { return receptionState; }
    virtual TransmissionState getTransmissionState() const override { return transmissionState; }

    virtual const ITransmission *getTransmissionInProgress() const override;
    virtual const ITransmission *getReceptionInProgress() const override;

    virtual IRadioSignal::SignalPart getTransmittedSignalPart() const override;
    virtual IRadioSignal::SignalPart getReceivedSignalPart() const override;

    virtual void encapsulate(Packet *packet) const { }
    virtual void decapsulate(Packet *packet) const { }
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_RADIO_H

