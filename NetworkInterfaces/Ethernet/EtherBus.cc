/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <vector>
#include <omnetpp.h>
#include "INETDefs.h"
#include "EtherFrame_m.h"  // for EtherAutoconfig only
#include "utils.h"

// Direction of frame travel on bus; also used as selfmessage kind
#define UPSTREAM        0
#define DOWNSTREAM      1



// Implements the physical locations on the bus where each network entity is connected to on the bus
struct BusTap
{
    int id;                         // which tap is this
    double position;                // Physical location of where each entity is connected to on the bus, (physical location of the tap on the bus)
    simtime_t propagationDelay[2];  // Propagation delays to the adjacent tap points on the bus: 0:upstream, 1:downstream
};

/**
 * Implements the bus which connects hosts, switches and other LAN entities on an Ethernet LAN.
 */
class INET_API EtherBus : public cSimpleModule
{
  public:
    EtherBus();
    virtual ~EtherBus();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage*);
    virtual void finish();

    // tokenize string containing space-separated numbers into the array
    void tokenize(const char *str, std::vector<double>& array);

  private:
    double  propagationSpeed;  // propagation speed of electrical signals through copper

    BusTap *tap;  // physical locations of where the hosts is connected to the bus
    int taps;     // number of tap points on the bus

    long numMessages;             // number of messages handled
};

Define_Module(EtherBus);

static cEnvir& operator<< (cEnvir& ev, cMessage *msg)
{
    ev.printf("(%s)%s",msg->className(),msg->fullName());
    return ev;
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
    taps = gate("in",0)->size();
    if (gate("out",0)->size()!=taps)
        error("the sizes of the in[] and out[] gate vectors must be the same");

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
    EV << "Parameters of (" << className() << ") " << fullPath() << "\n";
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
        send(autoconf,"out",i);
    }
}

void EtherBus::handleMessage (cMessage *msg)
{
    if (!msg->isSelfMessage())
    {
        // Handle frame sent down from the network entity
        int tapPoint = msg->arrivalGate()->index();
        EV << "Frame " << msg << " arrived on tap " << tapPoint << endl;

        // create upstream and downstream events
        if (tapPoint>0)
        {
            // start UPSTREAM travel
            cMessage *event = new cMessage("upstream", UPSTREAM);
            event->setContextPointer(&tap[tapPoint-1]);
            // if goes downstream too, we need to make a copy
            cMessage *msg2 = (tapPoint<taps-1) ? (cMessage *)msg->dup() : msg;
            event->encapsulate(msg2);
            scheduleAt(simTime()+tap[tapPoint].propagationDelay[UPSTREAM], event);
        }
        if (tapPoint<taps-1)
        {
            // start DOWNSTREAM travel
            cMessage *event = new cMessage("downstream", DOWNSTREAM);
            event->setContextPointer(&tap[tapPoint+1]);
            event->encapsulate(msg);
            scheduleAt(simTime()+tap[tapPoint].propagationDelay[DOWNSTREAM], event);
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
        int direction = msg->kind();
        BusTap *thistap = (BusTap *) msg->contextPointer();
        int tapPoint = thistap->id;

        EV << "Event " << msg << " on tap " << tapPoint << ", sending out frame\n";

        // send out on gate
        bool isLast = (direction==UPSTREAM) ? (tapPoint==0) : (tapPoint==taps-1);
        cMessage *msg2 = isLast ? msg->decapsulate() : (cMessage *)msg->encapsulatedMsg()->dup();
        send(msg2, "out", tapPoint);

        // if not end of the bus, schedule for next tap
        if (isLast)
        {
            EV << "End of bus reached\n";
            delete msg;
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
    if (par("writeScalars").boolValue())
    {
        double t = simTime();
        recordScalar("simulated time", t);
        recordScalar("messages handled", numMessages);
        if (t>0)
            recordScalar("messages/sec", numMessages/t);
    }
}
