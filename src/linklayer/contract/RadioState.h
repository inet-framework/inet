/* -*- mode:c++ -*- ********************************************************
 * file:        RadioState.h
 *
 * author:      Andreas Koepke
 *              modifications Andras Varga, 2006
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#ifndef RADIOSTATE_H
#define RADIOSTATE_H

#include <omnetpp.h>
#include "INETDefs.h"

/**
 * Holds the current state and other properties of the radio.
 * Possible states are:
 *
 *  - IDLE: channel is empty (radio is in receive mode)
 *  - RECV: channel is busy (radio is in receive mode)
 *  - TRANSMIT: the radio is transmitting
 *  - SLEEP: the radio is sleeping
 *
 * @author Andreas Köpke, Andras Varga
 * @see NotificationBoard
 */
class INET_API RadioState : public cPolymorphic
{
  public:
    /** Possible states of the radio */
    enum State
    {
      IDLE,
      RECV,
      TRANSMIT,
      SLEEP
    };

    //XXX consider adding the following:
    //Q: how to notify a TRANSMIT->RECV transition? as TRANSMIT_END, RECV_START, both, or some 3rd one?
    // enum Transition
    // {
    //   TRANSMIT_START, ///< start of a transmission (non-TRANSMIT to TRANSMIT)
    //   TRANSMIT_END,   ///< TRANSMIT to non-TRANSMIT (note: new state is in "state" field)
    //   RECV_START,     ///< IDLE to RECV
    //   RECV_END_OK,    ///< reception ends, frame received OK
    //   RECV_END_ERROR  ///< reception ends, received frame has bit errors
    // };

  private:
    int radioId; // identifies the radio
    State state; // IDLE, RECV, TRANSMIT, SLEEP
    int channelNumber; // the radio channel
    double bitrate; // the current transmit bitrate

  public:
    /** Constructor */
    RadioState(int radioModuleId) : cPolymorphic() {
        radioId = radioModuleId; state = IDLE; channelNumber = -1; bitrate = -1;
    }

    /** id of the radio/snrEval module -- identifies the radio in case there're more than one in the host */
    int getRadioId() const { return radioId; }

	void setRadioId(int state) { radioId = state; } //AM 6 Dezember nachträglich eingefügt

    /** Returns radio state */
    State getState() const { return state; }

    /** Sets the radio state */
    void setState(State s) { state = s; }

    /** function to get the channel number (frequency) */
    int getChannelNumber() const { return channelNumber; }

    /** set the channel number (frequency) */
    void setChannelNumber(int chan) { channelNumber = chan; }

    /** function to get the bitrate */
    double getBitrate() const { return bitrate; }

    /** set the bitrate */
    void setBitrate(double d) { bitrate = d; }

    /** Returns the name of the radio state in a readable form */
    static const char *stateName(State state) {
        switch(state) {
            case IDLE: return "IDLE";
            case RECV: return "RECV";
            case TRANSMIT: return "TRANSMIT";
            case SLEEP: return "SLEEP";
            default: return "???";
        }
    }

    /** Enables inspection */
    std::string info() const {
        std::stringstream out;
        out << stateName(state) << ", channel #" << channelNumber << ", " << (bitrate/1e6) << "Mbps ";
        return out.str();
    }

};


inline std::ostream& operator<<(std::ostream& os, const RadioState& r)
{
    return os << r.info();
}

#endif
