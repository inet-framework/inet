/***************************************************************************
 * file:        ChannelAccessExtended.cc
 *
 * author:      Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 * copyright:   (C) 2009 Juan-Carlos Maureira
 * copyright:   (C) 2009 Alfonso Ariza
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 *
 *              ChangeLog
 *              -- Added Multiple radios support (Juan-Carlos Maureira / Paula Uribe. INRIA 2009)
 *
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#include "ChannelAccessExtended.h"


#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << logName() << "::ChannelAccess: "

/**
 * Upon initialization ChannelAccess registers the nic parent module
 * to have all its connections handled by ChannelControl
 */
void ChannelAccessExtended::initialize(int stage)
{
    BasicModule::initialize(stage);

    if (stage == 0)
    {
        ccExt = ChannelControlExtended::get();
        if (ccExt==NULL)
            cc = ChannelControl::get();
        else
            cc = ccExt;

        // register to get a notification when position changes
        nb->subscribe(this, NF_HOSTPOSITION_UPDATED);
    }
    else if (stage == 2)
    {
        cModule *hostModule = findHost();
        if (ccExt)
            myHostRef = ccExt->lookupHost(hostModule);
        else
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
void ChannelAccessExtended::sendToChannel(AirFrame *msg)
{
    if (!ccExt)
    {
        ChannelAccess::sendToChannel(msg);
        return;
    }
    ccExt->sendToChannel(this, myHostRef, msg);
}
