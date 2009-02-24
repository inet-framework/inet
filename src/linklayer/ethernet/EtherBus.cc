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

#include "EtherBus.h"
#include "EtherFrame_m.h"  // for EtherAutoconfig only

Define_Module(EtherBus);

static cEnvir& operator<< (cEnvir& out, cMessage *msg)
{
    out.printf("(%s)%s",msg->getClassName(),msg->getFullName());
    return out;
}

EtherBus::EtherBus()
{
    tap = NULL;
}

EtherBus::~EtherBus()
{
    delete [] tap;
}

void EtherBus::initialize()
{
    numMessages = 0;
    WATCH(numMessages);

    propagationSpeed = par("propagationSpeed").doubleValue();

    // initialize the positions where the hosts connects to the bus
    taps = gateSize("ethg");

    // read positions and check if positions are defined in order (we're lazy to sort...)
    std::vector<double> pos;
    tokenize(par("positions").stringValue(), pos);
    int numPos = pos.size();
    if (numPos>taps)
        EV << "Note: `positions' parameter contains more values ("<< numPos << ") than "
              "the number of taps (" << taps << "), ignoring excess values.\n";
    else if (numPos<taps && numPos>=2)
        EV << "Note: `positions' parameter contains less values ("<< numPos << ") than "
              "the number of taps (" << taps << "), repeating distance between last 2 positions.\n";
    else if (numPos<taps && numPos<2)
        EV << "Note: `positions' parameter contains too few values, using 5m distances.\n";

    tap = new BusTap[taps];

    int i;
    double distance = numPos>=2 ? pos[numPos-1]-pos[numPos-2] : 5;
    for (i=0; i<taps; i++)
    {
        tap[i].id = i;
        tap[i].position = i<numPos ? pos[i] : i==0 ? 5 : tap[i-1].position+distance;
    }
    for (i=0; i<taps-1; i++)
    {
        if (tap[i].position > tap[i+1].position)
            error("Tap positions must be ordered in ascending fashion, modify 'positions' parameter and rerun\n");
    }

    // Calculate propagation of delays between tap points on the bus
    for (i=0; i<taps; i++)
    {
        // Propagation delay between adjacent tap points
        if (i == 0) {
            tap[i].propagationDelay[UPSTREAM] = 0;
            tap[i].propagationDelay[DOWNSTREAM] = (tap[i+1].position - tap[i].position)/propagationSpeed;
        }
        else if (i == taps-1) {
            tap[i].propagationDelay[UPSTREAM] = tap[i-1].propagationDelay[DOWNSTREAM];
            tap[i].propagationDelay[DOWNSTREAM] = 0;
        }
        else {
            tap[i].propagationDelay[UPSTREAM] = tap[i-1].propagationDelay[DOWNSTREAM];
            tap[i].propagationDelay[DOWNSTREAM] = (tap[i+1].position - tap[i].position)/propagationSpeed;;
        }
    }

    // Prints out data of parameters for parameter checking...
    EV << "Parameters of (" << getClassName() << ") " << getFullPath() << "\n";
    EV << "propagationSpeed: " << propagationSpeed << "\n";
    for (i=0; i<taps; i++)
    {
        EV << "tap[" << i << "] pos: " << tap[i].position <<
              "  upstream delay: " << tap[i].propagationDelay[UPSTREAM] <<
              "  downstream delay: " << tap[i].propagationDelay[DOWNSTREAM] << endl;
    }
    EV << "\n";

    // autoconfig: tell everyone that bus supports only 10Mb half-duplex
    EV << "Autoconfig: advertising that we only support 10Mb half-duplex operation\n";
    for (i=0; i<taps; i++)
    {
        EtherAutoconfig *autoconf = new EtherAutoconfig("autoconf-10Mb-halfduplex");
        autoconf->setHalfDuplex(true);
        autoconf->setTxrate(10000000); // 10Mb
        send(autoconf,"ethg$o",i);
    }
}

void EtherBus::handleMessage (cMessage *msg)
{
    if (!msg->isSelfMessage())
    {
        // Handle frame sent down from the network entity
        int tapPoint = msg->getArrivalGate()->getIndex();
        EV << "Frame " << msg << " arrived on tap " << tapPoint << endl;

        // create upstream and downstream events
        if (tapPoint>0)
        {
            // start UPSTREAM travel
            // if goes downstream too, we need to make a copy
            cMessage *msg2 = (tapPoint<taps-1) ? (cMessage *)msg->dup() : msg;
            msg2->setKind(UPSTREAM);
            msg2->setContextPointer(&tap[tapPoint-1]);
            scheduleAt(simTime()+tap[tapPoint].propagationDelay[UPSTREAM], msg2);
        }
        if (tapPoint<taps-1)
        {
            // start DOWNSTREAM travel
            msg->setKind(DOWNSTREAM);
            msg->setContextPointer(&tap[tapPoint+1]);
            scheduleAt(simTime()+tap[tapPoint].propagationDelay[DOWNSTREAM], msg);
        }
        if (taps==1)
        {
            // if there's only one tap, there's nothing to do
            delete msg;
        }
    }
    else
    {
        // handle upstream and downstream events
        int direction = msg->getKind();
        BusTap *thistap = (BusTap *) msg->getContextPointer();
        int tapPoint = thistap->id;

        EV << "Event " << msg << " on tap " << tapPoint << ", sending out frame\n";

        // send out on gate
        bool isLast = (direction==UPSTREAM) ? (tapPoint==0) : (tapPoint==taps-1);
        cPacket *msg2 = isLast ? PK(msg) : PK(msg->dup());
        send(msg2, "ethg$o", tapPoint);

        // if not end of the bus, schedule for next tap
        if (isLast)
        {
            EV << "End of bus reached\n";
        }
        else
        {
            EV << "Scheduling for next tap\n";
            int nextTap = (direction==UPSTREAM) ? (tapPoint-1) : (tapPoint+1);
            msg->setContextPointer(&tap[nextTap]);
            scheduleAt(simTime()+tap[tapPoint].propagationDelay[direction], msg);
        }
    }
}

void EtherBus::tokenize(const char *str, std::vector<double>& array)
{
    char *str2 = opp_strdup(str);
    if (!str2) return;
        char *s = strtok(str2, " ");
    while (s)
    {
        array.push_back(atof(s));
        s = strtok(NULL, " ");
    }
    delete [] str2;
}


void EtherBus::finish ()
{
    simtime_t t = simTime();
    recordScalar("simulated time", t);
    recordScalar("messages handled", numMessages);
    if (t>0)
        recordScalar("messages/sec", numMessages/t);
}
