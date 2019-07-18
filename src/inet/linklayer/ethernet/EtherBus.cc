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

#include "inet/common/INETMath.h"
#include "inet/linklayer/ethernet/EtherBus.h"

namespace inet {

Define_Module(EtherBus);

inline std::ostream& operator<<(std::ostream& os, cMessage *msg)
{
    os << "(" << msg->getClassName() << ")" << msg->getFullName();
    return os;
}

EtherBus::EtherBus()
{
}

EtherBus::~EtherBus()
{
    delete[] tap;
}

void EtherBus::initialize()
{
    numMessages = 0;
    WATCH(numMessages);

    propagationSpeed = par("propagationSpeed");  //TODO there is a hardcoded propagation speed in EtherMACBase.cc -- use that?

    // initialize the positions where the hosts connects to the bus
    numTaps = gateSize("ethg");
    inputGateBaseId = gateBaseId("ethg$i");
    outputGateBaseId = gateBaseId("ethg$o");

    // read positions and check if positions are defined in order (we're lazy to sort...)
    std::vector<double> pos = cStringTokenizer(par("positions")).asDoubleVector();
    int numPos = pos.size();

    if (numPos > numTaps)
        EV << "Note: `positions' parameter contains more values (" << numPos << ") than "
                                                                                "the number of taps (" << numTaps << "), ignoring excess values.\n";
    else if (numPos < numTaps && numPos >= 2)
        EV << "Note: `positions' parameter contains less values (" << numPos << ") than "
                                                                                "the number of taps (" << numTaps << "), repeating distance between last 2 positions.\n";
    else if (numPos < numTaps && numPos < 2)
        EV << "Note: `positions' parameter contains too few values, using 5m distances.\n";

    tap = new BusTap[numTaps];

    int i;
    double distance = numPos >= 2 ? pos[numPos - 1] - pos[numPos - 2] : 5;

    for (i = 0; i < numTaps; i++) {
        tap[i].id = i;
        tap[i].position = i < numPos ? pos[i] : i == 0 ? 5 : tap[i - 1].position + distance;
    }

    for (i = 0; i < numTaps - 1; i++) {
        if (tap[i].position > tap[i + 1].position)
            throw cRuntimeError("Tap positions must be ordered in ascending fashion, modify 'positions' parameter and rerun\n");
    }

    // Prints out data of parameters for parameter checking...
    EV << "Parameters of (" << getClassName() << ") " << getFullPath() << "\n";
    EV << "propagationSpeed: " << propagationSpeed << "\n";

    // Calculate propagation of delays between tap points on the bus
    for (i = 0; i < numTaps; i++) {
        // Propagation delay between adjacent tap points
        tap[i].propagationDelay[UPSTREAM] = (i > 0) ? tap[i - 1].propagationDelay[DOWNSTREAM] : 0;
        tap[i].propagationDelay[DOWNSTREAM] = (i + 1 < numTaps) ? (tap[i + 1].position - tap[i].position) / propagationSpeed : 0;
        EV << "tap[" << i << "] pos: " << tap[i].position
           << "  upstream delay: " << tap[i].propagationDelay[UPSTREAM]
           << "  downstream delay: " << tap[i].propagationDelay[DOWNSTREAM] << endl;
    }
    EV << "\n";

    // ensure we receive frames when their first bits arrive
    for (int i = 0; i < numTaps; i++)
        gate(inputGateBaseId + i)->setDeliverOnReceptionStart(true);
    subscribe(POST_MODEL_CHANGE, this);    // we'll need to do the same for dynamically added gates as well

    checkConnections(true);
}

void EtherBus::checkConnections(bool errorWhenAsymmetric)
{
    int numActiveTaps = 0;

    double datarate = 0.0;

    for (int i = 0; i < numTaps; i++) {
        cGate *igate = gate(inputGateBaseId + i);
        cGate *ogate = gate(outputGateBaseId + i);
        if (!igate->isConnected() && !ogate->isConnected())
            continue;

        if (!igate->isConnected() || !ogate->isConnected()) {
            // half connected gate
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input or output gate not connected at tap %i", i);
            dataratesDiffer = true;
            EV << "The input or output gate not connected at tap " << i << ".\n";
            continue;
        }

        numActiveTaps++;
        double drate = igate->getIncomingTransmissionChannel()->getNominalDatarate();

        if (numActiveTaps == 1)
            datarate = drate;
        else if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The input datarate at tap %i differs from datarates of previous taps", i);
            dataratesDiffer = true;
            EV << "The input datarate at tap " << i << " differs from datarates of previous taps.\n";
        }

        cChannel *outTrChannel = ogate->getTransmissionChannel();
        drate = outTrChannel->getNominalDatarate();

        if (datarate != drate) {
            if (errorWhenAsymmetric)
                throw cRuntimeError("The output datarate at tap %i differs from datarates of previous taps", i);
            dataratesDiffer = true;
            EV << "The output datarate at tap " << i << " differs from datarates of previous taps.\n";
        }

        if (!outTrChannel->isSubscribed(POST_MODEL_CHANGE, this))
            outTrChannel->subscribe(POST_MODEL_CHANGE, this);
    }
}

void EtherBus::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    ASSERT(signalID == POST_MODEL_CHANGE);

    // throw error if new gates have been added
    cPostGateVectorResizeNotification *notif = dynamic_cast<cPostGateVectorResizeNotification *>(obj);
    if (notif) {
        if (strcmp(notif->gateName, "ethg") == 0)
            throw cRuntimeError("EtherBus does not allow adding/removing links dynamically");
    }

    cPostPathCreateNotification *connNotif = dynamic_cast<cPostPathCreateNotification *>(obj);
    if (connNotif) {
        if ((this == connNotif->pathStartGate->getOwnerModule()) || (this == connNotif->pathEndGate->getOwnerModule()))
            checkConnections(false);
        return;
    }

    cPostPathCutNotification *cutNotif = dynamic_cast<cPostPathCutNotification *>(obj);
    if (cutNotif) {
        if ((this == cutNotif->pathStartGate->getOwnerModule()) || (this == cutNotif->pathEndGate->getOwnerModule()))
            checkConnections(false);
        return;
    }

    // note: we are subscribed to the channel object too
    cPostParameterChangeNotification *parNotif = dynamic_cast<cPostParameterChangeNotification *>(obj);
    if (parNotif) {
        cChannel *channel = dynamic_cast<cDatarateChannel *>(parNotif->par->getOwner());
        if (channel) {
            cGate *gate = channel->getSourceGate();
            if (gate->pathContains(this))
                checkConnections(false);
        }
        return;
    }
}

void EtherBus::handleMessage(cMessage *msg)
{
    if (dataratesDiffer)
        checkConnections(true);

    if (!msg->isSelfMessage()) {
        // Handle frame sent down from the network entity
        int tapPoint = msg->getArrivalGate()->getIndex();
        EV << "Frame " << msg << " arrived on tap " << tapPoint << endl;

        // create upstream and downstream events
        if (tapPoint > 0) {
            // start UPSTREAM travel
            // if goes downstream too, we need to make a copy
            cMessage *msg2 = (tapPoint < numTaps - 1) ? msg->dup() : msg;
            msg2->setKind(UPSTREAM);
            msg2->setContextPointer(&tap[tapPoint - 1]);
            scheduleAt(simTime() + tap[tapPoint].propagationDelay[UPSTREAM], msg2);
        }

        if (tapPoint < numTaps - 1) {
            // start DOWNSTREAM travel
            msg->setKind(DOWNSTREAM);
            msg->setContextPointer(&tap[tapPoint + 1]);
            scheduleAt(simTime() + tap[tapPoint].propagationDelay[DOWNSTREAM], msg);
        }

        if (numTaps == 1) {
            // if there's only one tap, there's nothing to do
            delete msg;
        }
    }
    else {
        // handle upstream and downstream events
        int direction = msg->getKind();
        BusTap *thistap = (BusTap *)msg->getContextPointer();
        int tapPoint = thistap->id;
        msg->setContextPointer(nullptr);

        EV << "Event " << msg << " on tap " << tapPoint << ", sending out frame\n";

        // send out on gate
        bool isLast = (direction == UPSTREAM) ? (tapPoint == 0) : (tapPoint == numTaps - 1);
        cGate *ogate = gate(outputGateBaseId + tapPoint);
        if (ogate->isConnected()) {
            // send out on gate
            cMessage *msg2 = isLast ? msg : msg->dup();

            // stop current transmission
            ogate->getTransmissionChannel()->forceTransmissionFinishTime(SIMTIME_ZERO);
            send(msg2, ogate);
        }
        else {
            // skip gate
            if (isLast)
                delete msg;
        }

        // if not end of the bus, schedule for next tap
        if (isLast) {
            EV << "End of bus reached\n";
        }
        else {
            EV << "Scheduling for next tap\n";
            int nextTap = (direction == UPSTREAM) ? (tapPoint - 1) : (tapPoint + 1);
            msg->setContextPointer(&tap[nextTap]);
            scheduleAt(simTime() + tap[tapPoint].propagationDelay[direction], msg);
        }
    }
}

void EtherBus::finish()
{
    simtime_t t = simTime();
    recordScalar("simulated time", t);
    recordScalar("messages handled", numMessages);

    if (t > 0)
        recordScalar("messages/sec", numMessages / t);
}

} // namespace inet

