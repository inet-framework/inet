/*
	file: DLStub.cc
	Purpose: Test thingie for Data link stub
		doesn't do much
	author: me
*/

#include <omnetpp.h>
#include "IPDatagram.h"

class DLStub: public cSimpleModule
{
public:
    Module_Class_Members(DLStub, cSimpleModule, 0);

    void handleMessage(cMessage *);
};

Define_Module( DLStub );

// catches IP datagrams and prints out some info
void DLStub::handleMessage(cMessage *msg)
{
    IPDatagram *d = (IPDatagram *)msg;
    simtime_t arrivalTime = d->arrivalTime();
    int length = d->totalLength();
    int outputPort = d->outputPort();
    char src[20], dest[20];

    strcpy (src, d->srcAddress());
    strcpy (dest, d->destAddress());

    ev << "*** Datagram arrived at DLStub:\n"
        << " Time:" << arrivalTime
        << " Port: " << outputPort
        << " Len: " << length
        << " Src: " << src
        << " Dest: " << dest << "\n";

	delete(d);
}

