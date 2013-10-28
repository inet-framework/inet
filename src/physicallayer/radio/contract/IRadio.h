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

#include "IMobility.h"
#include "IPhysicalLayer.h"

/**
 * This purely virtual interface provides an abstraction for different radios.
 * It represents a physical device that is capable of receiving and transmitting
 * radio signals. Simultaneous reception and transmission is not supported. The
 * radio has an operation mode, it's bound to a radio channel, and it provides
 * the state of the radio channel at its position.
 *
 * @author Levente Meszaros
 */
class INET_API IRadio : IPhysicalLayer
{
  public:
    /**
     * This signal is emitted every time the radio mode changes.
     * The signal value is the new radio mode.
     */
    static simsignal_t radioModeChangedSignal;

    /**
     * This signal is emitted every time the radio channel state changes.
     * The signal value is the new radio channel state.
     */
    static simsignal_t radioChannelStateChangedSignal;

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
        RADIO_MODE_OFF,         // completely turned off, power consumption is zero, slow startup
        RADIO_MODE_SLEEP,       // reception or transmission isn't possible, power consumption is minimal, quick startup
        RADIO_MODE_RECEIVER,    // only reception is possible, low power consumption
        RADIO_MODE_TRANSMITTER, // only transmission is possible, high power consumption
        // TODO: implement RADIO_MODE_TRANSCEIVER, // reception and transmission is possible simultaneously, high power consumption
        RADIO_MODE_SWITCHING    // switching from one mode to another
    };

    /**
     * This enumeration specifies the state of the radio channel at this radio.
     */
    enum RadioChannelState
    {
        RADIO_CHANNEL_STATE_FREE,         // no signal is detected, the radio channel is free
        RADIO_CHANNEL_STATE_BUSY,         // some signal is detected but not strong enough to receive, the radio channel is busy
        RADIO_CHANNEL_STATE_RECEIVING,    // a signal strong enough to receive is detected, the radio channel is busy
        RADIO_CHANNEL_STATE_TRANSMITTING, // no signal detected due to ongoing transmission, the radio channel is busy
        RADIO_CHANNEL_STATE_UNKNOWN,      // signal detection is not possible, the radio channel state is unknown
    };

    /**
     * The enumeration registered for radio mode.
     */
    static cEnum *radioModeEnum;

    /**
     * The enumeration registered for radio channel state.
     */
    static cEnum *radioChannelStateEnum;

  public:
    virtual ~IRadio() { }

    /**
     * Returns the position of the radio.
     */
    virtual IMobility *getMobility() const = 0;

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
     * The new radio mode will be published with emitting a radioModeChangedSignal.
     */
    virtual void setRadioMode(RadioMode radioMode) = 0;

    /**
     * Returns the current radio channel state. This is the same state as the one emitted
     * with the last radioChannelStateChangedSignal
     */
    virtual RadioChannelState getRadioChannelState() const = 0;

    /**
     * Returns the current radio channel. This is the same channel as the one emitted
     * with the last radioChannelChangedSignal.
     */
    virtual int getRadioChannel() const = 0;
    /**
     * Changes the current radio channel. The actual change may take zero or more time.
     * The new radio channel will be published with emitting a radioChannelChangedSignal.
     */
    virtual void setRadioChannel(int radioChannel) = 0;

  public:
    /**
     * Returns the name of the provided radio mode.
     */
    static const char *getRadioModeName(RadioMode radioMode);

    /**
     * Returns the name of the provided radio channel.
     */
    static const char *getRadioChannelStateName(RadioChannelState radioChannelState);
};

#endif
