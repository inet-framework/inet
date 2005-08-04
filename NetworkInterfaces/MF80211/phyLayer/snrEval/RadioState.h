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
 * BUSY: channel is busy (radio is in receive mode)
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
    enum States
    {
      IDLE,
      RECV,
      TRANSMIT,
      SLEEP
    };

private:
    /** @brief Variable that hold the actual state*/
    States state;

    /** @brief Identifies the radio channel */
    int channelId;   // FIXME use channel!

public:
    /** @brief function to get the state*/
    States getState() const { return state; }
    /** @brief set the state of the radio*/
    void setState(States s) { state = s; }

    /** @brief function to get the channel */
    int getChannelId() const { return channelId; }
    /** @brief set the radio channel */
    void setChannelId(int chan) { channelId = chan; }

    /** @brief Constructor*/
    RadioState(States s=IDLE) : cPolymorphic(), state(s), channelId(-1) {};

    /** @brief Enables inspection */
    std::string info() const {
        // FIXME add channel
        switch(state) {
            case IDLE: return "IDLE";
            case RECV: return "RECV";
            case TRANSMIT: return "TRANSMIT";
            case SLEEP: return "SLEEP";
            default: return "???";
        }
    }

};



#endif
