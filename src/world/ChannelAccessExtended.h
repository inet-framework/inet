/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelAccess.h
 *
 * author:      Marc Loebbers
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


#ifndef CHANNEL_ACCESS_EXTENDED_H
#define CHANNEL_ACCESS_EXTENDED_H

#include <list>
#include <omnetpp.h>
#include "AirFrame_m.h"
#include "BasicModule.h"
#include "ChannelControlExtended.h"
#include "ChannelAccess.h"

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with ChannelControl to handle
 * the dynamically created gates. This means EVERY SnrEval (the lowest
 * layer in a host) has to be derived from this class!!!! And please
 * follow the instructions on how to declare a physical layer in a
 * .ned file in "The Design of a Mobility Framework in OMNeT++"
 * paper.
 *
 * Please don't touch this class.
 *
 * @author Marc L�bbers
 * @ingroup channelControl
 * @ingroup phyLayer
 */
class INET_API ChannelAccessExtended : public ChannelAccess
{
  protected:
    /** @brief Pointer to the ChannelControl module*/
    ChannelControlExtended* ccExt;

    /** @brief Identifies this host in the ChannelControl module*/

  protected:
    /** @brief Sends a message to all hosts in range*/
    virtual void sendToChannel(AirFrame *msg);

  protected:
    /** @brief Register with ChannelControl and subscribe to hostPos*/
    virtual void initialize(int stage);
};

#endif

