//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef TraCIDemo_H
#define TraCIDemo_H

#include <omnetpp.h>
#include "UDPSocket.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"
#include "mobility/single/TraCIMobility.h"

/**
 * Small IVC Demo
 */
class TraCIDemo : public cSimpleModule, protected cListener, public ILifecycle
{
    public:
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    protected:
        virtual int numInitStages() const { return 4; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage* msg);

    protected:
        TraCIMobility* traci;
        bool sentMessage;
        UDPSocket socket;
        static simsignal_t mobilityStateChangedSignal;

    protected:
        void setupLowerLayer();
        virtual void handleSelfMsg(cMessage* apMsg);
        virtual void handleLowerMsg(cMessage* apMsg);

        virtual void sendMessage();
        virtual void handlePositionUpdate();
};

#endif

