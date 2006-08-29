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


#define coreEV (ev.disabled()||!coreDebug) ? std::cout : EV << logName() << "::ChannelAccess: "

/**
 * Upon initialization ChannelAccess registers the nic parent module
 * to have all its connections handled by ChannelControl
 */
void ChannelAccess::initialize(int stage)
{
    BasicModule::initialize(stage);

    if (stage == 0)
    {
        cc = dynamic_cast<ChannelControl *>(simulation.moduleByPath("channelcontrol"));
        if (cc == 0)
            error("Could not find channelcontrol module");

        // register to get a notification when position changes
        nb->subscribe(this, NF_HOSTPOSITION_UPDATED);
    }
    else if (stage == 2)
    {
        cModule *hostModule = findHost();
        myHostRef = cc->lookupHost(hostModule);
        if (myHostRef==0)
            error("host not registered yet in ChannelControl (this should be done by "
                  "the Mobility module -- maybe this host doesn't have one?)");
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
    const ChannelControl::ModuleList& neighbors = cc->getNeighbors(myHostRef);
    coreEV << "sendToChannel: sending to gates\n";

    // loop through all hosts in range
    ChannelControl::ModuleList::const_iterator it;
    for (it = neighbors.begin(); it != neighbors.end(); ++it)
    {
        cModule *mod = *it;

        // we need to send to each radioIn[] gate
        cGate *radioGate = mod->gate("radioIn");

        if (radioGate == NULL)
            error("module %s must have a gate called radioIn", mod->fullPath().c_str());

        for (int i = 0; i < radioGate->size(); i++)
        {
            ChannelControl::HostRef h = cc->lookupHost(mod);

            if (h == NULL)
                error("cannot find module in channel control");

            if (h->channel == msg->getChannelNumber())
            {
                coreEV << "sending message to host listening on the same channel\n";
                // account for propagation delay, based on distance in meters
                // Over 300m, dt=1us=10 bit times @ 10Mbps
                sendDirect((cMessage *)msg->dup(), myHostRef->pos.distance(h->pos) / LIGHT_SPEED, mod, radioGate->id() + i);
            }
            else
                coreEV << "skipping host listening on a different channel\n";
        }
    }

    // register transmission in ChannelControl
    cc->addOngoingTransmission(myHostRef, msg);
}
