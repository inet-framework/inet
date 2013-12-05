/***************************************************************************
 * file:        SimplifiedRadioChannelAccess.cc
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


#include "ModuleAccess.h"
#include "SimplifiedRadioChannelAccess.h"
#include "IMobility.h"
#include "ModuleAccess.h"

static int parseInt(const char *s, int defaultValue)
{
    if (!s || !*s)
        return defaultValue;

    char *endptr;
    int value = strtol(s, &endptr, 10);
    return *endptr == '\0' ? value : defaultValue;
}

// the destructor unregister the radio module
SimplifiedRadioChannelAccess::~SimplifiedRadioChannelAccess()
{
    if (cc && myRadioRef)
    {
        // check if radio channel exist
        ISimplifiedRadioChannel *cc = dynamic_cast<ISimplifiedRadioChannel *>(simulation.getModuleByPath("radioChannel"));
        if (cc)
             cc->unregisterRadio(myRadioRef);
        myRadioRef = NULL;
    }
}

/**
 * Upon initialization SimplifiedRadioChannelAccess registers the nic parent module
 * to have all its connections handled by SimplifiedRadioChannel
 */
void SimplifiedRadioChannelAccess::initialize(int stage)
{
    RadioBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        cc = getSimplifiedRadioChannel();
        hostModule = getContainingNode(this);
        myRadioRef = NULL;

        positionUpdateArrived = false;
        hostModule->subscribe(IMobility::mobilityStateChangedSignal, this);
        myRadioRef = cc->registerRadio(this);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        if (!positionUpdateArrived)
        {
            radioPos.x = parseInt(hostModule->getDisplayString().getTagArg("p", 0), -1);
            radioPos.y = parseInt(hostModule->getDisplayString().getTagArg("p", 1), -1);

            if (radioPos.x == -1 || radioPos.y == -1)
                throw cRuntimeError("The coordinates of '%s' host are invalid. Please set coordinates in "
                        "'@display' attribute, or configure Mobility for this host.",
                        hostModule->getFullPath().c_str());

            const char *s = hostModule->getDisplayString().getTagArg("p", 2);
            if (s && *s)
                throw cRuntimeError("The coordinates of '%s' host are invalid. Please remove automatic arrangement"
                        " (3rd argument of 'p' tag)"
                        " from '@display' attribute, or configure Mobility for this host.",
                        hostModule->getFullPath().c_str());
        }

        cc->setRadioPosition(myRadioRef, radioPos);
    }
}

ISimplifiedRadioChannel *SimplifiedRadioChannelAccess::getSimplifiedRadioChannel()
{
    ISimplifiedRadioChannel *cc = dynamic_cast<ISimplifiedRadioChannel *>(simulation.getModuleByPath("radioChannel"));
    if (!cc)
        throw cRuntimeError("Could not find SimplifiedRadioChannel module with name 'radioChannel' in the toplevel network.");
    return cc;
}

/**
 * This function has to be called whenever a packet is supposed to be
 * sent to the channel.
 *
 * This function really sends the message away, so if you still want
 * to work with it you should send a duplicate!
 */
void SimplifiedRadioChannelAccess::sendToChannel(SimplifiedRadioFrame *msg)
{
    EV << "sendToChannel: sending to gates\n";

    // delegate it to SimplifiedRadioChannel
    cc->sendToChannel(myRadioRef, msg);
}

void SimplifiedRadioChannelAccess::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if (signalID == IMobility::mobilityStateChangedSignal)
    {
        IMobility *mobility = check_and_cast<IMobility*>(obj);
        radioPos = mobility->getCurrentPosition();
        positionUpdateArrived = true;

        if (myRadioRef)
            cc->setRadioPosition(myRadioRef, radioPos);
    }
}

