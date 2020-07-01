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

#ifndef __INET_IRADIO_H
#define __INET_IRADIO_H

#include "inet/physicallayer/contract/packetlevel/IAntenna.h"
#include "inet/physicallayer/contract/packetlevel/IPhysicalLayer.h"
#include "inet/physicallayer/contract/packetlevel/IReceiver.h"
#include "IWirelessSignal.h"
#include "inet/physicallayer/contract/packetlevel/ITransmitter.h"

namespace inet {
namespace physicallayer {

class IRadioMedium;

/**
 * This interface represents a physical device that is capable of transmitting
 * and receiving radio signals. Simultaneous reception and transmission is also
 * supported. The radio has an operation mode and it provides the state of the
 * radio medium at its position.
 *
 * @author Levente Meszaros
 */
// TODO: add burst support, sending of signals back to back (using a resource limited queue)
// TODO: rename *Changed signals to *Change signals and emit them just before overwriting
//       the current state, and thus allowing listeners to use the current value too
class INET_API IRadio : public IPhysicalLayer, public IPrintableObject
{
  protected:
    static int nextId;

  public:
    /**
     * This signal is emitted when the radio mode of the radio is changed.
     * The source is the radio and the emitted value is the new radio mode.
     */
    static simsignal_t radioModeChangedSignal;

    /**
     * This signal is emitted when the radio listening of the radio is changed.
     * The source is the radio and the emitted value is the new listening.
     */
    static simsignal_t listeningChangedSignal;

    /**
     * This signal is emitted when the radio reception state of the radio is changed.
     * The source is the radio and the emitted value is the new radio reception state.
     */
    static simsignal_t receptionStateChangedSignal;

    /**
     * This signal is emitted when the radio transmission state of the radio is changed.
     * The source is the radio and the emitted value is the new radio transmission state.
     */
    static simsignal_t transmissionStateChangedSignal;

    /**
     * This signal is emitted when the received part is changed by the radio.
     * The source is the radio and the emitted value is the new received part.
     */
    static simsignal_t receivedSignalPartChangedSignal;

    /**
     * This signal is emitted when the transmitted part is changed by the radio.
     * The source is the radio and the emitted value is the new transmitted part.
     */
    static simsignal_t transmittedSignalPartChangedSignal;

    /**
     * This enumeration specifies the requested operational mode of the radio.
     * NOTE: Some parts of the code base may be sensitive to the order of the
     * enum items because they may be used as an array index.
     */
    enum RadioMode {
        /**
         * The radio is turned off, frame reception or transmission is not
         * possible, power consumption is zero, radio mode switching is slow.
         */
        RADIO_MODE_OFF,

        /**
         * The radio is sleeping, frame reception or transmission is not possible,
         * power consumption is minimal, radio mode switching is fast.
         */
        RADIO_MODE_SLEEP,

        /**
         * The radio is prepared for frame reception, frame transmission is not
         * possible, power consumption is low when receiver is idle and medium
         * when receiving.
         */
        RADIO_MODE_RECEIVER,

        /**
         * The radio is prepared for frame transmission, frame reception is not
         * possible, power consumption is low when transmitter is idle and high
         * when transmitting.
         */
        RADIO_MODE_TRANSMITTER,

        /**
         * The radio is prepared for simultaneous frame reception and transmission,
         * power consumption is low when transceiver is idle, medium when receiving
         * and high when transmitting.
         */
        RADIO_MODE_TRANSCEIVER,

        /**
         * The radio is switching from one mode to another, frame reception or
         * transmission is not possible, power consumption is minimal.
         */
        RADIO_MODE_SWITCHING    // this radio mode must be the very last
    };

    /**
     * This enumeration specifies the reception state of the radio. This also
     * determines the state of the radio medium.
     */
    enum ReceptionState {
        /**
         * The radio medium state is unknown, reception state is meaningless,
         * signal detection is not possible. (e.g. the radio mode is off, sleep
         * or transmitter)
         */
        RECEPTION_STATE_UNDEFINED,

        /**
         * The radio medium is free, no signal is detected. (e.g. the RSSI is
         * below the energy detection threshold)
         */
        RECEPTION_STATE_IDLE,

        /**
         * The radio medium is busy, a signal is detected but it is not strong
         * enough to receive. (e.g. the RSSI is above the energy detection
         * threshold but below the reception threshold)
         */
        RECEPTION_STATE_BUSY,

        /**
         * The radio medium is busy, a signal strong enough to receive is detected.
         * (e.g. the SNIR was above the reception threshold)
         */
        RECEPTION_STATE_RECEIVING
    };

    /**
     * This enumeration specifies the transmission state of the radio.
     */
    enum TransmissionState {
        /**
         * The transmission state is undefined or meaningless. (e.g. the radio
         * mode is off, sleep or receiver)
         */
        TRANSMISSION_STATE_UNDEFINED,

        /**
         * The radio is not transmitting a signal on the radio medium. (e.g. the
         * last transmission has been completed)
         */
        TRANSMISSION_STATE_IDLE,

        /**
         * The radio medium is busy, the radio is currently transmitting a signal.
         */
        TRANSMISSION_STATE_TRANSMITTING
    };

  protected:
    /**
     * The enumeration registered for radio mode.
     */
    static cEnum *radioModeEnum;

    /**
     * The enumeration registered for radio reception state.
     */
    static cEnum *receptionStateEnum;

    /**
     * The enumeration registered for radio transmission state.
     */
    static cEnum *transmissionStateEnum;

  public:
    /**
     * Returns the gate of the radio that receives incoming signals.
     */
    virtual const cGate *getRadioGate() const = 0;

    /**
     * Returns the current radio mode, This is the same mode as the one emitted
     * with the last radioModeChangedSignal.
     */
    virtual RadioMode getRadioMode() const = 0;

    /**
     * Changes the current radio mode. The actual change may take zero or more time.
     * The new radio mode will be emitted with a radioModeChangedSignal.
     */
    virtual void setRadioMode(RadioMode radioMode) = 0;

    /**
     * Returns the current radio reception state. This is the same state as the
     * one emitted with the last receptionStateChangedSignal.
     */
    virtual ReceptionState getReceptionState() const = 0;

    /**
     * Returns the current radio transmission state. This is the same state as
     * the one emitted with the last transmissionStateChangedSignal.
     */
    virtual TransmissionState getTransmissionState() const = 0;

    /**
     * Returns an identifier for this radio which is globally unique
     * for the whole lifetime of the simulation among all radios.
     */
    virtual int getId() const = 0;

    /**
     * Returns the antenna used by the transceiver of this radio. This function
     * never returns nullptr.
     */
    virtual const IAntenna *getAntenna() const = 0;

    /**
     * Returns the transmitter part of this radio. This function never returns nullptr.
     */
    virtual const ITransmitter *getTransmitter() const = 0;

    /**
     * Returns the receiver part of this radio. This function never returns nullptr.
     */
    virtual const IReceiver *getReceiver() const = 0;

    /**
     * Returns the radio medium where this radio is transmitting and receiving
     * radio signals. This function never returns nullptr.
     */
    virtual const IRadioMedium *getMedium() const = 0;

    /**
     * Returns the ongoing transmission that the transmitter is currently
     * transmitting or nullptr.
     */
    virtual const ITransmission *getTransmissionInProgress() const = 0;

    /**
     * Returns the ongoing reception that the receiver is currently receiving
     * or nullptr.
     */
    virtual const ITransmission *getReceptionInProgress() const = 0;

    /**
     * Returns the signal part of the ongoing transmission that the transmitter
     * is currently transmitting or -1 if no transmission is in progress. This
     * is the same part as the one emitted with the last transmittedPartChangedSignal.
     */
    virtual IRadioSignal::SignalPart getTransmittedSignalPart() const = 0;

    /**
     * Returns the signal part of the ongoing reception that the receiver is
     * currently receiving or -1 if no reception is in progress. This is the
     * same part as the one emitted with the last receivedPartChangedSignal.
     */
    virtual IRadioSignal::SignalPart getReceivedSignalPart() const = 0;

  public:
    /**
     * Returns the name of the provided radio mode.
     */
    static const char *getRadioModeName(RadioMode radioMode);

    /**
     * Returns the name of the provided radio reception state.
     */
    static const char *getRadioReceptionStateName(ReceptionState receptionState);

    /**
     * Returns the name of the provided radio transmission state.
     */
    static const char *getRadioTransmissionStateName(TransmissionState transmissionState);
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_IRADIO_H

