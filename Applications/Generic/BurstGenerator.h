/*
	file: BurstGenerator.h
	Purpose: Traffic Generator that sends out one big burst of packets
		all at once (time = 1.)
	author: Jochen Reber
*/

#ifndef __BURSTGENERATOR_H__
#define __BURSTGENERATOR_H__

#include <omnetpp.h>

#include "IPInterfacePacket.h"


class BurstGenerator: public cSimpleModule
{
private:
	int burstSize;
	int packetSize;
	char nodename[NODE_NAME_SIZE];
	int nodenr;
	int destctr;
	bool usesTCPProt;

	char *chooseDestAddr(char *);
public:
    Module_Class_Members(BurstGenerator, cSimpleModule, 
			ACTIVITY_STACK_SIZE);

	void initialize();
    void activity();
};

#endif
