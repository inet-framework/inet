/* -*- mode:c++ -*- ********************************************************
 * file:        RadioState.h
 *
 * author:      Andreas Koepke
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
 * @brief Class to hold the radio state of the host
 *
 * Holds the actual state of the radio. Possible states
 * are : IDLE, RECV, TRANSMIT and SLEEP
 *
 * IDLE: channel is empty (radio is in receive mode)
 *
 * RECV: channel is busy (radio is in receive mode)
 *
 * TRANSMIT: the radio is transmitting
 *
 * SLEEP: the radio is sleeping
 *
 * @ingroup utils
 * @author Andreas Köpke
 * @sa NotificationBoard
 */
class INET_API RadioState : public cPolymorphic
{
public:
    /** @brief possible states of the radio*/
    enum State
    {
      IDLE,
      RECV,
      TRANSMIT,
      SLEEP
    };

private:
    /** @brief Identifies the radio */
    int radioId;

    /** @brief Variable that hold the actual state*/
    State state;

    /** @brief The radio channel */
    int channel;

public:
    /** @brief id of the SnrEval module -- identifies the radio in case there're more than one in the host */
    int getRadioId() const { return radioId; }

    /** @brief function to get the state*/
    State getState() const { return state; }

    /** @brief set the state of the radio*/
    void setState(State s) { state = s; }

    /** @brief function to get the channel number (frequency) */
    int getChannel() const { return channel; }

    /** @brief set the channel number (frequency) */
    void setChannel(int chan) { channel = chan; }

    /** @brief Constructor */
    RadioState(int radioModuleId) : cPolymorphic() {radioId=radioModuleId; state=IDLE; channel=-1;}

    static const char *stateName(State state) {
        switch(state) {
            case IDLE: return "IDLE";
            case RECV: return "RECV";
            case TRANSMIT: return "TRANSMIT";
            case SLEEP: return "SLEEP";
            default: return "???";
        }
    }

    /** @brief Enables inspection */
    std::string info() const {
        std::stringstream out;
        out << "channel=" << channel << " " << stateName(state);
        return out.str();
    }

};


inline std::ostream& operator<<(std::ostream& os, const RadioState& r)
{
    return os << r.info();
}

#endif
