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

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "utils.h"


/**
 * Simple traffic sink to test the Ethernet models.
 */
class EtherTrafSink : public cSimpleModule
{
  protected:
    long packetsReceived; // number of packets received
    simtime_t lastEED;    // end-to-end delay
    cOutVector endToEndDelay;

  public:
    Module_Class_Members(EtherTrafSink,cSimpleModule,0);

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

Define_Module (EtherTrafSink);

void EtherTrafSink::initialize()
{
    endToEndDelay.setName("End-To-End Delay");
    lastEED = 0;
    packetsReceived = 0;
    WATCH(packetsReceived);
    WATCH(lastEED);
}

void EtherTrafSink::handleMessage(cMessage *msg)
{
    EV << "Received packet `" << msg->name() << "'\n";

    packetsReceived++;
    lastEED = simTime() - msg->creationTime();
    endToEndDelay.record(lastEED);

    delete msg;
}

void EtherTrafSink::finish()
{
}


