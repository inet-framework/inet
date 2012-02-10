/***************************************************************************
 * file:        ChannelAccess.cc
 *
 * author:      Marc Loebbers, Rudolf Hornig, Zoltan Bojthe
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
 **************************************************************************/


#include "ChannelAccess.h"
#include "IMobility.h"

#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << logName() << "::ChannelAccess: "

simsignal_t ChannelAccess::mobilityStateChangedSignal = SIMSIGNAL_NULL;

static int parseInt(const char *s, int defaultValue)
{
    if (!s || !*s)
        return defaultValue;

    char *endptr;
    int value = strtol(s, &endptr, 10);
    return *endptr == '\0' ? value : defaultValue;
}

// the destructor unregister the radio module
ChannelAccess::~ChannelAccess()
{
    if (cc && myRadioRef)
    {
        // check if channel control exist
        IChannelControl *cc = dynamic_cast<IChannelControl *>(simulation.getModuleByPath("channelControl"));
        if (cc)
             cc->unregisterRadio(myRadioRef);
        myRadioRef = NULL;
    }
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
        cc = getChannelControl();
        nb = NotificationBoardAccess().get();
        hostModule = findHost();

        positionUpdateArrived = false;
        // register to get a notification when position changes
        mobilityStateChangedSignal = registerSignal("mobilityStateChanged");
        hostModule->subscribe(mobilityStateChangedSignal, this);
    }
    else if (stage == 2)
    {
        if (!positionUpdateArrived)
        {
            radioPos.x = parseInt(hostModule->getDisplayString().getTagArg("p", 0), -1);
            radioPos.y = parseInt(hostModule->getDisplayString().getTagArg("p", 1), -1);

            if (radioPos.x == -1 || radioPos.y == -1)
                error("The coordinates of '%s' host are invalid. Please set coordinates in "
                        "'@display' attribute, or configure Mobility for this host.",
                        hostModule->getFullPath().c_str());

            const char *s = hostModule->getDisplayString().getTagArg("p", 2);
            if (s && *s)
                error("The coordinates of '%s' host are invalid. Please remove automatic arrangement"
                        " (3rd argument of 'p' tag)"
                        " from '@display' attribute, or configure Mobility for this host.",
                        hostModule->getFullPath().c_str());
        }

        myRadioRef = cc->registerRadio(this);
        cc->setRadioPosition(myRadioRef, radioPos);
    }
}

IChannelControl *ChannelAccess::getChannelControl()
{
    IChannelControl *cc = dynamic_cast<IChannelControl *>(simulation.getModuleByPath("channelControl"));
    if (!cc)
        throw cRuntimeError("Could not find ChannelControl module with name 'channelControl' in the toplevel network.");
    return cc;
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
    cc->sendToChannel(myRadioRef, msg);
}

void ChannelAccess::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if (signalID == mobilityStateChangedSignal)
    {
        IMobility *mobility = check_and_cast<IMobility*>(obj);
        radioPos = mobility->getCurrentPosition();
        positionUpdateArrived = true;

        if (myRadioRef)
            cc->setRadioPosition(myRadioRef, radioPos);
    }
}

