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

#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/contract/IRadioMedium.h"
#include "inet/physicallayer/contract/IAntenna.h"
#include "inet/physicallayer/base/PhysicalLayerBase.h"
#include "inet/physicallayer/common/RadioFrame.h"

namespace inet {

namespace physicallayer {

/**
 * This class is the default implementation of the IRadio interface.
 *
 * The transmission process starts when the radio module receives a packet
 * from the higher layer. The received packet can have a TransmissionRequest
 * attached as a control info object. The radio must be in transmitter or
 * transceiver mode before receiving a packet, otherwise it throws an exception.
 * The radio changes its transmitter state to transmitting, and emits a
 * transmitter state changed signal. Finally, it schedules a timer to the end of
 * the transmission.
 *
 * The transmission process ends when the above timer expires. The radio changes
 * its transmitter state back to idle, and emits a transmitter state changed
 * signal.
 *
 * The reception process starts when the radio module receives a radio frame.
 * The radio must be in receiver or transceiver mode before the message arrives,
 * otherwise it just ignores the message. The radio changes its receiver state
 * to the appropriate value, and emits a receiver state changed signal. Finally,
 * it schedules a timer to the end of the reception.
 *
 * The reception process ends when one of the above timer expires. If the timer
 * corresponds to an attempted reception, then the radio determines the
 * reception decision. Independently of whether the reception is successful or
 * not, the encapsulated packet is sent up to the higher layer. The radio also
 * attaches a ReceptionIndication as a control info object. Finally, the radio
 * changes its receiver state to the appropriate value, and emits a receiver
 * state changed signal.
 */
// TODO: support capturing a stronger transmission
class INET_API Radio : public PhysicalLayerBase, public virtual IRadio
{
  public:
    static simsignal_t minSNIRSignal;
    static simsignal_t packetErrorRateSignal;
    static simsignal_t bitErrorRateSignal;
    static simsignal_t symbolErrorRateSignal;

  protected:
    /**
     * An identifier which is globally unique for the whole lifetime of the
     * simulation among all radios.
     */
    const int id;

    /** @name Parameters that determine the behavior of the radio. */
    //@{
    /**
     * The radio antenna model is never NULL.
     */
    const IAntenna *antenna;
    /**
     * The transmitter model is never NULL.
     */
    const ITransmitter *transmitter;
    /**
     * The receiver model is never NULL.
     */
    const IReceiver *receiver;
    /**
     * The radio medium model is never NULL.
     */
    IRadioMedium *medium;
    /**
     * Simulation time required to switch from one radio mode to another.
     */
    simtime_t switchingTimes[RADIO_MODE_SWITCHING][RADIO_MODE_SWITCHING];
    /**
     * Displays a circle around the host submodule representing the communication range.
     */
    bool displayCommunicationRange;
    /**
     * Displays a circle around the host submodule representing the interference range.
     */
    bool displayInterferenceRange;
    //@}

    /** Gates */
    //@{
    cGate *upperLayerOut;
    cGate *upperLayerIn;
    cGate *radioIn;
    //@}

    /** State */
    //@{
    /**
     * The current radio mode.
     */
    RadioMode radioMode;
    /**
     * The radio is switching to this radio radio mode if a switch is in
     * progress, otherwise this is the same as the current radio mode.
     */
    RadioMode nextRadioMode;
    /**
     * The radio is switching from this radio mode to another if a switch is
     * in progress, otherwise this is the same as the current radio mode.
     */
    RadioMode previousRadioMode;
    /**
     * The current reception state.
     */
    ReceptionState receptionState;
    /**
     * The current transmission state.
     */
    TransmissionState transmissionState;
    //@}

    /** @name Timer */
    //@{
    /**
     * The timer that is scheduled to the end of the current transmission.
     * If this timer is not scheduled then no transmission is in progress.
     */
    cMessage *endTransmissionTimer;
    /**
     * The timer that is scheduled to the end of the current reception.
     * If this timer is NULL then no attempted reception is in progress but
     * there still may be incoming receptions which are not attempted.
     */
    cMessage *endReceptionTimer;
    /**
     * The timer that is scheduled to the end of the radio mode switch.
     */
    cMessage *endSwitchTimer;
    //@}

  private:
    void parseRadioModeSwitchingTimes();
    void startRadioModeSwitch(RadioMode newRadioMode, simtime_t switchingTime);
    void completeRadioModeSwitch(RadioMode newRadioMode);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);

    virtual m computeMaxRange(W maxTransmissionPower, W minReceptionPower) const;
    virtual m computeMaxCommunicationRange() const;
    virtual m computeMaxInterferenceRange() const;

    virtual void handleMessageWhenDown(cMessage *message);
    virtual void handleMessageWhenUp(cMessage *message);
    virtual void handleSelfMessage(cMessage *message);
    virtual void handleUpperCommand(cMessage *command);
    virtual void handleLowerCommand(cMessage *command);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

    virtual void startTransmission(cPacket *macFrame);
    virtual void endTransmission();

    virtual void startReception(RadioFrame *radioFrame);
    virtual void endReception(cMessage *message);

    virtual bool isListeningPossible();
    virtual void updateTransceiverState();
    virtual void updateDisplayString();

  public:
    Radio();
    virtual ~Radio();

    virtual int getId() const { return id; }

    virtual void printToStream(std::ostream& stream) const;

    virtual const IAntenna *getAntenna() const { return antenna; }
    virtual const ITransmitter *getTransmitter() const { return transmitter; }
    virtual const IReceiver *getReceiver() const { return receiver; }
    virtual const IRadioMedium *getMedium() const { return medium; }

    virtual const cGate *getRadioGate() const { return radioIn; }

    virtual RadioMode getRadioMode() const { return radioMode; }
    virtual void setRadioMode(RadioMode newRadioMode);

    virtual ReceptionState getReceptionState() const { return receptionState; }
    virtual TransmissionState getTransmissionState() const { return transmissionState; }

    virtual const ITransmission *getTransmissionInProgress() const;
    virtual const ITransmission *getReceptionInProgress() const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RADIO_H

