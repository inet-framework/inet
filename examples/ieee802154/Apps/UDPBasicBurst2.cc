//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Málaga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "UDPBasicBurst2.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"


Define_Module(UDPBasicBurst2);

int UDPBasicBurst2::counter;

uint64_t UDPBasicBurst2::totalSend;
uint64_t UDPBasicBurst2::totalSendMtoM;
uint64_t UDPBasicBurst2::totalSendFtoF;
uint64_t UDPBasicBurst2::totalSendFtoM;
uint64_t UDPBasicBurst2::totalSendMtoF;
uint64_t UDPBasicBurst2::totalRec;
uint64_t UDPBasicBurst2::totalRecMtoM;
uint64_t UDPBasicBurst2::totalRecFtoF;
uint64_t UDPBasicBurst2::totalRecMtoF;
uint64_t UDPBasicBurst2::totalRecFtoM;
double UDPBasicBurst2::totalDelay;
double UDPBasicBurst2::totalDelayMtoM;
double UDPBasicBurst2::totalDelayFtoF;
double UDPBasicBurst2::totalDelayMtoF;
double UDPBasicBurst2::totalDelayFtoM;
bool UDPBasicBurst2::isResultWrite;


static bool selectFunctionName(cModule *mod, void *name)
{
    return strcmp(mod->getName(), (char *)name) == 0;
}

static bool selectFunction(cModule *mod, void *name)
{
    return strstr(mod->getName(), (char *)name) != NULL;
}

void UDPBasicBurst2::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    totalSend = 0;
    totalSendMtoM = 0;
    totalSendFtoF = 0;
    totalSendFtoM = 0;
    totalSendMtoF = 0;
    totalRec = 0;
    totalRecMtoM = 0;
    totalRecFtoF = 0;
    totalRecMtoF = 0;
    totalRecFtoM = 0;
    totalDelay = 0;
    totalDelayMtoM = 0;
    totalDelayFtoF = 0;
    totalDelayMtoF = 0;
    totalDelayFtoM = 0;
    isResultWrite = false;


    counter = 0;
    numSent = 0;
    numReceived = 0;
    numDeleted = 0;

    limitDelay = par("limitDelay");
    endSend = par("time_end");
    nextPkt = 0;
    timeBurst = 0;

    numSentFtoF = 0;
    numReceivedFtoF = 0;

    numSentFtoM = 0;
    numReceivedFtoM = 0;

    numSentMtoF = 0;
    numReceivedMtoF = 0;

    numSentMtoM = 0;
    numReceivedMtoM = 0;

    randGenerator = par("rand_generator");

    WATCH(numSent);
    WATCH(numReceived);
    WATCH(numDeleted);

    localPort = par("localPort");
    destPort = par("destPort");

    if (localPort != -1)
        bindToPort(localPort);
    else
        bindToPort(destPort);

    msgByteLength = par("messageLength").longValue();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    const char *random_add;
    const char *fixNodeName = par("FixNodeName");

    double msgFrec = (double)par("messageFreq");
    if (msgFrec == -1)
        isSink = true;
    else
        isSink = false;

    offDisable = false;

    if ((double) par("time_off") == 0)
        offDisable = true;

    while ((token = tokenizer.nextToken()) != NULL)
    {
        if ((random_add = strstr(token, "random")) != NULL)
        {
            const char *leftparenp = strchr(random_add, '(');
            const char *rightparenp = strchr(random_add, ')');
            std::string nodetype;
            nodetype.assign(leftparenp + 1, rightparenp - leftparenp - 1);

            // find module and check protocol
            cTopology topo;
            if ((random_add = strstr(token, "random_name")) != NULL)
            {
                char name[30];
                strcpy(name, nodetype.c_str());

                if ((random_add = strstr(token, "random_nameExact")) != NULL)
                    topo.extractFromNetwork(selectFunctionName, name);
                else
                    topo.extractFromNetwork(selectFunction, name);

                for (int i = 0; i < topo.getNumNodes(); i++)
                {
                    cTopology::Node *node = topo.getNode(i);
                    if (strstr(this->getFullPath().c_str(), node->getModule()->getFullPath().c_str()) == NULL)
                    {
                        destAddresses.push_back(IPvXAddressResolver().resolve(node->getModule()->getFullPath().c_str()));

                        if (strstr(node->getModule()->getFullPath().c_str(), fixNodeName) != NULL)
                            destName.push_back(true);
                        else
                            destName.push_back(false);
                    }
                }
            }
            else
            {
                // topo.extractByModuleType(nodetype.c_str(), NULL);
                topo.extractByNedTypeName(cStringTokenizer(nodetype.c_str()).asVector());

                for (int i = 0; i < topo.getNumNodes(); i++)
                {
                    cTopology::Node *node = topo.getNode(i);
                    if (strstr(this->getFullPath().c_str(), node->getModule()->getFullPath().c_str()) == NULL)
                    {
                        destAddresses.push_back(IPvXAddressResolver().resolve(node->getModule()->getFullPath().c_str()));

                        if (strstr(node->getModule()->getFullPath().c_str(), fixNodeName) != NULL)
                            destName.push_back(true);
                        else
                            destName.push_back(false);
                    }
                }
            }
        }
        else if (strstr(token, "Broadcast") != NULL)
            destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
        else
        {
            destAddresses.push_back(IPvXAddressResolver().resolve(token));
            if (strstr(token, fixNodeName) != NULL)
                destName.push_back(true);
            else
                destName.push_back(false);
        }
    }
    pktDelayMtoM = new cStdDev("burst pkt delay M to M");
    pktDelayMtoF = new cStdDev("burst pkt delay M to F");
    pktDelayFtoF = new cStdDev("burst pkt delay F to F");
    pktDelayFtoM = new cStdDev("burst pkt delay F to M");

    if (strstr(this->getFullPath().c_str(), fixNodeName) != NULL)
        fixName = true;
    else
        fixName = false;

    if (destAddresses.empty())
    {
        isSink = true;
        return;
    }

    destAddr.set("0.0.0.0");

    activeBurst = par("activeBurst");
    if (!activeBurst) // new burst
    {
        destAddr = chooseDestAddr(toFix);
    }

    if ((double)par("time_begin") == -1)
        scheduleAt(0, &timerNext);
    else
        scheduleAt(par("time_begin"), &timerNext);
}

IPvXAddress UDPBasicBurst2::chooseDestAddr(bool &fix)
{
    // int k = intrand(destAddresses.size());
    if (!destAddr.isUnspecified() && par("fixedDestination"))
        return destAddr;

    int k = genk_intrand(randGenerator, destAddresses.size());
    fix = destName[k];
    return destAddresses[k];
}


cPacket *UDPBasicBurst2::createPacket()
{
    char msgName[32];
    sprintf(msgName, "UDPBasicAppData-%d", counter++);
    msgByteLength = par("messageLength").longValue();
    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(msgByteLength);
    payload->addPar("sourceId") = getId();
    payload->addPar("msgId") = numSent;
    payload->addPar("nodeType") = fixName;
    return payload;
}

void UDPBasicBurst2::sendPacket()
{
    cPacket *payload = createPacket();
    IPvXAddress destAddr = chooseDestAddr(toFix);
    sendToUDP(payload, localPort, destAddr, destPort);
    numSent++;
}

void UDPBasicBurst2::sendToUDPDelayed(cPacket *msg, int srcPort, const IPvXAddress& destAddr,
                                      int destPort, double delay)
{
    // send message to UDP, with the appropriate control info attached
    msg->setKind(UDP_C_DATA);

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSrcPort(srcPort);
    ctrl->setDestAddr(destAddr);
    ctrl->setDestPort(destPort);
    msg->setControlInfo(ctrl);
    msg->setTimestamp(delay);

    EV << "Sending packet: ";
    printPacket(msg);

    sendDelayed (msg,delay-simTime(), "udpOut");
}

void UDPBasicBurst2::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if ((endSend == 0) || (simTime() < endSend))
        {
            // send and reschedule next sending
            if (!isSink) //if the node is sink it don't generate messages
                generateBurst();
        }
    }
    else
    {
        // process incoming packet
        processPacket(PK(msg));
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}


void UDPBasicBurst2::processPacket(cPacket *msg)
{
    if (msg->getKind() == UDP_I_ERROR)
    {
        delete msg;
        return;
    }

    if (msg->hasPar("sourceId"))
    {
        // duplicate control
        int moduleId = (int)msg->par("sourceId");
        int msgId = (int)msg->par("msgId");
        SurceSequence::iterator i;
        i = sourceSequence.find(moduleId);

        if (i != sourceSequence.end())
        {
            if (i->second >= msgId)
            {
                EV << "Duplicated packet: ";
                printPacket(msg);
                delete msg;
                numDeleted++;
                return;
            }
            else
                i->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;

    }
    if (limitDelay >= 0)
    {
        if (simTime() - msg->getTimestamp() > limitDelay)
        {
            EV << "Old packet: ";
            printPacket(msg);
            delete msg;
            numDeleted++;
            return;
        }
    }

    EV << "Received packet: ";
    printPacket(msg);
    numReceived++;
    totalRec++;

    bool sourceFix = msg->par("nodeType").boolValue();

    double delay = SIMTIME_DBL(simTime() - msg->getTimestamp());
    totalDelay += delay;

    if (fixName && sourceFix)
    {
        pktDelayFtoF->collect(delay);
        numReceivedFtoF++;
        totalRecFtoF++;
        totalDelayFtoF += delay;
    }
    else if (!fixName && !sourceFix)
    {
        pktDelayMtoM ->collect(delay);
        numReceivedMtoM++;
        totalRecMtoM++;
        totalDelayMtoM += delay;
    }
    else if (fixName && !sourceFix)
    {
        pktDelayMtoF->collect(delay);
        numReceivedMtoF++;
        totalRecMtoF++;
        totalDelayMtoF += delay;
    }
    else if (!fixName && sourceFix)
    {
        pktDelayFtoM->collect(delay);
        numReceivedFtoM++;
        totalRecFtoM++;
        totalDelayFtoM += delay;
    }
    else
    {
        error(" no type recognized");
    }
//    meanDelay += (msg->getTimestamp()-simTime());
    delete msg;
}

void UDPBasicBurst2::generateBurst()
{
    simtime_t pkt_time;
    simtime_t now = simTime();

    if (timeBurst < now && activeBurst) // new burst
    {
        timeBurst = now + par("burstDuration");
        destAddr = chooseDestAddr(toFix);
    }

    if (nextPkt < now)
    {
        nextPkt = now;
    }

    cPacket *payload = createPacket();
    payload->setTimestamp();
    sendToUDP(payload, localPort, destAddr, destPort);

    totalSend++;
    numSent++;
    if (fixName && toFix)
    {
        numSentFtoF++;
        totalSendFtoF++;
    }
    else if (!fixName && !toFix)
    {
        numSentMtoM++;
        totalSendMtoM++;
    }
    else if (!fixName && toFix)
    {
        numSentMtoF++;
        totalSendMtoF++;
    }
    else if (fixName && !toFix)
    {
        numSentFtoM++;
        totalSendFtoM++;
    }
    else
    {
        error(" no type recognized");
    }

    // Next pkt
    nextPkt +=  par("messageFreq");
    if (nextPkt > timeBurst && activeBurst)
    {
        if (!offDisable)
        {
            pkt_time = now + par("time_off");

            if (pkt_time > nextPkt)
                nextPkt = pkt_time;
        }
    }

    pkt_time = nextPkt + par("message_freq_jitter");

    if (pkt_time < now)
    {
        opp_error("UDPBasicBurst bad parameters: next pkt time in the past ");
    }

    scheduleAt(pkt_time, &timerNext);
}

void UDPBasicBurst2::finish()
{
    simtime_t t = simTime();

    if (t == 0)
        return;

    if (!isResultWrite)
    {
        isResultWrite = true;
        recordScalar("Global Total send", totalSend);
        recordScalar("Global Total send FtoF", totalSendFtoF);
        recordScalar("Global send MtoM", totalSendMtoM);
        recordScalar("Global Total send FtoM", totalSendFtoM);
        recordScalar("Global Total send MtoF", totalSendMtoF);
        recordScalar("Global Total rec", totalRec);
        recordScalar("Global Total rec FtoF", totalRecFtoF);
        recordScalar("Global Total rec MtoM", totalRecMtoM);
        recordScalar("Global Total rec FtoM", totalRecFtoM);
        recordScalar("Global Total rec MtoF", totalRecMtoF);
        recordScalar("Global delay", totalDelay / totalRec);
        recordScalar("Global delay FtoF", totalDelayFtoF / totalRecFtoF);
        recordScalar("Global delay MtoM", totalDelayMtoM / totalRecMtoM);
        recordScalar("Global delay FtoM", totalDelayFtoM / totalRecFtoM);
        recordScalar("Global delay MtoF", totalDelayMtoF / totalRecMtoF);
    }

    recordScalar("Total send", numSent);
    recordScalar("Total received", numReceived);

// F to F
    recordScalar("Total send FtoF", numSentFtoF);
    recordScalar("Total received FtoF", numReceivedFtoF);
//    recordScalar("Mean delay", meanDelay/numReceived);
    recordScalar("Mean delay FtoF", pktDelayFtoF->getMean());
    recordScalar("Min delay FtoF", pktDelayFtoF->getMin());
    recordScalar("Max delay FtoF", pktDelayFtoF->getMax());
    recordScalar("Deviation delay FtoF", pktDelayFtoF->getStddev());

// M to M
    recordScalar("Total send MtoM", numSentMtoM);
    recordScalar("Total received MtoM", numReceivedMtoM);
//    recordScalar("Mean delay", meanDelay/numReceived);
    recordScalar("Mean delay MtoM", pktDelayMtoM->getMean());
    recordScalar("Min delay MtoM", pktDelayMtoM->getMin());
    recordScalar("Max delay MtoM", pktDelayMtoM->getMax());
    recordScalar("Deviation delay MtoM", pktDelayMtoM->getStddev());

// F to M
    recordScalar("Total send FtoM", numSentFtoM);
    recordScalar("Total received FtoM", numReceivedFtoM);
//    recordScalar("Mean delay", meanDelay/numReceived);
    recordScalar("Mean delay FtoM", pktDelayFtoM->getMean());
    recordScalar("Min delay FtoM", pktDelayFtoM->getMin());
    recordScalar("Max delay FtoM", pktDelayFtoM->getMax());
    recordScalar("Deviation delay FtoM", pktDelayFtoM->getStddev());

// M to F
    recordScalar("Total send MtoF", numSentMtoF);
    recordScalar("Total received MtoF", numReceivedMtoF);
//    recordScalar("Mean delay", meanDelay/numReceived);
    recordScalar("Mean delay MtoF", pktDelayMtoF->getMean());
    recordScalar("Min delay MtoF", pktDelayMtoF->getMin());
    recordScalar("Max delay MtoF", pktDelayMtoF->getMax());
    recordScalar("Deviation delay MtoF", pktDelayMtoF->getStddev());

    delete pktDelayFtoF;
    delete pktDelayFtoM;
    delete pktDelayMtoM;
    delete pktDelayMtoF;

    pktDelayFtoF = NULL;
    pktDelayFtoM = NULL;
    pktDelayMtoM = NULL;
    pktDelayMtoF = NULL;

}

UDPBasicBurst2::~UDPBasicBurst2()
{

    if (pktDelayFtoF) delete pktDelayFtoF;
    if (pktDelayFtoM) delete pktDelayFtoM;
    if (pktDelayMtoM) delete pktDelayMtoM;
    if (pktDelayMtoF) delete pktDelayMtoF;

    if (timerNext.isScheduled())
        cancelEvent(&timerNext);
}
