/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "EtherHub.h"


Define_Module(EtherHub);


simsignal_t EtherHub::pkSignal = SIMSIGNAL_NULL;

static cEnvir& operator<<(cEnvir& out, cMessage *msg)
{
    out.printf("(%s)%s", msg->getClassName(), msg->getFullName());
    return out;
}

void EtherHub::initialize()
{
    numPorts = gateSize("ethg");
    inputGateBaseId = gateBaseId("ethg$i");
    outputGateBaseId = gateBaseId("ethg$o");
    pkSignal = registerSignal("pk");

    numMessages = 0;
    WATCH(numMessages);

    // ensure we receive frames when their first bits arrive
    for (int i = 0; i < numPorts; i++)
        gate(inputGateBaseId + i)->setDeliverOnReceptionStart(true);
    subscribe(POST_MODEL_CHANGE, this);  // we'll need to do the same for dynamically added gates as well

    // To keep the code small, we only check datarates once on startup. If it's important to check
    // it after dynamic model changes too, it can be done by listening on the POST_MODEL_CHANGE
    // signal; see EtherMACBase for how it's done.
    checkConnections();
}

void EtherHub::checkConnections()
{
    int numActivePorts = 0;
    double datarate = 0.0;

    for (int i = 0; i < numPorts; i++)
    {
        cGate *igate = gate(inputGateBaseId + i);
        cGate *ogate = gate(outputGateBaseId + i);
        if (!igate->isConnected() || !ogate->isConnected())
            continue;

        numActivePorts++;
        double drate = igate->getIncomingTransmissionChannel()->getNominalDatarate();

        if (numActivePorts == 1)
            datarate = drate;
        else if (datarate != drate)
            throw cRuntimeError("The input datarate at port %i differs from datarates of previous ports", i);

        drate = gate(outputGateBaseId + i)->getTransmissionChannel()->getNominalDatarate();

        if (datarate != drate)
            throw cRuntimeError("The output datarate at port %i differs from datarates of previous ports", i);
    }
}

void EtherHub::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    ASSERT(signalID == POST_MODEL_CHANGE);

    // if new gates have been added, we need to call setDeliverOnReceptionStart(true) on them
    cPostGateVectorResizeNotification *notif = dynamic_cast<cPostGateVectorResizeNotification*>(obj);
    if (notif)
    {
        if (strcmp(notif->gateName, "ethg") == 0)
        {
            int newSize = gateSize("ethg");
            for (int i = notif->oldSize; i < newSize; i++)
                gate(inputGateBaseId + i)->setDeliverOnReceptionStart(true);
        }
    }
}

void EtherHub::handleMessage(cMessage *msg)
{
    // Handle frame sent down from the network entity: send out on every other port
    int arrivalPort = msg->getArrivalGate()->getIndex();
    EV << "Frame " << msg << " arrived on port " << arrivalPort << ", broadcasting on all other ports\n";

    numMessages++;
    emit(pkSignal, msg);

    if (numPorts <= 1)
    {
        delete msg;
        return;
    }

    for (int i = 0; i < numPorts; i++)
    {
        if (i != arrivalPort)
        {
            cGate *ogate = gate(outputGateBaseId + i);
            if (!ogate->isConnected())
                continue;

            bool isLast = (arrivalPort == numPorts-1) ? (i == numPorts-2) : (i == numPorts-1);
            cMessage *msg2 = isLast ? msg : msg->dup();

            // stop current transmission
            ogate->getTransmissionChannel()->forceTransmissionFinishTime(SIMTIME_ZERO);

            // send
            send(msg2, ogate);

            if (isLast)
                msg = NULL;  // msg sent, do not delete it.
        }
    }
    delete msg;
}

void EtherHub::finish()
{
    simtime_t t = simTime();
    recordScalar("simulated time", t);

    if (t > 0)
        recordScalar("messages/sec", numMessages / t);
}

