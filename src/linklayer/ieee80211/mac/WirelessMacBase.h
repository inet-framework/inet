/* -*- mode:c++ -*- ********************************************************
 * file:        WirelessMacBase.h
 *
 * derived by Andras Varga using decremental programming from BasicMacLayer.h,
 * which had the following copyright:
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU Lesser General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#ifndef WIRELESSMACBASE_H
#define WIRELESSMACBASE_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "NotifierConsts.h"

class NotificationBoard;

/**
 * Base class for wireless MAC protocols that use MF's physical layer.
 *
 * @author Daniel Willkomm, Andras Varga
 */
class INET_API WirelessMacBase : public cSimpleModule
{
  protected:
    /** @brief gate id*/
    //@{
    int uppergateIn;
    int uppergateOut;
    int lowergateIn;
    int lowergateOut;
    //@}

    /** @brief Cached pointer to the NotificationBoard module*/
    NotificationBoard *nb;

  protected:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Called every time a message arrives*/
    virtual void handleMessage(cMessage *msg);

  protected:
    /**
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     */
    //@{
    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage *msg) = 0;

    /** @brief Handle packets from upper layer */
    virtual void handleUpperMsg(cPacket *msg) = 0;

    /** @brief Handle commands from upper layer */
    virtual void handleCommand(cMessage *msg) = 0;

    /** @brief Handle packets from lower layer */
    virtual void handleLowerMsg(cPacket *msg) = 0;

    virtual bool isUpperMsg(cMessage *msg);
    virtual bool isLowerMsg(cMessage *msg);
    //@}

    /** @name Convenience Functions*/
    //@{
    /** @brief Sends a message to the lower layer */
    virtual void sendDown(cMessage *msg);

    /** @brief Sends a message to the upper layer */
    virtual void sendUp(cMessage *msg);
    //@}
};

#endif

