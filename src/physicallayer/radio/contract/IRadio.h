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

#ifndef __INET_IRADIO_H_
#define __INET_IRADIO_H_

#include "IPhysicalLayer.h"
#include "IRadioFrame.h"
#include "IRadioSignalSource.h"
#include "IRadioAntenna.h"
#include "IRadioSignalReceiver.h"
#include "IRadioSignalTransmitter.h"

class IRadioChannel;

/**
 * This purely virtual interface provides an abstraction for different radios.
 * This interface represents a physical device that is capable of transmitting
 * and receiving radio signals. Simultaneous reception and transmission is also
 * supported. The radio has an operation mode, it's bound to a radio channel,
 * and it provides the state of the radio channel at its position.
 *
 * @author Levente Meszaros
 */
// TODO: rename *Changed signals to *Change signals and emit them just before overwriting
//       the current state, and thus allowing listeners to use the current value
// TODO: make enums global or remove prefix or abbreviate prefix or maybe keep as it is?
class INET_API IRadio : public IRadioSignalSource, public IPhysicalLayer
{
    public:
        /**
         * This signal is emitted every time the radio mode changes.
         * The signal value is the new radio mode.
         */
        static simsignal_t radioModeChangedSignal;

        /**
         * This signal is emitted every time the radio listening changes.
         * The signal value is the new listening.
         */
        static simsignal_t listeningChangedSignal;

        /**
         * This signal is emitted every time the radio reception state changes.
         * The signal value is the new radio reception state.
         */
        static simsignal_t receptionStateChangedSignal;

        /**
         * This signal is emitted every time the radio transmission state changes.
         * The signal value is the new radio transmission state.
         */
        static simsignal_t transmissionStateChangedSignal;

        /**
         * This signal is emitted every time the radio channel changes.
         * The signal value is the new radio channel.
         */
        static simsignal_t radioChannelChangedSignal;

        /**
         * This enumeration specifies the requested operational mode of the radio.
         */
        enum RadioMode
        {
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
            RADIO_MODE_SWITCHING
        };

        /**
         * This enumeration specifies the reception state of the radio. This also
         * determines the state of the radio channel.
         */
        enum ReceptionState
        {
            /**
             * The radio channel state is unknown, reception state is meaningless,
             * signal detection is not possible. (e.g. the radio mode is off, sleep
             * or transmitter)
             */
            RECEPTION_STATE_UNDEFINED,

            /**
             * The radio channel is free, no signal is detected. (e.g. the RSSI is
             * below the energy detection threshold)
             */
            RECEPTION_STATE_IDLE,

            /**
             * The radio channel is busy, a signal is detected but it is not strong
             * enough to receive. (e.g. the RSSI is above the energy detection
             * threshold but below the reception threshold)
             */
            RECEPTION_STATE_BUSY,

            /**
             * The radio channel is busy, a signal strong enough to evaluate is detected,
             * whether the signal is noise or not is not yet decided. (e.g. the RSSI is
             * above the reception threshold but the SNIR is not yet evaluated)
             */
            RECEPTION_STATE_SYNCHRONIZING,

            /**
             * The radio channel is busy, a signal strong enough to receive is detected.
             * (e.g. the SNIR was above the reception threshold during synchronize)
             */
            RECEPTION_STATE_RECEIVING
        };

        /**
         * This enumeration specifies the transmission state of the radio.
         */
        enum TransmissionState
        {
            /**
             * The transmission state is undefined or meaningless. (e.g. the radio
             * mode is off, sleep or receiver)
             */
            TRANSMISSION_STATE_UNDEFINED,

            /**
             * The radio is not transmitting a signal on the radio channel. (e.g. the
             * last transmission has been completed)
             */
            TRANSMISSION_STATE_IDLE,

            /**
             * The radio channel is busy, the radio is currently transmitting a signal.
             */
            TRANSMISSION_STATE_TRANSMITTING,
        };

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
        virtual ~IRadio() { }

        /**
         * Returns the gate of the radio that receives incoming radio frames.
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
         * Returns the current radio reception state. This is the same state as the one emitted
         * with the last receptionStateChangedSignal
         */
        virtual ReceptionState getReceptionState() const = 0;

        /**
         * Returns the current radio transmission state. This is the same state as the one emitted
         * with the last transmissionStateChangedSignal
         */
        virtual TransmissionState getTransmissionState() const = 0;

        /**
         * Returns the current radio channel. This is the same channel as the one emitted
         * with the last radioChannelChangedSignal.
         */
        // TODO: obsolete
        virtual int getOldRadioChannel() const = 0;

        /**
         * Changes the current radio channel. The actual change may take zero or more time.
         * The new radio channel will be published with emitting a radioChannelChangedSignal.
         */
        // TODO: obsolete
        virtual void setOldRadioChannel(int radioChannel) = 0;

        virtual int getId() const = 0;

        virtual const IRadioAntenna *getAntenna() const = 0;
        virtual const IRadioSignalTransmitter *getTransmitter() const = 0;
        virtual const IRadioSignalReceiver *getReceiver() const = 0;
        virtual const IRadioChannel *getChannel() const = 0;

        virtual const IRadioSignalTransmission *getTransmissionInProgress() const = 0;
        virtual const IRadioSignalTransmission *getReceptionInProgress() const = 0;

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

#endif
