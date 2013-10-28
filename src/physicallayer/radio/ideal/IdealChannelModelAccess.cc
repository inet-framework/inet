//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe
//

#include "IdealChannelModelAccess.h"
#include "ModuleAccess.h"
#include "IMobility.h"

simsignal_t IdealChannelModelAccess::mobilityStateChangedSignal = registerSignal("mobilityStateChanged");

// the destructor unregister the radio module
IdealChannelModelAccess::~IdealChannelModelAccess()
{
    if (cc && myRadioRef)
    {
        // check if channel control exist
        IdealChannelModel *cc = dynamic_cast<IdealChannelModel *>(simulation.getModuleByPath("channelControl"));
        if (cc)
             cc->unregisterRadio(myRadioRef);
        myRadioRef = NULL;
    }
}

/**
 * Upon initialization IdealChannelModelAccess registers the nic parent module
 * to have all its connections handled by ChannelControl
 */
void IdealChannelModelAccess::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        cc = dynamic_cast<IdealChannelModel *>(simulation.getModuleByPath("channelControl"));
        if (!cc)
            throw cRuntimeError("Could not find IdealChannelModel module with name 'channelControl' in the toplevel network.");

        hostModule = getContainingNode(this);

        positionUpdateArrived = false;
        // register to get a notification when position changes
        hostModule->subscribe(mobilityStateChangedSignal, this);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        if (!positionUpdateArrived)
            throw cRuntimeError("The coordinates of '%s' host are invalid. Please configure Mobility for this host.", hostModule->getFullPath().c_str());

        myRadioRef = cc->registerRadio(this);
        cc->setRadioPosition(myRadioRef, radioPos);
    }
}

/**
 * This function has to be called whenever a packet is supposed to be
 * sent to the channel.
 *
 * This function really sends the message away, so if you still want
 * to work with it you should send a duplicate!
 */
void IdealChannelModelAccess::sendToChannel(IdealAirFrame *msg)
{
    EV << "sendToChannel: sending to gates\n";

    // delegate it to ChannelControl
    cc->sendToChannel(myRadioRef, msg);
}

void IdealChannelModelAccess::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
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

