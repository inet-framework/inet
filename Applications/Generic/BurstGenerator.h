/*
    file: BurstGenerator.h
    Purpose: Traffic Generator that sends out one big burst of packets
        all at once (time = 1.)
    author: Jochen Reber
*/

#ifndef __BURSTGENERATOR_H__
#define __BURSTGENERATOR_H__

#include <omnetpp.h>

#include "basic_consts.h"
#include "IPControlInfo_m.h"


class BurstGenerator: public cSimpleModule
{
private:
    int burstSize;
    int packetSize;
    int destctr;
    bool usesTCPProt;
    IPAddress destAddress;
public:
    Module_Class_Members(BurstGenerator, cSimpleModule, ACTIVITY_STACK_SIZE);

    void initialize();
    void activity();
};

#endif
