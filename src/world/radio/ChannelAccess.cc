/***************************************************************************
 * file:        ChannelAccess.cc
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


#include "ChannelAccess.h"


#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << logName() << "::ChannelAccess: "

static int parseInt(const char *s, int defaultValue)
{
    if (!s || !*s)
        return defaultValue;

    char *endptr;
    int value = strtol(s, &endptr, 10);
    return *endptr == '\0' ? value : defaultValue;
}

/**
 * Upon initialization ChannelAccess registers the nic parent module
 * to have all its connections handled by ChannelControl
 */
void ChannelAccess::initialize(int stage)
{
    BasicModule::initialize(stage);

    if (stage == 0)
    {
        ccExt = ChannelControlExtended::get();

        if (ccExt == NULL)
            cc = ChannelControl::get();
        else
            cc = ccExt;

        cModule *hostModule = findHost();

        if (*hostModule->getDisplayString().getTagArg("p", 2))
            error("");

        hostPos.x = parseInt(hostModule->getDisplayString().getTagArg("p", 0), -1);
        hostPos.y = parseInt(hostModule->getDisplayString().getTagArg("p", 1), -1);
        posFromDisplayString = true;

        // register to get a notification when position changes
        nb->subscribe(this, NF_HOSTPOSITION_UPDATED);
    }
    else if (stage == 2)
    {
        cModule *hostModule = findHost();

        if (posFromDisplayString)
        {
            const char *s = hostModule->getDisplayString().getTagArg("p", 2);
            if (s && *s)
                error("The coordinates of '%s' host are invalid. Please remove automatic arrangement"
                        " (3rd argument of 'p' tag)"
                        " from '@display' attribute, or configure Mobility for this host.",
                        hostModule->getFullPath().c_str());
        }

        if (hostPos.x == -1 || hostPos.y == -1)
            error("The coordinates of '%s' host are invalid. Please set coordinates in "
                    "'@display' attribute, or configure Mobility for this host.",
                    hostModule->getFullPath().c_str());

        myHostRef = cc->registerHost(hostModule, hostPos);

        if (myHostRef == 0)
            error("host not registered yet in 'channelControl' module");

        double r = cc->getCommunicationRange(myHostRef);
        myHostRef->host->getDisplayString().setTagArg("r", 0, (long) r);
    }
}


/**
 * This function has to be called whenever a packet is supposed to be
 * sent to the channel.
 *
 * This function really sends the message away, so if you still want
 * to work with it you should send a duplicate!
 */
void ChannelAccess::sendToChannel(AirFrame *msg)
{
    coreEV << "sendToChannel: sending to gates\n";

    // delegate it to ChannelControl
    cc->sendToChannel(this, myHostRef, msg);
}

void ChannelAccess::receiveChangeNotification(int category, const cPolymorphic *details)
{
    if (category == NF_HOSTPOSITION_UPDATED)
    {
        const Coord *pos = check_and_cast<const Coord*>(details);

        hostPos = *pos;
        posFromDisplayString = false;

        if (myHostRef)
        {
            cc->updateHostPosition(myHostRef, hostPos);
            double r = cc->getCommunicationRange(myHostRef);
            myHostRef->host->getDisplayString().setTagArg("r", 0, (long) r);
        }
    }

    BasicModule::receiveChangeNotification(category, details);
}

