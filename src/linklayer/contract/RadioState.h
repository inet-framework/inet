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

#include "INETDefs.h"

/**
 * Holds the current state and other properties of the radio.
 * Possible states are:
 *
 *  - IDLE: channel is empty (radio is in receive mode)
 *  - RECV: channel is busy (radio is in receive mode)
 *  - TRANSMIT: the radio is transmitting
 *  - SLEEP: the radio is sleeping
 *  - OFF: the device is turned off
 *
 * @author Andreas Koepke, Andras Varga
 */
class INET_API RadioState : public cObject
{
  public:
    /** Possible states of the radio */
    enum State
    {
      IDLE,
      RECV,
      TRANSMIT,
      SLEEP, //XXX do we need this, or simply OFF would suffice?
      OFF
    };
};

#endif
