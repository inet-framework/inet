//
// Copyright (C) 2006 Andras Varga and Levente Meszaros
// Copyright (C) 2009 Lukáš Hlůže   lukas@hluze.cz (802.11e)
// Copyright (C) 2011 Alfonso Ariza  (clean code, fix some errors, new radio model)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "Ieee80211Mac.h"
#include "RadioState.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "PhyControlInfo_m.h"
#include "AirFrame_m.h"
#include "Radio80211aControlInfo_m.h"
#include "Ieee80211eClassifier.h"
#include "Ieee80211DataRate.h"

// TODO: 9.3.2.1, If there are buffered multicast or broadcast frames, the PC shall transmit these prior to any unicast frames.
// TODO: control frames must send before

Define_Module(Ieee80211Mac);

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Ieee80211Mac,
              (Ieee80211Mac::IDLE,
               Ieee80211Mac::DEFER,
               Ieee80211Mac::WAITAIFS,
               Ieee80211Mac::BACKOFF,
               Ieee80211Mac::WAITACK,
               Ieee80211Mac::WAITMULTICAST,
               Ieee80211Mac::WAITCTS,
               Ieee80211Mac::WAITSIFS,
               Ieee80211Mac::RECEIVE));

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(RadioState,
              (RadioState::IDLE,
               RadioState::RECV,
               RadioState::TRANSMIT,
               RadioState::SLEEP));

/****************************************************************
 * Construction functions.
 */

Ieee80211Mac::Ieee80211Mac()
{
    endSIFS = NULL;
    endDIFS = NULL;
    endTimeout = NULL;
    endReserve = NULL;
    mediumStateChange = NULL;
    pendingRadioConfigMsg = NULL;
    classifier = NULL;
}

Ieee80211Mac::~Ieee80211Mac()
{
    cancelAndDelete(endSIFS);
    cancelAndDelete(endDIFS);
    cancelAndDelete(endTimeout);
    cancelAndDelete(endReserve);
    cancelAndDelete(mediumStateChange);
    cancelAndDelete(endTXOP);
    for (unsigned int i = 0; i < edcCAF.size(); i++)
    {
        cancelAndDelete(endAIFS(i));
        cancelAndDelete(endBackoff(i));
        while (!transmissionQueue(i)->empty())
        {
            Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *> (transmissionQueue(i)->front());
            transmissionQueue(i)->pop_front();
            delete temp;
        }
    }
    edcCAF.clear();
    for (unsigned int i=0; i<edcCAFOutVector.size(); i++)
    {
        delete edcCAFOutVector[i].jitter;
        delete edcCAFOutVector[i].macDelay;
        delete edcCAFOutVector[i].throughput;
    }
    edcCAFOutVector.clear();
    if (pendingRadioConfigMsg)
        delete pendingRadioConfigMsg;
}

/****************************************************************
 * Initialization functions.
 */
void Ieee80211Mac::initialize(int stage)
{
    WirelessMacBase::initialize(stage);

    if (stage == 0)
    {
        EV << "Initializing stage 0\n";
        int numQueues = 1;
        if (par("EDCA"))
        {
            const char *classifierClass = par("classifier");
            classifier = check_and_cast<IQoSClassifier*>(createOne(classifierClass));
            numQueues = classifier->getNumQueues();
        }

        for (int i=0; i<numQueues; i++)
        {
            Edca catEdca;
            catEdca.backoff = false;
            catEdca.backoffPeriod = -1;
            catEdca.retryCounter = 0;
            edcCAF.push_back(catEdca);
        }
        // initialize parameters
        // Variable to apply the fsm fix
        fixFSM = par("fixFSM");
        const char *opModeStr = par("opMode").stringValue();
        if (strcmp("b", opModeStr)==0)
            opMode = 'b';
        else if (strcmp("g", opModeStr)==0)
            opMode = 'g';
        else if (strcmp("a", opModeStr)==0)
            opMode = 'a';
        else if (strcmp("p", opModeStr)==0)
             opMode = 'p';
        else
            throw cRuntimeError("Invalid opMode='%s'", opModeStr);

        PHY_HEADER_LENGTH = par("phyHeaderLength"); //26us

        if (strcmp("SHORT", par("wifiPreambleMode").stringValue())==0)
            wifiPreambleType = WIFI_PREAMBLE_SHORT;
        else if (strcmp("LONG", par("wifiPreambleMode").stringValue())==0)
            wifiPreambleType = WIFI_PREAMBLE_LONG;
        else
            opp_error("Invalid wifiPreambleType. Must be SHORT or LONG");

        useModulationParameters = par("useModulationParameters");

        prioritizeMulticast = par("prioritizeMulticast");

        EV<<"Operating mode: 802.11"<<opMode;
        maxQueueSize = par("maxQueueSize");
        rtsThreshold = par("rtsThresholdBytes");

        // the variable is renamed due to a confusion in the standard
        // the name retry limit would be misleading, see the header file comment
        transmissionLimit = par("retryLimit");
        if (transmissionLimit == -1) transmissionLimit = 7;
        ASSERT(transmissionLimit >= 0);

        EV<<" retryLimit="<<transmissionLimit;

        cwMinData = par("cwMinData");
        if (cwMinData == -1) cwMinData = CW_MIN;
        ASSERT(cwMinData >= 0 && cwMinData <= 32767);

        cwMaxData = par("cwMaxData");
        if (cwMaxData == -1) cwMaxData = CW_MAX;
        ASSERT(cwMaxData >= 0 && cwMaxData <= 32767);

        cwMinMulticast = par("cwMinMulticast");
        if (cwMinMulticast == -1) cwMinMulticast = 31;
        ASSERT(cwMinMulticast >= 0);
        EV<<" cwMinMulticast="<<cwMinMulticast;

        defaultAC = par("defaultAC");
        if (classifier && dynamic_cast<Ieee80211eClassifier*>(classifier))
            dynamic_cast<Ieee80211eClassifier*>(classifier)->setDefaultClass(defaultAC);

        for (int i=0; i<numCategories(); i++)
        {
            std::stringstream os;
            os<< i;
            std::string strAifs = "AIFSN"+os.str();
            std::string strTxop = "TXOP"+os.str();
            if (hasPar(strAifs.c_str()) && hasPar(strTxop.c_str()))
            {
                AIFSN(i) = par(strAifs.c_str());
                TXOP(i) = par(strTxop.c_str());
            }
            else
                opp_error("parameters %s , %s don't exist", strAifs.c_str(), strTxop.c_str());
        }
        if (numCategories()==1)
            AIFSN(0) = par("AIFSN");

        for (int i = 0; i < numCategories(); i++)
        {
            ASSERT(AIFSN(i) >= 0 && AIFSN(i) < 16);
            if (i == 0 || i == 1)
            {
                cwMin(i) = cwMinData;
                cwMax(i) = cwMaxData;
            }
            if (i == 2)
            {
                cwMin(i) = (cwMinData + 1) / 2 - 1;
                cwMax(i) = cwMinData;
            }
            if (i == 3)
            {
                cwMin(i) = (cwMinData + 1) / 4 - 1;
                cwMax(i) = (cwMinData + 1) / 2 - 1;
            }
        }

        ST = par("slotTime"); //added by sorin
        if (ST==-1)
            ST = 20e-6; //20us
        EV<<" slotTime="<<ST*1e6<<"us DIFS="<< getDIFS()*1e6<<"us";

        basicBitrate = par("basicBitrate");
        bitrate = par("bitrate");
        duplicateDetect = par("duplicateDetectionFilter");
        purgeOldTuples = par("purgeOldTuples");
        duplicateTimeOut = par("duplicateTimeOut");
        lastTimeDelete = 0;

        if (bitrate == -1)
        {
            rateIndex = Ieee80211Descriptor::getMaxIdx(opMode);
            bitrate = Ieee80211Descriptor::getDescriptor(rateIndex).bitrate;
        }
        else
            rateIndex = Ieee80211Descriptor::getIdx(opMode, bitrate);

        if (basicBitrate == -1)
        {
            int basicBitrateIdx = Ieee80211Descriptor::getMaxIdx(opMode);
            basicBitrate = Ieee80211Descriptor::getDescriptor(basicBitrateIdx).bitrate;
        }
        else
            Ieee80211Descriptor::getIdx(opMode, basicBitrate);

        EV<<" basicBitrate="<<basicBitrate/1e6<<"M";
        EV<<" bitrate="<<bitrate/1e6<<"M IDLE="<<IDLE<<" RECEIVE="<<RECEIVE<<endl;

        // configure AutoBit Rate
        configureAutoBitRate();
        //end auto rate code
        EV<<" bitrate="<<bitrate/1e6<<"M IDLE="<<IDLE<<" RECEIVE="<<RECEIVE<<endl;

        const char *addressString = par("address");
        address = isInterfaceRegistered();
        if (address.isUnspecified())
        {
            if (!strcmp(addressString, "auto"))
            {
                // assign automatic address
                address = MACAddress::generateAutoAddress();
                // change module parameter from "auto" to concrete address
                par("address").setStringValue(address.str().c_str());
            }
            else
                address.setAddress(addressString);
        }

        // subscribe for the information of the carrier sense
        nb->subscribe(this, NF_RADIOSTATE_CHANGED);

        // initialize self messages
        endSIFS = new cMessage("SIFS");
        endDIFS = new cMessage("DIFS");
        for (int i=0; i<numCategories(); i++)
        {
            setEndAIFS(i, new cMessage("AIFS", i));
            setEndBackoff(i, new cMessage("Backoff", i));
        }
        endTXOP = new cMessage("TXOP");
        endTimeout = new cMessage("Timeout");
        endReserve = new cMessage("Reserve");
        mediumStateChange = new cMessage("MediumStateChange");

        // interface
        if (isInterfaceRegistered().isUnspecified())
            registerInterface();

        // obtain pointer to external queue
        initializeQueueModule();

        // state variables
        fsm.setName("Ieee80211Mac State Machine");
        mode = DCF;
        sequenceNumber = 0;

        radioState = RadioState::IDLE;
        currentAC = 0;
        oldcurrentAC = 0;
        lastReceiveFailed = false;
        for (int i=0; i<numCategories(); i++)
            numDropped(i) = 0;
        nav = false;
        txop = false;
        last = 0;

        contI = 0;
        contJ = 0;
        recvdThroughput = 0;
        _snr = 0;
        samplingCoeff = 50;


        // statistics
        for (int i=0; i<numCategories(); i++)
        {
            numRetry(i) = 0;
            numSentWithoutRetry(i) = 0;
            numGivenUp(i) = 0;
            numSent(i) = 0;
            bites(i) = 0;
            maxjitter(i) = 0;
            minjitter(i) = 0;
        }

        numCollision = 0;
        numInternalCollision = 0;
        numReceived = 0;
        numSentMulticast = -1; //sorin
        numReceivedMulticast = 0;
        numBites = 0;
        numSentTXOP = 0;
        numReceivedOther = 0;
        numAckSend = 0;
        successCounter = 0;
        failedCounter = 0;
        recovery = 0;
        timer = 0;
        timeStampLastMessageReceived = 0;

        stateVector.setName("State");
        stateVector.setEnum("Ieee80211Mac");
        radioStateVector.setName("RadioState");
        radioStateVector.setEnum("RadioState");
        for (int i=0; i<numCategories(); i++)
        {
            EdcaOutVector outVectors;
            std::stringstream os;
            os<< i;
            std::string th = "throughput AC"+os.str();
            std::string delay = "Mac delay AC"+os.str();
            std::string jit = "jitter AC"+os.str();
            outVectors.jitter = new cOutVector(jit.c_str());
            outVectors.throughput = new cOutVector(th.c_str());
            outVectors.macDelay = new cOutVector(delay.c_str());
            edcCAFOutVector.push_back(outVectors);
        }
        // Code to compute the throughput over a period of time
        throughputTimePeriod = par("throughputTimePeriod");
        recBytesOverPeriod = 0;
        throughputLastPeriod = 0;
        throughputTimer = NULL;
        if (throughputTimePeriod>0)
            throughputTimer = new cMessage("throughput-timer");
        if (throughputTimer)
            scheduleAt(simTime()+throughputTimePeriod, throughputTimer);
        // end initialize variables throughput over a period of time
        // initialize watches
        validRecMode = false;
        initWatches();
        radioModule = gate("lowerLayerOut")->getNextGate()->getOwnerModule()->getId();
    }
}

void Ieee80211Mac::initWatches()
{
// initialize watches
     WATCH(fsm);
     WATCH(radioState);
     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].retryCounter);
     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].backoff);
     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].backoffPeriod);
     WATCH(currentAC);
     WATCH(oldcurrentAC);
     for (int i=0; i<numCategories(); i++)
         WATCH_LIST(edcCAF[i].transmissionQueue);
     WATCH(nav);
     WATCH(txop);

     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].numRetry);
     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].numSentWithoutRetry);
     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].numGivenUp);
     WATCH(numCollision);
     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].numSent);
     WATCH(numBites);
     WATCH(numSentTXOP);
     WATCH(numReceived);
     WATCH(numSentMulticast);
     WATCH(numReceivedMulticast);
     for (int i=0; i<numCategories(); i++)
         WATCH(edcCAF[i].numDropped);
     if (throughputTimer)
         WATCH(throughputLastPeriod);
}

void Ieee80211Mac::configureAutoBitRate()
{
    forceBitRate = par("forceBitRate");
    minSuccessThreshold = par("minSuccessThreshold");
    minTimerTimeout = par("minTimerTimeout");
    timerTimeout = par("timerTimeout");
    successThreshold = par("successThreshold");
    autoBitrate = par("autoBitrate");
    switch (autoBitrate)
    {
    case 0:
        rateControlMode = RATE_CR;
        EV<<"MAC Transmission algorithm : Constant Rate"  <<endl;
        break;
    case 1:
        rateControlMode = RATE_ARF;
        EV<<"MAC Transmission algorithm : ARF Rate"  <<endl;
        break;
    case 2:
        rateControlMode = RATE_AARF;
        successCoeff = par("successCoeff");
        timerCoeff = par("timerCoeff");
        maxSuccessThreshold = par("maxSuccessThreshold");
        EV<<"MAC Transmission algorithm : AARF Rate"  <<endl;
        break;
    default:
        throw cRuntimeError("Invalid autoBitrate parameter: '%d'", autoBitrate);
        break;
    }
}

void Ieee80211Mac::finish()
{
    recordScalar("number of received packets", numReceived);
    recordScalar("number of collisions", numCollision);
    recordScalar("number of internal collisions", numInternalCollision);
    for (int i=0; i<numCategories(); i++)
    {
        std::stringstream os;
        os<< i;
        std::string th = "number of retry for AC "+os.str();
        recordScalar(th.c_str(), numRetry(i));
    }
    recordScalar("sent and receive bites", numBites);
    for (int i=0; i<numCategories(); i++)
    {
        std::stringstream os;
        os<< i;
        std::string th = "sent packet within AC "+os.str();
        recordScalar(th.c_str(), numSent(i));
    }
    recordScalar("sent in TXOP ", numSentTXOP );
    for (int i=0; i<numCategories(); i++)
    {
        std::stringstream os;
        os<< i;
        std::string th = "sentWithoutRetry AC "+os.str();
        recordScalar(th.c_str(), numSentWithoutRetry(i));
    }
    for (int i=0; i<numCategories(); i++)
    {
        std::stringstream os;
        os<< i;
        std::string th = "numGivenUp AC "+os.str();
        recordScalar(th.c_str(), numGivenUp(i));
    }
    for (int i=0; i<numCategories(); i++)
    {
        std::stringstream os;
        os<< i;
        std::string th = "numDropped AC "+os.str();
        recordScalar(th.c_str(), numDropped(i));
    }
}

void Ieee80211Mac::registerInterface()
{
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;

    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NetworkInterface module's name without special characters ([])
    char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];
    char *d = interfaceName;
    for (const char *s = getParentModule()->getFullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->setName(interfaceName);
    delete [] interfaceName;

    // address
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // FIXME: MTU on 802.11 = ?
    e->setMtu(par("mtu").longValue());

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    ift->addInterface(e);
}

void Ieee80211Mac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (par("queueModule").stringValue()[0])
    {
        cModule *module = getParentModule()->getSubmodule(par("queueModule").stringValue());
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV << "Requesting first two frames from queue module\n";
        queueModule->requestPacket();
        // needed for backoff: mandatory if next message is already present
        queueModule->requestPacket();
    }
}

/****************************************************************
 * Message handling functions.
 */
void Ieee80211Mac::handleSelfMsg(cMessage *msg)
{
    if (msg==throughputTimer)
    {
        throughputLastPeriod = recBytesOverPeriod/SIMTIME_DBL(throughputTimePeriod);
        recBytesOverPeriod = 0;
        scheduleAt(simTime()+throughputTimePeriod, throughputTimer);
        return;
    }

    EV << "received self message: " << msg << "(kind: " << msg->getKind() << ")" << endl;

    if (msg == endReserve)
        nav = false;

    if (msg == endTXOP)
        txop = false;

    if ( !strcmp(msg->getName(), "AIFS") || !strcmp(msg->getName(), "Backoff") )
    {
        EV << "Changing currentAC to " << msg->getKind() << endl;
        currentAC = msg->getKind();
    }
    //check internal colision
    if ((strcmp(msg->getName(), "Backoff") == 0) || (strcmp(msg->getName(), "AIFS")==0))
    {
        int kind;
        kind = msg->getKind();
        if (kind<0)
            kind = 0;
        EV <<" kind is " << kind << ",name is " << msg->getName() <<endl;
        for (unsigned int i = numCategories()-1; (int)i > kind; i--)  //mozna prochaze jen 3..kind XXX
        {
            if (((endBackoff(i)->isScheduled() && endBackoff(i)->getArrivalTime() == simTime())
                    || (endAIFS(i)->isScheduled() && !backoff(i) && endAIFS(i)->getArrivalTime() == simTime()))
                    && !transmissionQueue(i)->empty())
            {
                EV << "Internal collision AC" << kind << " with AC" << i << endl;
                numInternalCollision++;
                EV << "Cancel backoff event and schedule new one for AC" << kind << endl;
                cancelEvent(endBackoff(kind));
                if (retryCounter() == transmissionLimit - 1)
                {
                    EV << "give up transmission for AC" << currentAC << endl;
                    giveUpCurrentTransmission();
                }
                else
                {
                    EV << "retry transmission for AC" << currentAC << endl;
                    retryCurrentTransmission();
                }
                return;
            }
        }
        currentAC = kind;
    }
    handleWithFSM(msg);
}


void Ieee80211Mac::handleUpperMsg(cPacket *msg)
{
    if (queueModule && numCategories()>1 && (int)transmissionQueueSize() < maxQueueSize)
    {
        // the module are continuously asking for packets, except if the queue is full
        EV << "requesting another frame from queue module\n";
        queueModule->requestPacket();
    }

    // check if it's a command from the mgmt layer
    if (msg->getBitLength()==0 && msg->getKind()!=0)
    {
        handleCommand(msg);
        return;
    }

    // must be a Ieee80211DataOrMgmtFrame, within the max size because we don't support fragmentation
    Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
    if (frame->getByteLength() > fragmentationThreshold)
        error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->getClassName(), msg->getName(), (int)(msg->getByteLength()));
    EV << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    // if you get error from this assert check if is client associated to AP
    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(address);
    frame->setSequenceNumber(sequenceNumber);
    sequenceNumber = (sequenceNumber+1) % 4096;  //XXX seqNum must be checked upon reception of frames!

    if (mappingAccessCategory(frame) == 200)
    {
        // if function mappingAccessCategory() returns 200, it means transsmissionQueue is full
        return;
    }
    frame->setMACArrive(simTime());
    handleWithFSM(frame);
}

int Ieee80211Mac::mappingAccessCategory(Ieee80211DataOrMgmtFrame *frame)
{
    bool isDataFrame = (dynamic_cast<Ieee80211DataFrame *>(frame) != NULL);

    currentAC = classifier ? classifier->classifyPacket(frame) : 0;

    // check for queue overflow
    if (isDataFrame && maxQueueSize && (int)transmissionQueueSize() >= maxQueueSize)
    {
        EV << "message " << frame << " received from higher layer but AC queue is full, dropping message\n";
        numDropped()++;
        delete frame;
        return 200;
    }
    if (isDataFrame)
    {
        if (!prioritizeMulticast  || !frame->getReceiverAddress().isMulticast() || transmissionQueue()->size() < 2)
            transmissionQueue()->push_back(frame);
        else
        {
            // if the last frame is management insert here
            Ieee80211DataFrame * frameAux = dynamic_cast<Ieee80211DataFrame *>(transmissionQueue()->back());
            if ((frameAux == NULL) || (frameAux && frameAux->getReceiverAddress().isMulticast()))
                transmissionQueue()->push_back(frame);
            else
            {
                // in other case search the possition
                std::list<Ieee80211DataOrMgmtFrame*>::iterator p = transmissionQueue()->end();
                while ((*p)->getReceiverAddress().isMulticast() && (p != transmissionQueue()->begin())) // search the first broadcast frame
                    {
                    if (dynamic_cast<Ieee80211DataFrame *>(*p) == NULL)
                        break;
                    p--;
                    }
                p++;
                transmissionQueue()->insert(p, frame);
            }
        }
    }
    else
    {
        if (transmissionQueue()->empty() || transmissionQueue()->size() == 1)
        {
            transmissionQueue()->push_back(frame);
        }
        else
        {
            std::list<Ieee80211DataOrMgmtFrame*>::iterator p;
            //we don't know if first frame in the queue is in middle of transmission
            //so for sure we placed it on second place
            p = transmissionQueue()->begin();
            p++;
            while ((dynamic_cast<Ieee80211DataFrame *> (*p) == NULL) && (p != transmissionQueue()->end())) // search the first not management frame
                p++;
            transmissionQueue()->insert(p, frame);
        }
    }
    EV << "frame classified as access category "<< currentAC <<" (0 background, 1 best effort, 2 video, 3 voice)\n";
    return true;
}

void Ieee80211Mac::handleCommand(cMessage *msg)
{
    if (msg->getKind()==PHY_C_CONFIGURERADIO)
    {
        EV << "Passing on command " << msg->getName() << " to physical layer\n";
        if (pendingRadioConfigMsg != NULL)
        {
            // merge contents of the old command into the new one, then delete it
            PhyControlInfo *pOld = check_and_cast<PhyControlInfo *>(pendingRadioConfigMsg->getControlInfo());
            PhyControlInfo *pNew = check_and_cast<PhyControlInfo *>(msg->getControlInfo());
            if (pNew->getChannelNumber()==-1 && pOld->getChannelNumber()!=-1)
                pNew->setChannelNumber(pOld->getChannelNumber());
            if (pNew->getBitrate()==-1 && pOld->getBitrate()!=-1)
                pNew->setBitrate(pOld->getBitrate());
            delete pendingRadioConfigMsg;
            pendingRadioConfigMsg = NULL;
        }

        if (fsm.getState() == IDLE || fsm.getState() == DEFER || fsm.getState() == BACKOFF)
        {
            EV << "Sending it down immediately\n";
/*
// Dynamic power
            PhyControlInfo *phyControlInfo = dynamic_cast<PhyControlInfo *>(msg->getControlInfo());
            if (phyControlInfo)
                phyControlInfo->setAdaptiveSensitivity(true);
// end dynamic power
*/
            sendDown(msg);
        }
        else
        {
            EV << "Delaying " << msg->getName() << " until next IDLE or DEFER state\n";
            pendingRadioConfigMsg = msg;
        }
    }
    else
    {
        error("Unrecognized command from mgmt layer: (%s)%s msgkind=%d", msg->getClassName(), msg->getName(), msg->getKind());
    }
}

void Ieee80211Mac::handleLowerMsg(cPacket *msg)
{
    EV<<"->Enter handleLowerMsg...\n";
    EV << "received message from lower layer: " << msg << endl;
    Radio80211aControlInfo * cinfo = dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo());
    if (cinfo && cinfo->getAirtimeMetric())
    {
        double rtsTime = 0;
        if (rtsThreshold*8<cinfo->getTestFrameSize())
             rtsTime=  computeFrameDuration(LENGTH_CTS, basicBitrate) +computeFrameDuration(LENGTH_RTS, basicBitrate);
        double frameDuration = cinfo->getTestFrameDuration() + computeFrameDuration(LENGTH_ACK, basicBitrate)+rtsTime;
        cinfo->setTestFrameDuration(frameDuration);
    }
    nb->fireChangeNotification(NF_LINK_FULL_PROMISCUOUS, msg);
    validRecMode = false;
    if (msg->getControlInfo() && dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo()))
    {
        Radio80211aControlInfo *cinfo = dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo());
        recFrameModulationType = cinfo->getModulationType();
        if (recFrameModulationType.getDataRate()>0)
            validRecMode = true;
    }

    if (rateControlMode == RATE_CR)
    {
        if (msg->getControlInfo())
            delete msg->removeControlInfo();
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame *>(msg);

    if (msg->getControlInfo() && dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo()))
    {
        Radio80211aControlInfo *cinfo = (Radio80211aControlInfo*) msg->removeControlInfo();
        if (contJ%10==0)
        {
            snr = _snr;
            contJ = 0;
            _snr = 0;
        }
        contJ++;
        _snr += cinfo->getSnr()/10;
        lossRate = cinfo->getLossRate();
        delete cinfo;
    }

    if (contI%samplingCoeff==0)
    {
        contI = 0;
        recvdThroughput = 0;
    }
    contI++;

    frame = dynamic_cast<Ieee80211Frame *>(msg);
    if (timeStampLastMessageReceived == 0)
        timeStampLastMessageReceived = simTime();
    else
    {
        if (frame)
            recvdThroughput += ((frame->getBitLength()/(simTime()-timeStampLastMessageReceived))/1000000)/samplingCoeff;
        timeStampLastMessageReceived = simTime();
    }
    if (frame && throughputTimer)
        recBytesOverPeriod += frame->getByteLength();


    if (!frame)
    {
        EV << "message from physical layer (%s)%s is not a subclass of Ieee80211Frame" << msg->getClassName() << " " << msg->getName() <<  endl;
        delete msg;
        return;
        // error("message from physical layer (%s)%s is not a subclass of Ieee80211Frame",msg->getClassName(), msg->getName());
    }

    EV << "Self address: " << address
    << ", receiver address: " << frame->getReceiverAddress()
    << ", received frame is for us: " << isForUs(frame)
    << ", received frame was sent by us: " << isSentByUs(frame)<<endl;

    Ieee80211TwoAddressFrame *twoAddressFrame = dynamic_cast<Ieee80211TwoAddressFrame *>(msg);
    ASSERT(!twoAddressFrame || twoAddressFrame->getTransmitterAddress() != address);

    handleWithFSM(msg);

    // if we are the owner then we did not send this message up
    if (msg->getOwner() == this)
        delete msg;
    EV<<"Leave handleLowerMsg...\n";
}

void Ieee80211Mac::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category == NF_RADIOSTATE_CHANGED)
    {
        const RadioState * rstate = check_and_cast<const RadioState *>(details);
        if (rstate->getRadioId()!=getRadioModuleId())
            return;

        RadioState::State newRadioState = rstate->getState();



        // FIXME: double recording, because there's no sample hold in the gui
        radioStateVector.record(radioState);
        radioStateVector.record(newRadioState);

        radioState = newRadioState;

        handleWithFSM(mediumStateChange);
    }
}

/**
 * Msg can be upper, lower, self or NULL (when radio state changes)
 */
void Ieee80211Mac::handleWithFSM(cMessage *msg)
{
    removeOldTuplesFromDuplicateMap();
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (isUpperMsg(msg) && fsm.getState() != IDLE)
    {
        if (fsm.getState() == WAITAIFS && endDIFS->isScheduled())
        {
            // a difs was schedule because all queues ware empty
            // change difs for aifs
            simtime_t remaint = getAIFS(currentAC)-getDIFS();
            scheduleAt(endDIFS->getArrivalTime()+remaint, endAIFS(currentAC));
            cancelEvent(endDIFS);
        }
        else if (fsm.getState() == BACKOFF && endBackoff(numCategories()-1)->isScheduled() &&  transmissionQueue(numCategories()-1)->empty())
        {
            // a backoff was schedule with all the queues empty
            // reschedule the backoff with the appropriate AC
            backoffPeriod(currentAC) = backoffPeriod(numCategories()-1);
            backoff(currentAC) = backoff(numCategories()-1);
            backoff(numCategories()-1) = false;
            scheduleAt(endBackoff(numCategories()-1)->getArrivalTime(), endBackoff(currentAC));
            cancelEvent(endBackoff(numCategories()-1));
        }
        EV << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
        return;
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame*>(msg);
    int frameType = frame ? frame->getType() : -1;
    int msgKind = msg->getKind();
    logState();
    stateVector.record(fsm.getState());

    if (frame && isLowerMsg(frame))
    {
        lastReceiveFailed = (msgKind == COLLISION || msgKind == BITERROR);
        scheduleReservePeriod(frame);
    }

    // TODO: fix bug according to the message: [omnetpp] A possible bug in the Ieee80211's FSM.
    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            /*
            if (fixFSM)
            {
            FSMA_Event_Transition(Data-Ready,
                                  // isUpperMsg(msg),
                                  isUpperMsg(msg) && backoffPeriod[currentAC] > 0,
                                  DEFER,
                //ASSERT(isInvalidBackoffPeriod() || backoffPeriod == 0);
                //invalidateBackoffPeriod();
               ASSERT(false);

            );
            FSMA_No_Event_Transition(Immediate-Data-Ready,
                                     //!transmissionQueue.empty(),
                !transmissionQueueEmpty(),
                                     DEFER,
              //  invalidateBackoffPeriod();
                ASSERT(backoff[currentAC]);

            );
            }
            */
            FSMA_Event_Transition(Data-Ready,
                                  isUpperMsg(msg),
                                  DEFER,
                                  ASSERT(isInvalidBackoffPeriod() || backoffPeriod() == 0);
                                  invalidateBackoffPeriod();
                                 );
            FSMA_No_Event_Transition(Immediate-Data-Ready,
                                     !transmissionQueueEmpty(),
                                     DEFER,
                                     if (retryCounter() == 0) //  jesjones patch.  TODO: check this particular case, I haven't been sure about this particular case
                                        invalidateBackoffPeriod();
                                    );
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVE,
                                 );
        }
        FSMA_State(DEFER)
        {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Wait-AIFS,
                                  isMediumStateChange(msg) && isMediumFree(),
                                  WAITAIFS,
                                  ;);
            FSMA_No_Event_Transition(Immediate-Wait-AIFS,
                                     isMediumFree() || (!isBackoffPending()),
                                     WAITAIFS,
                                     ;);
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVE,
                                  ;);
        }
        FSMA_State(WAITAIFS)
        {
            FSMA_Enter(scheduleAIFSPeriod());

            FSMA_Event_Transition(EDCAF-Do-Nothing,
                                  isMsgAIFS(msg) && transmissionQueue()->empty(),
                                  WAITAIFS,
                                  ASSERT(0==1);
                                  ;);
            FSMA_Event_Transition(Immediate-Transmit-RTS,
                                  isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                                  && getCurrentTransmission()->getByteLength() >= rtsThreshold && !backoff(),
                                  WAITCTS,
                                  sendRTSFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                 );
            FSMA_Event_Transition(Immediate-Transmit-Multicast,
                                  isMsgAIFS(msg) && isMulticast(getCurrentTransmission()) && !backoff(),
                                  WAITMULTICAST,
                                  sendMulticastFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                 );
            FSMA_Event_Transition(Immediate-Transmit-Data,
                                  isMsgAIFS(msg) && !isMulticast(getCurrentTransmission()) && !backoff(),
                                  WAITACK,
                                  sendDataFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                 );
            /*FSMA_Event_Transition(AIFS-Over,
                                  isMsgAIFS(msg) && backoff[currentAC],
                                  BACKOFF,
                if (isInvalidBackoffPeriod())
                    generateBackoffPeriod();
            );*/
            FSMA_Event_Transition(AIFS-Over,
                                  isMsgAIFS(msg),
                                  BACKOFF,
                                  if (isInvalidBackoffPeriod())
                                  generateBackoffPeriod();
                                 );
            // end the difs and no other packet has been received
            FSMA_Event_Transition(DIFS-Over,
                                  msg == endDIFS && transmissionQueueEmpty(),
                                  BACKOFF,
                                  currentAC = numCategories()-1;
                                  if (isInvalidBackoffPeriod())
                                     generateBackoffPeriod();
                                   );
            FSMA_Event_Transition(DIFS-Over,
                                   msg == endDIFS,
                                   BACKOFF,
                                   for (int i=numCategories()-1; i>=0; i--)
                                   {
                                       if (!transmissionQueue(i)->empty())
                                       {
                                            currentAC = i;
                                       }
                                   }
                                   if (isInvalidBackoffPeriod())
                                      generateBackoffPeriod();
                                    );
            FSMA_Event_Transition(Busy,
                                  isMediumStateChange(msg) && !isMediumFree(),
                                  DEFER,
                                  for (int i=0; i<numCategories(); i++)
                                  {
                                      if (endAIFS(i)->isScheduled())
                                          backoff(i) = true;
                                  }
                                  if (endDIFS->isScheduled()) backoff(numCategories()-1) = true;
                                  cancelAIFSPeriod();
                                  );
            FSMA_No_Event_Transition(Immediate-Busy,
                                     !isMediumFree(),
                                     DEFER,
                                     for (int i=0; i<numCategories(); i++)
                                     {
                                         if (endAIFS(i)->isScheduled())
                                             backoff(i) = true;
                                     }
                                     if (endDIFS->isScheduled()) backoff(numCategories()-1) = true;
                                     cancelAIFSPeriod();

                                     );
            // radio state changes before we actually get the message, so this must be here
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVE,
                                  cancelAIFSPeriod();
                                  ;);
        }
        FSMA_State(BACKOFF)
        {
            FSMA_Enter(scheduleBackoffPeriod());
            if (getCurrentTransmission())
            {
                FSMA_Event_Transition(Transmit-RTS,
                                      msg == endBackoff() && !isMulticast(getCurrentTransmission())
                                      && getCurrentTransmission()->getByteLength() >= rtsThreshold,
                                      WAITCTS,
                                      sendRTSFrame(getCurrentTransmission());
                                      oldcurrentAC = currentAC;
                                      cancelAIFSPeriod();
                                      decreaseBackoffPeriod();
                                      cancelBackoffPeriod();
                                     );
                FSMA_Event_Transition(Transmit-Multicast,
                                      msg == endBackoff() && isMulticast(getCurrentTransmission()),
                                      WAITMULTICAST,
                                      sendMulticastFrame(getCurrentTransmission());
                                      oldcurrentAC = currentAC;
                                      cancelAIFSPeriod();
                                      decreaseBackoffPeriod();
                                      cancelBackoffPeriod();
                                     );
                FSMA_Event_Transition(Transmit-Data,
                                      msg == endBackoff() && !isMulticast(getCurrentTransmission()),
                                      WAITACK,
                                      sendDataFrame(getCurrentTransmission());
                                      oldcurrentAC = currentAC;
                                      cancelAIFSPeriod();
                                      decreaseBackoffPeriod();
                                      cancelBackoffPeriod();
                                     );
            }
            FSMA_Event_Transition(AIFS-Over-backoff,
                                  isMsgAIFS(msg) && backoff(),
                                  BACKOFF,
                                  if (isInvalidBackoffPeriod())
                                  generateBackoffPeriod();
                                 );
            FSMA_Event_Transition(AIFS-Immediate-Transmit-RTS,
                                  isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                                  && getCurrentTransmission()->getByteLength() >= rtsThreshold && !backoff(),
                                  WAITCTS,
                                  sendRTSFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );
            FSMA_Event_Transition(AIFS-Immediate-Transmit-Multicast,
                                  isMsgAIFS(msg) && isMulticast(getCurrentTransmission()) && !backoff(),
                                  WAITMULTICAST,
                                  sendMulticastFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );
            FSMA_Event_Transition(AIFS-Immediate-Transmit-Data,
                                  isMsgAIFS(msg) && !isMulticast(getCurrentTransmission()) && !backoff(),
                                  WAITACK,
                                  sendDataFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );
            FSMA_Event_Transition(Backoff-Idle,
                                  isBakoffMsg(msg) && transmissionQueueEmpty(),
                                  IDLE,
                                  resetStateVariables();
                                  );
            FSMA_Event_Transition(Backoff-Busy,
                                  isMediumStateChange(msg) && !isMediumFree(),
                                  DEFER,
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );

        }
        FSMA_State(WAITACK)
        {
            FSMA_Enter(scheduleDataTimeoutPeriod(getCurrentTransmission()));
            FSMA_Event_Transition(Receive-ACK-TXOP,
                                  isLowerMsg(msg) && isForUs(frame) && frameType == ST_ACK && txop,
                                  WAITSIFS,
                                  currentAC = oldcurrentAC;
                                  if (retryCounter() == 0) numSentWithoutRetry()++;
                                  numSent()++;
                                  fr = getCurrentTransmission();
                                  numBites += fr->getBitLength();
                                  bites() += fr->getBitLength();


                                  macDelay()->record(simTime() - fr->getMACArrive());
                                  if (maxjitter() == 0 || maxjitter() < (simTime() - fr->getMACArrive()))
                                      maxjitter() = simTime() - fr->getMACArrive();
                                  if (minjitter() == 0 || minjitter() > (simTime() - fr->getMACArrive()))
                                      minjitter() = simTime() - fr->getMACArrive();
                                  EV << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;
                                  numSentTXOP++;
                                  cancelTimeoutPeriod();
                                  finishCurrentTransmission();
                                  );
/*
            FSMA_Event_Transition(Receive-ACK,
                                  isLowerMsg(msg) && isForUs(frame) && frameType == ST_ACK,
                                  IDLE,
                                  currentAC=oldcurrentAC;
                                  if (retryCounter[currentAC] == 0) numSentWithoutRetry[currentAC]++;
                                  numSent[currentAC]++;
                                  fr=getCurrentTransmission();
                                  numBites += fr->getBitLength();
                                  bites[currentAC] += fr->getBitLength();

                                  macDelay[currentAC].record(simTime() - fr->getMACArrive());
                                  if (maxjitter[currentAC] == 0 || maxjitter[currentAC] < (simTime() - fr->getMACArrive())) maxjitter[currentAC]=simTime() - fr->getMACArrive();
                                      if (minjitter[currentAC] == 0 || minjitter[currentAC] > (simTime() - fr->getMACArrive())) minjitter[currentAC]=simTime() - fr->getMACArrive();
                                          EV << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;

                                          cancelTimeoutPeriod();
                                          finishCurrentTransmission();
                                         );

             */
             /*Ieee 802.11 2007 9.9.1.2 EDCA TXOPs*/
             FSMA_Event_Transition(Receive-ACK,
                                  isLowerMsg(msg) && isForUs(frame) && frameType == ST_ACK,
                                  DEFER,
                                  currentAC = oldcurrentAC;
                                  if (retryCounter() == 0)
                                      numSentWithoutRetry()++;
                                  numSent()++;
                                  fr = getCurrentTransmission();
                                  numBites += fr->getBitLength();
                                  bites() += fr->getBitLength();

                                  macDelay()->record(simTime() - fr->getMACArrive());
                                  if (maxjitter() == 0 || maxjitter() < (simTime() - fr->getMACArrive()))
                                      maxjitter() = simTime() - fr->getMACArrive();
                                  if (minjitter() == 0 || minjitter() > (simTime() - fr->getMACArrive()))
                                      minjitter() = simTime() - fr->getMACArrive();
                                  EV << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;
                                  cancelTimeoutPeriod();
                                  finishCurrentTransmission();
                                  resetCurrentBackOff();
                                  );
            FSMA_Event_Transition(Transmit-Data-Failed,
                                  msg == endTimeout && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                  IDLE,
                                  currentAC = oldcurrentAC;
                                  giveUpCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
            FSMA_Event_Transition(Receive-ACK-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                                  currentAC = oldcurrentAC;
                                  retryCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
            FSMA_Event_Transition(Interrupted-ACK-Failure,
                                  isLowerMsg(msg) && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                  RECEIVE,
                                  currentAC=oldcurrentAC;
                                  giveUpCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
            FSMA_Event_Transition(Retry-Interrupted-ACK,
                                 isLowerMsg(msg),
                                 RECEIVE,
                                 currentAC=oldcurrentAC;
                                 retryCurrentTransmission();
                                 txop = false;
                                 if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
        }
        // wait until multicast is sent
        FSMA_State(WAITMULTICAST)
        {
            FSMA_Enter(scheduleMulticastTimeoutPeriod(getCurrentTransmission()));
            /*
                        FSMA_Event_Transition(Transmit-Multicast,
                                              msg == endTimeout,
                                              IDLE,
                            currentAC=oldcurrentAC;
                            finishCurrentTransmission();
                            numSentMulticast++;
                        );
            */
            ///changed
            FSMA_Event_Transition(Transmit-Multicast,
                                  msg == endTimeout,
                                  DEFER,
                                  currentAC = oldcurrentAC;
                                  fr = getCurrentTransmission();
                                  numBites += fr->getBitLength();
                                  bites() += fr->getBitLength();
                                  finishCurrentTransmission();
                                  numSentMulticast++;
                                  resetCurrentBackOff();
                                 );
        }
        // accoriding to 9.2.5.7 CTS procedure
        FSMA_State(WAITCTS)
        {
            FSMA_Enter(scheduleCTSTimeoutPeriod());
            FSMA_Event_Transition(Receive-CTS,
                                  isLowerMsg(msg) && isForUs(frame) && frameType == ST_CTS,
                                  WAITSIFS,
                                  cancelTimeoutPeriod();
                                 );
            FSMA_Event_Transition(Transmit-RTS-Failed,
                                  msg == endTimeout && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                  IDLE,
                                  currentAC = oldcurrentAC;
                                  giveUpCurrentTransmission();
                                 );
            FSMA_Event_Transition(Receive-CTS-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                                  currentAC = oldcurrentAC;
                                  retryCurrentTransmission();
                                 );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Transmit-Data-TXOP,
                                  msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_ACK,
                                  WAITACK,
                                  sendDataFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                 );
            FSMA_Event_Transition(Transmit-CTS,
                                  msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_RTS,
                                  IDLE,
                                  sendCTSFrameOnEndSIFS();
                                  if (fixFSM)
                                      finishReception();
                                  else
                                      resetStateVariables();
                                  );
            FSMA_Event_Transition(Transmit-DATA,
                                  msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_CTS,
                                  WAITACK,
                                  sendDataFrameOnEndSIFS(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                 );
            FSMA_Event_Transition(Transmit-ACK,
                                  msg == endSIFS && isDataOrMgmtFrame(getFrameReceivedBeforeSIFS()),
                                  IDLE,
                                  sendACKFrameOnEndSIFS();
                                  if (fixFSM)
                                      finishReception();
                                  else
                                      resetStateVariables();
                                   );
        }
        // this is not a real state
        FSMA_State(RECEIVE)
        {
            FSMA_No_Event_Transition(Immediate-Receive-Error,
                                     isLowerMsg(msg) && (msgKind == COLLISION || msgKind == BITERROR),
                                     IDLE,
                                     EV << "received frame contains bit errors or collision, next wait period is EIFS\n";
                                     numCollision++;
                                     if (fixFSM)
                                         finishReception();
                                     else
                                         resetStateVariables();
                                     );
            FSMA_No_Event_Transition(Immediate-Receive-Multicast,
                                     isLowerMsg(msg) && isMulticast(frame) && !isSentByUs(frame) && isDataOrMgmtFrame(frame),
                                     IDLE,
                                     sendUp(frame);
                                     numReceivedMulticast++;
                                     if (fixFSM)
                                         finishReception();
                                     else
                                         resetStateVariables();
                                     );
            FSMA_No_Event_Transition(Immediate-Receive-Data,
                                     isLowerMsg(msg) && isForUs(frame) && isDataOrMgmtFrame(frame),
                                     WAITSIFS,
                                     sendUp(frame);
                                     numReceived++;
                                    );
            FSMA_No_Event_Transition(Immediate-Receive-RTS,
                                     isLowerMsg(msg) && isForUs(frame) && frameType == ST_RTS,
                                     WAITSIFS,
                                    );
            FSMA_No_Event_Transition(Immediate-Receive-Other-backtobackoff,
                                     isLowerMsg(msg) && isBackoffPending(), //(backoff[0] || backoff[1] || backoff[2] || backoff[3]),
                                     DEFER,
                                    );

            FSMA_No_Event_Transition(Immediate-Promiscuous-Data,
                                     isLowerMsg(msg) && !isForUs(frame) && isDataOrMgmtFrame(frame),
                                     IDLE,
                                     promiscousFrame(frame);
                                     if (fixFSM)
                                         finishReception();
                                     else
                                         resetStateVariables();
                                     numReceivedOther++;
                                     );
            FSMA_No_Event_Transition(Immediate-Receive-Other,
                                     isLowerMsg(msg),
                                     IDLE,
                                     if (fixFSM)
                                         finishReception();
                                     else
                                         resetStateVariables();
                                     numReceivedOther++;
                                     );
        }
    }
    EV<<"leaving handleWithFSM\n\t";
    logState();
    stateVector.record(fsm.getState());
    if (simTime() - last > 0.1)
    {
        for (int i = 0; i<numCategories(); i++)
        {
            throughput(i)->record(bites(i)/(simTime()-last));
            bites(i) = 0;
            if (maxjitter(i) > 0 && minjitter(i) > 0)
            {
                jitter(i)->record(maxjitter(i)-minjitter(i));
                maxjitter(i) = 0;
                minjitter(i) = 0;
            }
        }
        last = simTime();
    }
}

void Ieee80211Mac::finishReception()
{
    if (getCurrentTransmission())
    {
        backoff() = true;
    }
    else
    {
        resetStateVariables();
    }
}


/****************************************************************
 * Timing functions.
 */
simtime_t Ieee80211Mac::getSIFS()
{
// TODO:   return aRxRFDelay() + aRxPLCPDelay() + aMACProcessingDelay() + aRxTxTurnaroundTime();
    if (useModulationParameters)
    {
        ModulationType modType;
        modType = WifiModulationType::getModulationType(opMode, bitrate);
        return WifiModulationType::getSifsTime(modType,wifiPreambleType);
    }

    return SIFS;
}

simtime_t Ieee80211Mac::getSlotTime()
{
// TODO:   return aCCATime() + aRxTxTurnaroundTime + aAirPropagationTime() + aMACProcessingDelay();
    if (useModulationParameters)
    {
        ModulationType modType;
        modType = WifiModulationType::getModulationType(opMode, bitrate);
        return WifiModulationType::getSlotDuration(modType,wifiPreambleType);
    }
    return ST;
}

simtime_t Ieee80211Mac::getPIFS()
{
    return getSIFS() + getSlotTime();
}

simtime_t Ieee80211Mac::getDIFS(int category)
{
    if (category<0 || category>(numCategories()-1))
    {
        int index = numCategories()-1;
        if (index<0)
            index = 0;
        return getSIFS() + ((double)AIFSN(index) * getSlotTime());
    }
    else
    {
        return getSIFS() + ((double)AIFSN(category)) * getSlotTime();
    }

}

simtime_t Ieee80211Mac::getHeaderTime(double bitrate)
{
    ModulationType modType;
    modType = WifiModulationType::getModulationType(opMode, bitrate);
    return WifiModulationType::getPreambleAndHeader(modType, wifiPreambleType);
}

simtime_t Ieee80211Mac::getAIFS(int AccessCategory)
{
    return AIFSN(AccessCategory) * getSlotTime() + getSIFS();
}

simtime_t Ieee80211Mac::getEIFS()
{
// FIXME:   return getSIFS() + getDIFS() + (8 * ACKSize + aPreambleLength + aPLCPHeaderLength) / lowestDatarate;
    if (PHY_HEADER_LENGTH<0)
    {
        if ((opMode=='b') || (opMode=='g'))
            return getSIFS() + getDIFS() + (8 * LENGTH_ACK) / 1E+6 + getHeaderTime(1E+6);
        else if (opMode=='a')
            return getSIFS() + getDIFS() + (8 * LENGTH_ACK) / 6E+6 + getHeaderTime(6E+6);
        else if (opMode=='p')
             return getSIFS() + getDIFS() + (8 * LENGTH_ACK) / 6E+6 + getHeaderTime(3E+6);
    }
    else
    {
        // FIXME: check how PHY_HEADER_LENGTH is handled. Is that given in bytes or secs ???
        // what is the real unit? The use seems to be incosistent betwen b and g modes.
        if (opMode=='b')
            return getSIFS() + getDIFS() + (8 * LENGTH_ACK + PHY_HEADER_LENGTH) / 1E+6;
        else if (opMode=='g')
            return getSIFS() + getDIFS() + (8 * LENGTH_ACK) / 1E+6 + PHY_HEADER_LENGTH;
        else if (opMode=='a')
            return getSIFS() + getDIFS() + (8 * LENGTH_ACK) / 6E+6 + PHY_HEADER_LENGTH;
        else if (opMode=='p')
            return getSIFS() + getDIFS() + (8 * LENGTH_ACK) / 3E+6 + PHY_HEADER_LENGTH;
    }
    // if arrive here there is an error
    opp_error("mode not supported");
    return 0;
}

simtime_t Ieee80211Mac::computeBackoffPeriod(Ieee80211Frame *msg, int r)
{
    int cw;

    EV << "generating backoff slot number for retry: " << r << endl;
    if (msg && isMulticast(msg))
        cw = cwMinMulticast;
    else
    {
        ASSERT(0 <= r && r < transmissionLimit);

        cw = (cwMin() + 1) * (1 << r) - 1;

        if (cw > cwMax())
            cw = cwMax();
    }

    int c = intrand(cw + 1);

    EV << "generated backoff slot number: " << c << " , cw: " << cw << " ,cwMin:cwMax = " << cwMin() << ":" << cwMax() << endl;

    return ((double)c) * getSlotTime();
}

/****************************************************************
 * Timer functions.
 */
void Ieee80211Mac::scheduleSIFSPeriod(Ieee80211Frame *frame)
{
    EV << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + getSIFS(), endSIFS);
}

void Ieee80211Mac::scheduleDIFSPeriod()
{
    if (lastReceiveFailed)
    {
        EV << "reception of last frame failed, scheduling EIFS period\n";
        scheduleAt(simTime() + getEIFS(), endDIFS);
    }
    else
    {
        EV << "scheduling DIFS period\n";
        scheduleAt(simTime() + getDIFS(), endDIFS);
    }
}

void Ieee80211Mac::cancelDIFSPeriod()
{
    EV << "canceling DIFS period\n";
    cancelEvent(endDIFS);
}

void Ieee80211Mac::scheduleAIFSPeriod()
{
    bool schedule = false;
    for (int i = 0; i<numCategories(); i++)
    {
        if (!endAIFS(i)->isScheduled() && !transmissionQueue(i)->empty())
        {

            if (lastReceiveFailed)
            {
                EV << "reception of last frame failed, scheduling EIFS-DIFS+AIFS period (" << i << ")\n";
                scheduleAt(simTime() + getEIFS() - getDIFS() + getAIFS(i), endAIFS(i));
            }
            else
            {
                EV << "scheduling AIFS period (" << i << ")\n";
                scheduleAt(simTime() + getAIFS(i), endAIFS(i));
            }

        }
        if (endAIFS(i)->isScheduled())
            schedule = true;
    }
    if (!schedule && !endDIFS->isScheduled())
    {
        // schedule default DIFS
        currentAC = numCategories()-1;
        scheduleDIFSPeriod();
    }
}

void Ieee80211Mac::rescheduleAIFSPeriod(int AccessCategory)
{
    ASSERT(1);
    EV << "rescheduling AIFS[" << AccessCategory << "]\n";
    cancelEvent(endAIFS(AccessCategory));
    scheduleAt(simTime() + getAIFS(AccessCategory), endAIFS(AccessCategory));
}

void Ieee80211Mac::cancelAIFSPeriod()
{
    EV << "canceling AIFS period\n";
    for (int i = 0; i<numCategories(); i++)
        cancelEvent(endAIFS(i));
    cancelEvent(endDIFS);
}

//XXXvoid Ieee80211Mac::checkInternalColision()
//{
//  EV << "We obtain endAIFS, so we have to check if there
//}


void Ieee80211Mac::scheduleDataTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    double tim;
    double bitRate = bitrate;
    if (dynamic_cast<PhyControlInfo*> (frameToSend->getControlInfo()))
    {
        bitRate = dynamic_cast<PhyControlInfo*> (frameToSend->getControlInfo())->getBitrate();
        if (bitRate == 0)
            bitRate = bitrate;
    }
    if (!endTimeout->isScheduled())
    {
        EV << "scheduling data timeout period\n";
        if (useModulationParameters)
        {
            ModulationType modType;
            modType = WifiModulationType::getModulationType(opMode, bitRate);
            WifiModulationType::getSlotDuration(modType,wifiPreambleType);
            tim = computeFrameDuration(frameToSend) +SIMTIME_DBL(
                 WifiModulationType::getSlotDuration(modType,wifiPreambleType) +
                 WifiModulationType::getSifsTime(modType,wifiPreambleType) +
                 WifiModulationType::get_aPHY_RX_START_Delay (modType,wifiPreambleType));
        }
        else
            tim = computeFrameDuration(frameToSend) +SIMTIME_DBL( getSIFS()) + computeFrameDuration(LENGTH_ACK, basicBitrate) + MAX_PROPAGATION_DELAY * 2;
        EV<<" time out="<<tim*1e6<<"us"<<endl;
        scheduleAt(simTime() + tim, endTimeout);
    }
}

void Ieee80211Mac::scheduleMulticastTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    if (!endTimeout->isScheduled())
    {
        EV << "scheduling multicast timeout period\n";
        scheduleAt(simTime() + computeFrameDuration(frameToSend), endTimeout);
    }
}

void Ieee80211Mac::cancelTimeoutPeriod()
{
    EV << "canceling timeout period\n";
    cancelEvent(endTimeout);
}

void Ieee80211Mac::scheduleCTSTimeoutPeriod()
{
    if (!endTimeout->isScheduled())
    {
        EV << "scheduling CTS timeout period\n";
        scheduleAt(simTime() + computeFrameDuration(LENGTH_RTS, basicBitrate) + getSIFS()
                   + computeFrameDuration(LENGTH_CTS, basicBitrate) + MAX_PROPAGATION_DELAY * 2, endTimeout);
    }
}

void Ieee80211Mac::scheduleReservePeriod(Ieee80211Frame *frame)
{
    simtime_t reserve = frame->getDuration();

    // see spec. 7.1.3.2
    if (!isForUs(frame) && reserve != 0 && reserve < 32768)
    {
        if (endReserve->isScheduled())
        {
            simtime_t oldReserve = endReserve->getArrivalTime() - simTime();

            if (oldReserve > reserve)
                return;

            reserve = std::max(reserve, oldReserve);
            cancelEvent(endReserve);
        }
        else if (radioState == RadioState::IDLE)
        {
            // NAV: the channel just became virtually busy according to the spec
            scheduleAt(simTime(), mediumStateChange);
        }

        EV << "scheduling reserve period for: " << reserve << endl;

        ASSERT(reserve > 0);

        nav = true;
        scheduleAt(simTime() + reserve, endReserve);
    }
}

void Ieee80211Mac::invalidateBackoffPeriod()
{
    backoffPeriod() = -1;
}

bool Ieee80211Mac::isInvalidBackoffPeriod()
{
    return backoffPeriod() == -1;
}

void Ieee80211Mac::generateBackoffPeriod()
{
    backoffPeriod() = computeBackoffPeriod(getCurrentTransmission(), retryCounter());
    ASSERT(backoffPeriod() >= 0);
    EV << "backoff period set to " << backoffPeriod()<< endl;
}

void Ieee80211Mac::decreaseBackoffPeriod()
{
    // see spec 9.9.1.5
    // decrase for every EDCAF
    // cancel event endBackoff after decrease or we don't know which endBackoff is scheduled
    for (int i = 0; i<numCategories(); i++)
    {
        if (backoff(i) && endBackoff(i)->isScheduled())
        {
            EV<< "old backoff[" << i << "] is " << backoffPeriod(i) << ", sim time is " << simTime()
            << ", endbackoff sending period is " << endBackoff(i)->getSendingTime() << endl;
            simtime_t elapsedBackoffTime = simTime() - endBackoff(i)->getSendingTime();
            backoffPeriod(i) -= ((int)(elapsedBackoffTime / getSlotTime())) * getSlotTime();
            EV << "actual backoff[" << i << "] is " <<backoffPeriod(i) << ", elapsed is " << elapsedBackoffTime << endl;
            ASSERT(backoffPeriod(i) >= 0);
            EV << "backoff[" << i << "] period decreased to " << backoffPeriod(i) << endl;
        }
    }
}

void Ieee80211Mac::scheduleBackoffPeriod()
{
    EV << "scheduling backoff period\n";
    scheduleAt(simTime() + backoffPeriod(), endBackoff());
}

void Ieee80211Mac::cancelBackoffPeriod()
{
    EV << "cancelling Backoff period - only if some is scheduled\n";
    for (int i = 0; i<numCategories(); i++)
        cancelEvent(endBackoff(i));
}

/****************************************************************
 * Frame sender functions.
 */
void Ieee80211Mac::sendACKFrameOnEndSIFS()
{
    Ieee80211Frame *frameToACK = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendACKFrame(check_and_cast<Ieee80211DataOrMgmtFrame*>(frameToACK));
    delete frameToACK;
}

void Ieee80211Mac::sendACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    EV << "sending ACK frame\n";
    numAckSend++;
    sendDown(setBasicBitrate(buildACKFrame(frameToACK)));
}

void Ieee80211Mac::sendDataFrameOnEndSIFS(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211Frame *ctsFrame = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendDataFrame(frameToSend);
    delete ctsFrame;
}

void Ieee80211Mac::sendDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    simtime_t t = 0, time = 0;
    int count = 0;
    std::list<Ieee80211DataOrMgmtFrame*>::iterator frame;

    frame = transmissionQueue()->begin();
    ASSERT(*frame==frameToSend);
    if (!txop && TXOP() > 0 && transmissionQueue()->size() >= 2 )
    {
        //we start packet burst within TXOP time period
        txop = true;

        for (frame=transmissionQueue()->begin(); frame != transmissionQueue()->end(); ++frame)
        {
            count++;
            t = computeFrameDuration(*frame) + 2 * getSIFS() + computeFrameDuration(LENGTH_ACK, basicBitrate);
            EV << "t is " << t << endl;
            if (TXOP()>time+t)
            {
                time += t;
                EV << "adding t \n";
            }
            else
            {
                break;
            }
        }
        //to be sure we get endTXOP earlier then receive ACK and we have to minus SIFS time from first packet
        time -= getSIFS()/2 + getSIFS();
        EV << "scheduling TXOP for AC" << currentAC << ", duration is " << time << ",count is " << count << endl;
        scheduleAt(simTime() + time, endTXOP);
    }
    EV << "sending Data frame\n";
    sendDown(buildDataFrame(dynamic_cast<Ieee80211DataOrMgmtFrame*>(setBitrateFrame(frameToSend))));
}

void Ieee80211Mac::sendRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "sending RTS frame\n";
    sendDown(setBasicBitrate(buildRTSFrame(frameToSend)));
}

void Ieee80211Mac::sendMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "sending Multicast frame\n";
    if (frameToSend->getControlInfo())
        delete frameToSend->removeControlInfo();
    sendDown(buildDataFrame(dynamic_cast<Ieee80211DataOrMgmtFrame*>(setBasicBitrate(frameToSend))));
}

void Ieee80211Mac::sendCTSFrameOnEndSIFS()
{
    Ieee80211Frame *rtsFrame = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendCTSFrame(check_and_cast<Ieee80211RTSFrame*>(rtsFrame));
    delete rtsFrame;
}

void Ieee80211Mac::sendCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    EV << "sending CTS frame\n";
    sendDown(setBasicBitrate(buildCTSFrame(rtsFrame)));
}

/****************************************************************
 * Frame builder functions.
 */
Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    if (frameToSend->getControlInfo()!=NULL)
    {
        cObject * ctr = frameToSend->getControlInfo();
        PhyControlInfo *ctrl = dynamic_cast <PhyControlInfo*> (ctr);
        if (ctrl == NULL)
            opp_error("control info is not PhyControlInfo type %s");
        frame->setControlInfo(ctrl->dup());
    }
    if (isMulticast(frameToSend))
        frame->setDuration(0);
    else if (!frameToSend->getMoreFragments())
    {
        if (txop)

        {
            // ++ operation is safe because txop is true
            std::list<Ieee80211DataOrMgmtFrame*>::iterator nextframeToSend;
            nextframeToSend = transmissionQueue()->begin();
            nextframeToSend++;
            ASSERT(transmissionQueue()->end() != nextframeToSend);
            double bitRate = bitrate;
            int size = (*nextframeToSend)->getBitLength();
            if (transmissionQueue()->front()->getControlInfo() && dynamic_cast<PhyControlInfo*>(transmissionQueue()->front()->getControlInfo()))
            {
                bitRate = dynamic_cast<PhyControlInfo*>(transmissionQueue()->front()->getControlInfo())->getBitrate();
                if (bitRate == 0)
                    bitRate = bitrate;
            }
            frame->setDuration(3 * getSIFS() + 2 * computeFrameDuration(LENGTH_ACK, basicBitrate)
                               + computeFrameDuration(size,bitRate));
        }
        else
            frame->setDuration(getSIFS() + computeFrameDuration(LENGTH_ACK, basicBitrate));
    }
    else
        // FIXME: shouldn't we use the next frame to be sent?
        frame->setDuration(3 * getSIFS() + 2 * computeFrameDuration(LENGTH_ACK, basicBitrate) + computeFrameDuration(frameToSend));

    return frame;
}

Ieee80211ACKFrame *Ieee80211Mac::buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    Ieee80211ACKFrame *frame = new Ieee80211ACKFrame("wlan-ack");
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());

    if (!frameToACK->getMoreFragments())
        frame->setDuration(0);
    else
        frame->setDuration(frameToACK->getDuration() - getSIFS() - computeFrameDuration(LENGTH_ACK, basicBitrate));

    return frame;
}

Ieee80211RTSFrame *Ieee80211Mac::buildRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211RTSFrame *frame = new Ieee80211RTSFrame("wlan-rts");
    frame->setTransmitterAddress(address);
    frame->setReceiverAddress(frameToSend->getReceiverAddress());
    frame->setDuration(3 * getSIFS() + computeFrameDuration(LENGTH_CTS, basicBitrate) +
                       computeFrameDuration(frameToSend) +
                       computeFrameDuration(LENGTH_ACK, basicBitrate));

    return frame;
}

Ieee80211CTSFrame *Ieee80211Mac::buildCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("wlan-cts");
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - getSIFS() - computeFrameDuration(LENGTH_CTS, basicBitrate));

    return frame;
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    PhyControlInfo *phyControlInfo_old = dynamic_cast<PhyControlInfo *>( frameToSend->getControlInfo() );
    if (phyControlInfo_old)
    {
        ev<<"Per frame1 params"<<endl;
        PhyControlInfo *phyControlInfo_new = new PhyControlInfo;
        *phyControlInfo_new = *phyControlInfo_old;
        //EV<<"PhyControlInfo bitrate "<<phyControlInfo->getBitrate()/1e6<<"Mbps txpower "<<phyControlInfo->txpower()<<"mW"<<endl;
        frame->setControlInfo(  phyControlInfo_new  );
    }

    frame->setDuration(0);
    return frame;
}

Ieee80211Frame *Ieee80211Mac::setBasicBitrate(Ieee80211Frame *frame)
{
    ASSERT(frame->getControlInfo()==NULL);
    PhyControlInfo *ctrl = new PhyControlInfo();
    ctrl->setBitrate(basicBitrate);
    frame->setControlInfo(ctrl);
    return frame;
}

Ieee80211Frame *Ieee80211Mac::setBitrateFrame(Ieee80211Frame *frame)
{
    if (rateControlMode == RATE_CR && forceBitRate == false)
    {
        if (frame->getControlInfo())
            delete  frame->removeControlInfo();
        return frame;
    }
    PhyControlInfo *ctrl = NULL;
    if (frame->getControlInfo()==NULL)
    {
        ctrl = new PhyControlInfo();
        frame->setControlInfo(ctrl);
    }
    else
        ctrl = dynamic_cast<PhyControlInfo*>(frame->getControlInfo());
    if (ctrl)
        ctrl->setBitrate(getBitrate());
    return frame;
}


/****************************************************************
 * Helper functions.
 */
void Ieee80211Mac::finishCurrentTransmission()
{
    popTransmissionQueue();
    resetStateVariables();
}

void Ieee80211Mac::giveUpCurrentTransmission()
{
    Ieee80211DataOrMgmtFrame *temp = (Ieee80211DataOrMgmtFrame*) transmissionQueue()->front();
    nb->fireChangeNotification(NF_LINK_BREAK, temp);
    popTransmissionQueue();
    resetStateVariables();
    numGivenUp()++;
}

void Ieee80211Mac::retryCurrentTransmission()
{
    ASSERT(retryCounter() < transmissionLimit - 1);
    getCurrentTransmission()->setRetry(true);
    if (rateControlMode == RATE_AARF || rateControlMode == RATE_ARF)
        reportDataFailed();
    else
        retryCounter() ++;
    numRetry()++;
    backoff() = true;
    generateBackoffPeriod();
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::getCurrentTransmission()
{
    return transmissionQueue()->empty() ? NULL : (Ieee80211DataOrMgmtFrame *)transmissionQueue()->front();
}

void Ieee80211Mac::sendDownPendingRadioConfigMsg()
{
    if (pendingRadioConfigMsg != NULL)
    {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = NULL;
    }
}

void Ieee80211Mac::setMode(Mode mode)
{
    if (mode == PCF)
        error("PCF mode not yet supported");

    this->mode = mode;
}

void Ieee80211Mac::resetStateVariables()
{
    backoffPeriod() = 0;
    if (rateControlMode == RATE_AARF || rateControlMode == RATE_ARF)
        reportDataOk();
    else
        retryCounter() = 0;

    if (!transmissionQueue()->empty())
    {
        backoff() = true;
        getCurrentTransmission()->setRetry(false);
    }
    else
    {
        backoff() = false;
    }
}

bool Ieee80211Mac::isMediumStateChange(cMessage *msg)
{
    return msg == mediumStateChange || (msg == endReserve && radioState == RadioState::IDLE);
}

bool Ieee80211Mac::isMediumFree()
{
    return radioState == RadioState::IDLE && !endReserve->isScheduled();
}

bool Ieee80211Mac::isMulticast(Ieee80211Frame *frame)
{
    return frame && frame->getReceiverAddress().isMulticast();
}

bool Ieee80211Mac::isForUs(Ieee80211Frame *frame)
{
    return frame && frame->getReceiverAddress() == address;
}

bool Ieee80211Mac::isSentByUs(Ieee80211Frame *frame)
{

    if (dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
    {
        //EV<<"ad3 "<<((Ieee80211DataOrMgmtFrame *)frame)->getAddress3();
        //EV<<"myad "<<address<<endl;
        if ( ((Ieee80211DataOrMgmtFrame *)frame)->getAddress3() == address)//received frame sent by us
            return 1;
    }
    else
        EV<<"Cast failed"<<endl;

    return 0;

}

bool Ieee80211Mac::isDataOrMgmtFrame(Ieee80211Frame *frame)
{
    return dynamic_cast<Ieee80211DataOrMgmtFrame*>(frame);
}

bool Ieee80211Mac::isMsgAIFS(cMessage *msg)
{
    for (int i = 0; i<numCategories(); i++)
        if (msg == endAIFS(i))
            return true;
    return false;
}

Ieee80211Frame *Ieee80211Mac::getFrameReceivedBeforeSIFS()
{
    return (Ieee80211Frame *)endSIFS->getContextPointer();
}

void Ieee80211Mac::popTransmissionQueue()
{
    EV << "dropping frame from transmission queue\n";
    Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *>(transmissionQueue()->front());
    ASSERT(!transmissionQueue()->empty());
    transmissionQueue()->pop_front();
    if (queueModule)
    {
        if (numCategories()==1)
        {
        // the module are continuously asking for packets
            EV << "requesting another frame from queue module\n";
            queueModule->requestPacket();
         }
         else if (numCategories()>1 && (int)transmissionQueueSize()==maxQueueSize-1)
         {
         // Now exist a empty frame space
         // the module are continuously asking for packets
            EV << "requesting another frame from queue module\n";
            queueModule->requestPacket();
         }
    }
    delete temp;
}

double Ieee80211Mac::computeFrameDuration(Ieee80211Frame *msg)
{

    PhyControlInfo *ctrl;
    double duration;
    ev<<*msg;
    ctrl = dynamic_cast<PhyControlInfo*> ( msg->removeControlInfo() );
    if ( ctrl )
    {
        ev<<"Per frame2 params bitrate "<<ctrl->getBitrate()/1e6<<endl;
        duration = computeFrameDuration(msg->getBitLength(), ctrl->getBitrate());
        delete ctrl;
        return duration;
    }
    else

        return computeFrameDuration(msg->getBitLength(), bitrate);
}

double Ieee80211Mac::computeFrameDuration(int bits, double bitrate)
{
    double duration;
    if (PHY_HEADER_LENGTH<0)
    {
        ModulationType modType;
        modType = WifiModulationType::getModulationType(opMode, bitrate);
        duration = SIMTIME_DBL(WifiModulationType::calculateTxDuration(bits, modType, wifiPreambleType));
    }
    else
    {
        // FIXME: check how PHY_HEADER_LENGTH is handled. Is that given in bytes or secs ???
        // what is the real unit? The use seems to be incosistent betwen b and g/a/p modes.
        if ((opMode=='g') || (opMode=='a') || (opMode=='p'))
            duration = 4*ceil((16+bits+6)/(bitrate/1e6*4))*1e-6 + PHY_HEADER_LENGTH;
        else if (opMode=='b')
            duration = bits / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER;
        else
            opp_error("Opmode not supported");
    }

    EV<<" duration="<<duration*1e6<<"us("<<bits<<"bits "<<bitrate/1e6<<"Mbps)"<<endl;
    return duration;
}

void Ieee80211Mac::logState()
{
    std::string a[10];
    std::string b[10];
    std::string medium = "busy";
    for (int i=0; i<10; i++)
    {
        a[i] = "";
        b[i] = "";
    }

    if (isMediumFree())
        medium = "free";
    for (int i=0; i<numCategories(); i++)
    {
        if (endBackoff(i)->isScheduled())
            b[i] = "scheduled";
        if (endAIFS(i)->isScheduled())
            a[i] = "scheduled";
    }

    EV  << "# state information: mode = " << modeName(mode) << ", state = " << fsm.getStateName();
    EV << ", backoff 0.."<<numCategories()<<" = ";
    for (int i=0; i<numCategories(); i++)
         EV << backoff(i) << " ";
    EV <<  "\n# backoffPeriod 0.."<<numCategories()<<" = ";
    for (int i=0; i<numCategories(); i++)
         EV << backoffPeriod(i) << " ";
    EV << "\n# retryCounter 0.."<<numCategories()<<" = ";
    for (int i=0; i<numCategories(); i++)
         EV << retryCounter(i) << " ";
    EV << ", radioState = " << radioState << ", nav = " << nav <<  ",txop is "<< txop << "\n";
    EV << "#queue size 0.."<<numCategories()<<" = ";
    for (int i=0; i<numCategories(); i++)
        EV << transmissionQueue(i)->size() << " ";
    EV << ", medium is " << medium << ", scheduled AIFS are ";
    for (int i=0; i<numCategories(); i++)
        EV << i << "(" << a[i] << ")";
    EV << ", scheduled backoff are ";
    for (int i=0; i<numCategories(); i++)
        EV << i << "(" << b[i] << ")";
    EV << "\n# currentAC: " << currentAC << ", oldcurrentAC: " << oldcurrentAC;
    if (getCurrentTransmission() != NULL)
         EV << "\n# current transmission: " << getCurrentTransmission()->getId();
    else
        EV << "\n# current transmission: none";
    EV << endl;
}

const char *Ieee80211Mac::modeName(int mode)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (mode)
    {
        CASE(DCF);
        CASE(PCF);
    }
    return s;
#undef CASE
}

bool Ieee80211Mac::transmissionQueueEmpty()
{
    for (int i=0; i<numCategories(); i++)
       if (!transmissionQueue(i)->empty()) return false;
    return true;
}

unsigned int Ieee80211Mac::transmissionQueueSize()
{
    unsigned int totalSize=0;
    for (int i=0; i<numCategories(); i++)
        totalSize+=transmissionQueue(i)->size();
    return totalSize;
}

void Ieee80211Mac::reportDataOk()
{
    retryCounter() = 0;
    if (rateControlMode==RATE_CR)
       return;
    successCounter ++;
    failedCounter = 0;
    recovery = false;
    if ((successCounter == getSuccessThreshold() || timer == getTimerTimeout())
            && Ieee80211Descriptor::incIdx(rateIndex))
    {
        setBitrate(Ieee80211Descriptor::getDescriptor(rateIndex).bitrate);
        timer = 0;
        successCounter = 0;
        recovery = true;
    }
}

void Ieee80211Mac::reportDataFailed(void)
{
    retryCounter()++;
    if (rateControlMode==RATE_CR)
       return;
    timer++;
    failedCounter++;
    successCounter = 0;
    if (recovery)
    {
        if (retryCounter() == 1)
        {
            reportRecoveryFailure();
            if (Ieee80211Descriptor::decIdx(rateIndex))
                setBitrate(Ieee80211Descriptor::getDescriptor(rateIndex).bitrate);
        }
        timer = 0;
    }
    else
    {
        if (needNormalFallback())
        {
            reportFailure();
            if (Ieee80211Descriptor::decIdx(rateIndex))
                setBitrate(Ieee80211Descriptor::getDescriptor(rateIndex).bitrate);
        }
        if (retryCounter() >= 2)
        {
            timer = 0;
        }
    }
}

int Ieee80211Mac::getMinTimerTimeout(void)
{
    return minTimerTimeout;
}

int Ieee80211Mac::getMinSuccessThreshold(void)
{
    return minSuccessThreshold;
}

int Ieee80211Mac::getTimerTimeout(void)
{
    return timerTimeout;
}

int Ieee80211Mac::getSuccessThreshold(void)
{
    return successThreshold;
}

void Ieee80211Mac::setTimerTimeout(int timer_timeout)
{
    if (timer_timeout >= minTimerTimeout)
        timerTimeout = timer_timeout;
    else
        error("timer_timeout is less than minTimerTimeout");
}
void Ieee80211Mac::setSuccessThreshold(int success_threshold)
{
    if (success_threshold >= minSuccessThreshold)
        successThreshold = success_threshold;
    else
        error("success_threshold is less than minSuccessThreshold");
}

void Ieee80211Mac::reportRecoveryFailure(void)
{
    if (rateControlMode == RATE_AARF)
    {
        setSuccessThreshold((int)(std::min((double)getSuccessThreshold() * successCoeff, (double) maxSuccessThreshold)));
        setTimerTimeout((int)(std::max((double)getMinTimerTimeout(), (double)(getSuccessThreshold() * timerCoeff))));
    }
}

void Ieee80211Mac::reportFailure(void)
{
    if (rateControlMode == RATE_AARF)
    {
        setTimerTimeout(getMinTimerTimeout());
        setSuccessThreshold(getMinSuccessThreshold());
    }
}

bool Ieee80211Mac::needRecoveryFallback(void)
{
    if (retryCounter() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Ieee80211Mac::needNormalFallback(void)
{
    int retryMod = (retryCounter() - 1) % 2;
    if (retryMod == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

double Ieee80211Mac::getBitrate()
{
    return bitrate;
}

void Ieee80211Mac::setBitrate(double rate)
{
    bitrate = rate;
}


// method for access to the EDCA data


// methods for access to specific AC data
bool & Ieee80211Mac::backoff(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
    return edcCAF[i].backoff;
}

simtime_t & Ieee80211Mac::TXOP(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].TXOP;
}

simtime_t & Ieee80211Mac::backoffPeriod(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].backoffPeriod;
}

int & Ieee80211Mac::retryCounter(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].retryCounter;
}

int & Ieee80211Mac::AIFSN(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].AIFSN;
}

int & Ieee80211Mac::cwMax(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].cwMax;
}

int & Ieee80211Mac::cwMin(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].cwMin;
}

cMessage * Ieee80211Mac::endAIFS(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].endAIFS;
}

cMessage * Ieee80211Mac::endBackoff(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].endBackoff;
}

const bool Ieee80211Mac::isBakoffMsg(cMessage *msg)
{
    for (unsigned int i=0; i<edcCAF.size(); i++)
    {
        if (msg==edcCAF[i].endBackoff)
           return true;
    }
    return false;
}

// Statistics
long & Ieee80211Mac::numRetry(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].numRetry;
}

long & Ieee80211Mac::numSentWithoutRetry(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)numCategories())
         opp_error("AC doesn't exist");
     return edcCAF[i].numSentWithoutRetry;
}

long & Ieee80211Mac::numGivenUp(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].numGivenUp;
}

long & Ieee80211Mac::numSent(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].numSent;
}

long & Ieee80211Mac::numDropped(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].numDropped;
}

long & Ieee80211Mac::bites(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].bites;
}

simtime_t & Ieee80211Mac::minjitter(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].minjitter;
}

simtime_t & Ieee80211Mac::maxjitter(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAF[i].maxjitter;
}

// out vectors


cOutVector * Ieee80211Mac::jitter(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAFOutVector[i].jitter;
}

cOutVector * Ieee80211Mac::macDelay(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAFOutVector[i].macDelay;
}

cOutVector * Ieee80211Mac::throughput(int i)
{
    if (i==-1)
         i = currentAC;
    if (i>=(int)edcCAF.size())
         opp_error("AC doesn't exist");
     return edcCAFOutVector[i].throughput;
}

Ieee80211Mac::Ieee80211DataOrMgmtFrameList * Ieee80211Mac::transmissionQueue(int i)
{
    if (i==-1)
        i = currentAC;
    if (i>=(int)edcCAF.size())
        opp_error("AC doesn't exist");
    return &(edcCAF[i].transmissionQueue);
}


ModulationType
Ieee80211Mac::getControlAnswerMode(ModulationType reqMode)
{
  /**
   * The standard has relatively unambiguous rules for selecting a
   * control response rate (the below is quoted from IEEE 802.11-2007,
   * Section 9.6):
   *
   *   To allow the transmitting STA to calculate the contents of the
   *   Duration/ID field, a STA responding to a received frame shall
   *   transmit its Control Response frame (either CTS or ACK), other
   *   than the BlockAck control frame, at the highest rate in the
   *   BSSBasicRateSet parameter that is less than or equal to the
   *   rate of the immediately previous frame in the frame exchange
   *   sequence (as defined in 9.12) and that is of the same
   *   modulation class (see 9.6.1) as the received frame...
   */

  /**
   * If no suitable basic rate was found, we search the mandatory
   * rates. The standard (IEEE 802.11-2007, Section 9.6) says:
   *
   *   ...If no rate contained in the BSSBasicRateSet parameter meets
   *   these conditions, then the control frame sent in response to a
   *   received frame shall be transmitted at the highest mandatory
   *   rate of the PHY that is less than or equal to the rate of the
   *   received frame, and that is of the same modulation class as the
   *   received frame. In addition, the Control Response frame shall
   *   be sent using the same PHY options as the received frame,
   *   unless they conflict with the requirement to use the
   *   BSSBasicRateSet parameter.
   *
   * TODO: Note that we're ignoring the last sentence for now, because
   * there is not yet any manipulation here of PHY options.
   */
    bool found = false;
    ModulationType mode;
    for (uint32_t idx = Ieee80211Descriptor::getMinIdx(opMode); idx < Ieee80211Descriptor::size(); idx++)
    {
        if (Ieee80211Descriptor::getDescriptor(idx).mode != opMode)
            break;
        ModulationType thismode;
        thismode = WifiModulationType::getModulationType(opMode, Ieee80211Descriptor::getDescriptor(idx).bitrate);

      /* If the rate:
       *
       *  - is a mandatory rate for the PHY, and
       *  - is equal to or faster than our current best choice, and
       *  - is less than or equal to the rate of the received frame, and
       *  - is of the same modulation class as the received frame
       *
       * ...then it's our best choice so far.
       */
      if (thismode.getIsMandatory()
          && (!found || thismode.getPhyRate() > mode.getPhyRate())
          && thismode.getPhyRate() <= reqMode.getPhyRate()
          && thismode.getModulationClass() == reqMode.getModulationClass())
        {
          mode = thismode;
          // As above; we've found a potentially-suitable transmit
          // rate, but we need to continue and consider all the
          // mandatory rates before we can be sure we've got the right
          // one.
          found = true;
        }
    }

  /**
   * If we still haven't found a suitable rate for the response then
   * someone has messed up the simulation config. This probably means
   * that the WifiPhyStandard is not set correctly, or that a rate that
   * is not supported by the PHY has been explicitly requested in a
   * WifiRemoteStationManager (or descendant) configuration.
   *
   * Either way, it is serious - we can either disobey the standard or
   * fail, and I have chosen to do the latter...
   */
  if (!found)
    {
      opp_error("Can't find response rate for reqMode. Check standard and selected rates match.");
    }

  return mode;
}

// This methods implemet the duplicate filter
void Ieee80211Mac::sendUp(cMessage *msg)
{
    EV << "sending up " << msg << "\n";

    if (!isDuplicated(msg)) // duplicate detection filter
    {
        if (msg->isPacket())
            emit(packetSentToUpperSignal, msg);

        send(msg, upperLayerOut);
    }
}

void Ieee80211Mac::removeOldTuplesFromDuplicateMap()
{
    if (duplicateDetect && lastTimeDelete+duplicateTimeOut>=simTime())
    {
        lastTimeDelete=simTime();
        for (Ieee80211ASFTupleList::iterator it = asfTuplesList.begin();it!=asfTuplesList.begin();)
        {
            if (it->second.receivedTime+duplicateTimeOut<simTime())
            {
                Ieee80211ASFTupleList::iterator itAux=it;
                it++;
                asfTuplesList.erase(itAux);
            }
            else
                it++;
        }
    }
}

const MACAddress & Ieee80211Mac::isInterfaceRegistered()
{
    if (!par("multiMac").boolValue())
        return MACAddress::UNSPECIFIED_ADDRESS;

    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return MACAddress::UNSPECIFIED_ADDRESS;
    char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];
    char *d = interfaceName;
    for (const char *s = getParentModule()->getFullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';
    InterfaceEntry * e = ift->getInterfaceByName(interfaceName);
    delete [] interfaceName;
    if (e)
        return e->getMacAddress();
    return MACAddress::UNSPECIFIED_ADDRESS;
}

bool Ieee80211Mac::isDuplicated(cMessage *msg)
{
    if (duplicateDetect) // duplicate detection filter
    {
        Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(msg);
        if (frame)
        {
            Ieee80211ASFTupleList::iterator it = asfTuplesList.find(frame->getTransmitterAddress());
            if (it == asfTuplesList.end())
            {
                Ieee80211ASFTuple tuple;
                tuple.receivedTime = simTime();
                tuple.sequenceNumber = frame->getSequenceNumber();
                tuple.fragmentNumber = frame->getFragmentNumber();
                asfTuplesList.insert(std::pair<MACAddress, Ieee80211ASFTuple>(frame->getTransmitterAddress(), tuple));
            }
            else
            {
                // check if duplicate
                if (it->second.sequenceNumber == frame->getSequenceNumber()
                        && it->second.fragmentNumber == frame->getFragmentNumber())
                {
                    return true;
                }
                else
                {
                    // actualize
                    it->second.sequenceNumber = frame->getSequenceNumber();
                    it->second.fragmentNumber = frame->getFragmentNumber();
                    it->second.receivedTime = simTime();
                }
            }
        }
    }
    return false;
}

void Ieee80211Mac::promiscousFrame(cMessage *msg)
{
    if (!isDuplicated(msg)) // duplicate detection filter
        nb->fireChangeNotification(NF_LINK_PROMISCUOUS, msg);
}
