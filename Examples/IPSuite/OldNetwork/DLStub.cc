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

    ev << "*** Datagram arrived at DLStub:\n"
        << " Time:" << d->arrivalTime()
        << " Port: " << d->outputPort()
        << " Len: " << d->length()/8
        << " Src: " << d->srcAddress().getString()
        << " Dest: " << d->destAddress().getString() << "\n";

    delete d;
}

