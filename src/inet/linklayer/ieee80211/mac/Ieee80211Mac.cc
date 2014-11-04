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

#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/physicallayer/contract/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee80211/Ieee80211aControlInfo_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211eClassifier.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211DataRate.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace ieee80211 {

// TODO: 9.3.2.1, If there are buffered multicast or broadcast frames, the PC shall transmit these prior to any unicast frames.
// TODO: control frames must send before

Define_Module(Ieee80211Mac);

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(inet::Ieee80211Mac,
        (Ieee80211Mac::IDLE,
         Ieee80211Mac::DEFER,
         Ieee80211Mac::WAITAIFS,
         Ieee80211Mac::BACKOFF,
         Ieee80211Mac::WAITACK,
         Ieee80211Mac::WAITMULTICAST,
         Ieee80211Mac::WAITCTS,
         Ieee80211Mac::WAITSIFS,
         Ieee80211Mac::RECEIVE));

/****************************************************************
 * Construction functions.
 */

Ieee80211Mac::Ieee80211Mac() :
    transmissionState(IRadio::TRANSMISSION_STATE_UNDEFINED),
    throughputTimer(NULL),
    classifier(NULL),
    radio(NULL),
    fr(NULL),
    queueModule(NULL),
    pendingRadioConfigMsg(NULL),
    endSIFS(NULL),
    endDIFS(NULL),
    endTXOP(NULL),
    endTimeout(NULL),
    endReserve(NULL),
    mediumStateChange(NULL)
{
}

Ieee80211Mac::~Ieee80211Mac()
{
    cancelAndDelete(endSIFS);
    cancelAndDelete(endDIFS);
    cancelAndDelete(endTimeout);
    cancelAndDelete(endReserve);
    cancelAndDelete(mediumStateChange);
    cancelAndDelete(endTXOP);
    for (unsigned int i = 0; i < edcCAF.size(); i++) {
        cancelAndDelete(endAIFS(i));
        cancelAndDelete(endBackoff(i));
        while (!transmissionQueue(i)->empty()) {
            Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *>(transmissionQueue(i)->front());
            transmissionQueue(i)->pop_front();
            delete temp;
        }
    }
    edcCAF.clear();
    for (unsigned int i = 0; i < edcCAFOutVector.size(); i++) {
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
    EV_DEBUG << "Initializing stage " << stage << endl;

    MACProtocolBase::initialize(stage);

    //TODO: revise it: it's too big; should revise stages, too!!!
    if (stage == INITSTAGE_LOCAL) {
        int numQueues = 1;
        if (par("EDCA")) {
            const char *classifierClass = par("classifier");
            classifier = check_and_cast<IQoSClassifier *>(inet::utils::createOne(classifierClass));
            numQueues = classifier->getNumQueues();
        }

        for (int i = 0; i < numQueues; i++) {
            Edca catEdca;
            catEdca.backoff = false;
            catEdca.backoffPeriod = -1;
            catEdca.retryCounter = 0;
            edcCAF.push_back(catEdca);
        }
        // initialize parameters
        const char *opModeStr = par("opMode").stringValue();
        if (strcmp("b", opModeStr) == 0)
            opMode = 'b';
        else if (strcmp("g", opModeStr) == 0)
            opMode = 'g';
        else if (strcmp("a", opModeStr) == 0)
            opMode = 'a';
        else if (strcmp("p", opModeStr) == 0)
            opMode = 'p';
        else
            throw cRuntimeError("Invalid opMode='%s'", opModeStr);

        PHY_HEADER_LENGTH = par("phyHeaderLength");    //26us

        if (strcmp("SHORT", par("wifiPreambleMode").stringValue()) == 0)
            wifiPreambleType = IEEE80211_PREAMBLE_SHORT;
        else if (strcmp("LONG", par("wifiPreambleMode").stringValue()) == 0)
            wifiPreambleType = IEEE80211_PREAMBLE_LONG;
        else
            throw cRuntimeError("Invalid wifiPreambleType. Must be SHORT or LONG");

        useModulationParameters = par("useModulationParameters");

        prioritizeMulticast = par("prioritizeMulticast");

        EV_DEBUG << "Operating mode: 802.11" << opMode;
        maxQueueSize = par("maxQueueSize");
        rtsThreshold = par("rtsThresholdBytes");

        // the variable is renamed due to a confusion in the standard
        // the name retry limit would be misleading, see the header file comment
        transmissionLimit = par("retryLimit");
        if (transmissionLimit == -1)
            transmissionLimit = 7;
        ASSERT(transmissionLimit >= 0);

        EV_DEBUG << " retryLimit=" << transmissionLimit;

        cwMinData = par("cwMinData");
        if (cwMinData == -1)
            cwMinData = CW_MIN;
        ASSERT(cwMinData >= 0 && cwMinData <= 32767);

        cwMaxData = par("cwMaxData");
        if (cwMaxData == -1)
            cwMaxData = CW_MAX;
        ASSERT(cwMaxData >= 0 && cwMaxData <= 32767);

        cwMinMulticast = par("cwMinMulticast");
        if (cwMinMulticast == -1)
            cwMinMulticast = 31;
        ASSERT(cwMinMulticast >= 0);
        EV_DEBUG << " cwMinMulticast=" << cwMinMulticast;

        defaultAC = par("defaultAC");
        if (classifier && dynamic_cast<Ieee80211eClassifier *>(classifier))
            dynamic_cast<Ieee80211eClassifier *>(classifier)->setDefaultClass(defaultAC);

        for (int i = 0; i < numCategories(); i++) {
            std::stringstream os;
            os << i;
            std::string strAifs = "AIFSN" + os.str();
            std::string strTxop = "TXOP" + os.str();
            if (hasPar(strAifs.c_str()) && hasPar(strTxop.c_str())) {
                AIFSN(i) = par(strAifs.c_str());
                TXOP(i) = par(strTxop.c_str());
            }
            else
                throw cRuntimeError("parameters %s , %s don't exist", strAifs.c_str(), strTxop.c_str());
        }
        if (numCategories() == 1)
            AIFSN(0) = par("AIFSN");

        for (int i = 0; i < numCategories(); i++) {
            ASSERT(AIFSN(i) >= 0 && AIFSN(i) < 16);
            if (i == 0 || i == 1) {
                cwMin(i) = cwMinData;
                cwMax(i) = cwMaxData;
            }
            if (i == 2) {
                cwMin(i) = (cwMinData + 1) / 2 - 1;
                cwMax(i) = cwMinData;
            }
            if (i == 3) {
                cwMin(i) = (cwMinData + 1) / 4 - 1;
                cwMax(i) = (cwMinData + 1) / 2 - 1;
            }
        }

        ST = par("slotTime");    //added by sorin
        if (ST == -1)
            ST = 20e-6; //20us

        basicBitrate = par("basicBitrate");
        bitrate = par("bitrate");
        duplicateDetect = par("duplicateDetectionFilter");
        purgeOldTuples = par("purgeOldTuples");
        duplicateTimeOut = par("duplicateTimeOut");
        lastTimeDelete = 0;

        if (bitrate == -1) {
            rateIndex = Ieee80211Descriptor::getMaxIdx(opMode);
            bitrate = Ieee80211Descriptor::getDescriptor(rateIndex).bitrate;
        }
        else
            rateIndex = Ieee80211Descriptor::getIdx(opMode, bitrate);

        if (basicBitrate == -1) {
            int basicBitrateIdx = Ieee80211Descriptor::getMaxIdx(opMode);
            basicBitrate = Ieee80211Descriptor::getDescriptor(basicBitrateIdx).bitrate;
        }
        else
            Ieee80211Descriptor::getIdx(opMode, basicBitrate);

        controlBitRate = par("controlBitrate").doubleValue();

        EV_DEBUG << " slotTime=" << getSlotTime() * 1e6 << "us DIFS=" << getDIFS() * 1e6 << "us";

        if (controlBitRate == -1)
        {
            int basicBitrateIdx = Ieee80211Descriptor::getMaxIdx(opMode);
            controlBitRate = Ieee80211Descriptor::getDescriptor(basicBitrateIdx).bitrate;
            controlFrameModulationType = Ieee80211Descriptor::getDescriptor(basicBitrateIdx).modulationType;
        }
        else
        {
            int basicBitrateIdx = Ieee80211Descriptor::getIdx(opMode, controlBitRate);
            controlFrameModulationType = Ieee80211Descriptor::getDescriptor(basicBitrateIdx).modulationType;
        }

        // configure AutoBit Rate
        configureAutoBitRate();
        //end auto rate code
        EV_DEBUG << " basicBitrate=" << basicBitrate / 1e6 << "Mb ";
        EV_DEBUG << " bitrate=" << bitrate / 1e6 << "Mb IDLE=" << IDLE << " RECEIVE=" << RECEIVE << endl;

        const char *addressString = par("address");
        address = isInterfaceRegistered();
        if (address.isUnspecified()) {
            if (!strcmp(addressString, "auto")) {
                // assign automatic address
                address = MACAddress::generateAutoAddress();
                // change module parameter from "auto" to concrete address
                par("address").setStringValue(address.str().c_str());
            }
            else
                address.setAddress(addressString);
        }

        // initialize self messages
        endSIFS = new cMessage("SIFS");
        endDIFS = new cMessage("DIFS");
        for (int i = 0; i < numCategories(); i++) {
            setEndAIFS(i, new cMessage("AIFS", i));
            setEndBackoff(i, new cMessage("Backoff", i));
        }
        endTXOP = new cMessage("TXOP");
        endTimeout = new cMessage("Timeout");
        endReserve = new cMessage("Reserve");
        mediumStateChange = new cMessage("MediumStateChange");

        // obtain pointer to external queue
        initializeQueueModule();    //FIXME STAGE: this should be in L2 initialization!!!!

        // state variables
        fsm.setName("Ieee80211Mac State Machine");
        mode = DCF;
        sequenceNumber = 0;

        currentAC = 0;
        oldcurrentAC = 0;
        lastReceiveFailed = false;
        for (int i = 0; i < numCategories(); i++)
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
        for (int i = 0; i < numCategories(); i++) {
            numRetry(i) = 0;
            numSentWithoutRetry(i) = 0;
            numGivenUp(i) = 0;
            numSent(i) = 0;
            bits(i) = 0;
            maxJitter(i) = SIMTIME_ZERO;
            minJitter(i) = SIMTIME_ZERO;
        }

        numCollision = 0;
        numInternalCollision = 0;
        numReceived = 0;
        numSentMulticast = 0;
        numReceivedMulticast = 0;
        numBits = 0;
        numSentTXOP = 0;
        numReceivedOther = 0;
        numAckSend = 0;
        successCounter = 0;
        failedCounter = 0;
        recovery = 0;
        timer = 0;
        timeStampLastMessageReceived = SIMTIME_ZERO;

        stateVector.setName("State");
        stateVector.setEnum("inet::Ieee80211Mac");
        for (int i = 0; i < numCategories(); i++) {
            EdcaOutVector outVectors;
            std::stringstream os;
            os << i;
            std::string th = "throughput AC" + os.str();
            std::string delay = "Mac delay AC" + os.str();
            std::string jit = "jitter AC" + os.str();
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
        if (throughputTimePeriod > 0)
            throughputTimer = new cMessage("throughput-timer");
        if (throughputTimer)
            scheduleAt(simTime() + throughputTimePeriod, throughputTimer);
        // end initialize variables throughput over a period of time
        // initialize watches
        validRecMode = false;
        initWatches();

        cModule *radioModule = gate("lowerLayerOut")->getNextGate()->getOwnerModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        if (isOperational)
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        // interface
        if (isInterfaceRegistered().isUnspecified()) //TODO do we need multi-MAC feature? if so, should they share interfaceEntry??  --Andras
            registerInterface();
    }
}

void Ieee80211Mac::initWatches()
{
// initialize watches
    WATCH(fsm);
    for (int i = 0; i < numCategories(); i++)
        WATCH(edcCAF[i].retryCounter);
    for (int i = 0; i < numCategories(); i++)
        WATCH(edcCAF[i].backoff);
    for (int i = 0; i < numCategories(); i++)
        WATCH(edcCAF[i].backoffPeriod);
    WATCH(currentAC);
    WATCH(oldcurrentAC);
    for (int i = 0; i < numCategories(); i++)
        WATCH_LIST(edcCAF[i].transmissionQueue);
    WATCH(nav);
    WATCH(txop);

    for (int i = 0; i < numCategories(); i++)
        WATCH(edcCAF[i].numRetry);
    for (int i = 0; i < numCategories(); i++)
        WATCH(edcCAF[i].numSentWithoutRetry);
    for (int i = 0; i < numCategories(); i++)
        WATCH(edcCAF[i].numGivenUp);
    WATCH(numCollision);
    for (int i = 0; i < numCategories(); i++)
        WATCH(edcCAF[i].numSent);
    WATCH(numBits);
    WATCH(numSentTXOP);
    WATCH(numReceived);
    WATCH(numSentMulticast);
    WATCH(numReceivedMulticast);
    for (int i = 0; i < numCategories(); i++)
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
    switch (autoBitrate) {
        case 0:
            rateControlMode = RATE_CR;
            EV_DEBUG << "MAC Transmission algorithm : Constant Rate" << endl;
            break;

        case 1:
            rateControlMode = RATE_ARF;
            EV_DEBUG << "MAC Transmission algorithm : ARF Rate" << endl;
            break;

        case 2:
            rateControlMode = RATE_AARF;
            successCoeff = par("successCoeff");
            timerCoeff = par("timerCoeff");
            maxSuccessThreshold = par("maxSuccessThreshold");
            EV_DEBUG << "MAC Transmission algorithm : AARF Rate" << endl;
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
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "number of retry for AC " + os.str();
        recordScalar(th.c_str(), numRetry(i));
    }
    recordScalar("sent and received bits", numBits);
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "sent packet within AC " + os.str();
        recordScalar(th.c_str(), numSent(i));
    }
    recordScalar("sent in TXOP ", numSentTXOP);
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "sentWithoutRetry AC " + os.str();
        recordScalar(th.c_str(), numSentWithoutRetry(i));
    }
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "numGivenUp AC " + os.str();
        recordScalar(th.c_str(), numGivenUp(i));
    }
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "numDropped AC " + os.str();
        recordScalar(th.c_str(), numDropped(i));
    }
}

InterfaceEntry *Ieee80211Mac::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NetworkInterface module's name without special characters ([])
    std::string interfaceName = utils::stripnonalnum(getParentModule()->getFullName());
    e->setName(interfaceName.c_str());

    // address
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    e->setMtu(par("mtu").longValue());

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    return e;
}

void Ieee80211Mac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (par("queueModule").stringValue()[0]) {
        cModule *module = getParentModule()->getSubmodule(par("queueModule").stringValue());
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV_DEBUG << "Requesting first two frames from queue module\n";
        queueModule->requestPacket();
        // needed for backoff: mandatory if next message is already present
        queueModule->requestPacket();
    }
}

/****************************************************************
 * Message handling functions.
 */
void Ieee80211Mac::handleSelfMessage(cMessage *msg)
{
    if (msg == throughputTimer) {
        throughputLastPeriod = recBytesOverPeriod / SIMTIME_DBL(throughputTimePeriod);
        recBytesOverPeriod = 0;
        scheduleAt(simTime() + throughputTimePeriod, throughputTimer);
        return;
    }

    EV_DEBUG << "received self message: " << msg << "(kind: " << msg->getKind() << ")" << endl;

    if (msg == endReserve)
        nav = false;

    if (msg == endTXOP)
        txop = false;

    if (!strcmp(msg->getName(), "AIFS") || !strcmp(msg->getName(), "Backoff")) {
        EV_DEBUG << "Changing currentAC to " << msg->getKind() << endl;
        currentAC = msg->getKind();
    }
    //check internal collision
    if ((strcmp(msg->getName(), "Backoff") == 0) || (strcmp(msg->getName(), "AIFS") == 0)) {
        int kind;
        kind = msg->getKind();
        if (kind < 0)
            kind = 0;
        EV_DEBUG << " kind is " << kind << ",name is " << msg->getName() << endl;
        for (unsigned int i = numCategories() - 1; (int)i > kind; i--) {    //mozna prochaze jen 3..kind XXX
            if (((endBackoff(i)->isScheduled() && endBackoff(i)->getArrivalTime() == simTime())
                 || (endAIFS(i)->isScheduled() && !backoff(i) && endAIFS(i)->getArrivalTime() == simTime()))
                && !transmissionQueue(i)->empty())
            {
                EV_DEBUG << "Internal collision AC" << kind << " with AC" << i << endl;
                numInternalCollision++;
                EV_DEBUG << "Cancel backoff event and schedule new one for AC" << kind << endl;
                cancelEvent(endBackoff(kind));
                if (retryCounter() == transmissionLimit - 1) {
                    EV_WARN << "give up transmission for AC" << currentAC << endl;
                    giveUpCurrentTransmission();
                }
                else {
                    EV_WARN << "retry transmission for AC" << currentAC << endl;
                    retryCurrentTransmission();
                }
                return;
            }
        }
        currentAC = kind;
    }
    handleWithFSM(msg);
}

void Ieee80211Mac::handleUpperPacket(cPacket *msg)
{
    if (queueModule && numCategories() > 1 && (int)transmissionQueueSize() < maxQueueSize) {
        // the module are continuously asking for packets, except if the queue is full
        EV_DEBUG << "requesting another frame from queue module\n";
        queueModule->requestPacket();
    }

    // check if it's a command from the mgmt layer
    if (msg->getBitLength() == 0 && msg->getKind() != 0) {
        handleUpperCommand(msg);
        return;
    }

    // must be a Ieee80211DataOrMgmtFrame, within the max size because we don't support fragmentation
    Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
    if (frame->getByteLength() > fragmentationThreshold)
        throw cRuntimeError("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                msg->getClassName(), msg->getName(), (int)(msg->getByteLength()));
    EV_DEBUG << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    // if you get error from this assert check if is client associated to AP
    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(address);
    frame->setSequenceNumber(sequenceNumber);
    sequenceNumber = (sequenceNumber + 1) % 4096;    //XXX seqNum must be checked upon reception of frames!

    if (mappingAccessCategory(frame) == 200) {
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
    if (isDataFrame && maxQueueSize && (int)transmissionQueueSize() >= maxQueueSize) {
        EV_WARN << "message " << frame << " received from higher layer but AC queue is full, dropping message\n";
        numDropped()++;
        delete frame;
        return 200;
    }
    if (isDataFrame) {
        if (!prioritizeMulticast || !frame->getReceiverAddress().isMulticast() || transmissionQueue()->size() < 2)
            transmissionQueue()->push_back(frame);
        else {
            // if the last frame is management insert here
            Ieee80211DataFrame *frameAux = dynamic_cast<Ieee80211DataFrame *>(transmissionQueue()->back());
            if ((frameAux == NULL) || (frameAux && frameAux->getReceiverAddress().isMulticast()))
                transmissionQueue()->push_back(frame);
            else {
                // in other case search the possition
                std::list<Ieee80211DataOrMgmtFrame *>::iterator p = transmissionQueue()->end();
                while ((*p)->getReceiverAddress().isMulticast() && (p != transmissionQueue()->begin())) {    // search the first broadcast frame
                    if (dynamic_cast<Ieee80211DataFrame *>(*p) == NULL)
                        break;
                    p--;
                }
                p++;
                transmissionQueue()->insert(p, frame);
            }
        }
    }
    else {
        if (transmissionQueue()->empty() || transmissionQueue()->size() == 1) {
            transmissionQueue()->push_back(frame);
        }
        else {
            std::list<Ieee80211DataOrMgmtFrame *>::iterator p;
            //we don't know if first frame in the queue is in middle of transmission
            //so for sure we placed it on second place
            p = transmissionQueue()->begin();
            p++;
            while ((dynamic_cast<Ieee80211DataFrame *>(*p) == NULL) && (p != transmissionQueue()->end())) // search the first not management frame
                p++;
            transmissionQueue()->insert(p, frame);
        }
    }
    EV_DEBUG << "frame classified as access category " << currentAC << " (0 background, 1 best effort, 2 video, 3 voice)\n";
    return true;
}

void Ieee80211Mac::handleUpperCommand(cMessage *msg)
{
    if (msg->getKind() == RADIO_C_CONFIGURE) {
        EV_DEBUG << "Passing on command " << msg->getName() << " to physical layer\n";
        if (pendingRadioConfigMsg != NULL) {
            // merge contents of the old command into the new one, then delete it
            RadioConfigureCommand *oldConfigureCommand = check_and_cast<RadioConfigureCommand *>(pendingRadioConfigMsg->getControlInfo());
            RadioConfigureCommand *newConfigureCommand = check_and_cast<RadioConfigureCommand *>(msg->getControlInfo());
            if (newConfigureCommand->getChannelNumber() == -1 && oldConfigureCommand->getChannelNumber() != -1)
                newConfigureCommand->setChannelNumber(oldConfigureCommand->getChannelNumber());
            if (isNaN(newConfigureCommand->getBitrate().get()) && !isNaN(oldConfigureCommand->getBitrate().get()))
                newConfigureCommand->setBitrate(oldConfigureCommand->getBitrate());
            delete pendingRadioConfigMsg;
            pendingRadioConfigMsg = NULL;
        }

        if (fsm.getState() == IDLE || fsm.getState() == DEFER || fsm.getState() == BACKOFF) {
            EV_DEBUG << "Sending it down immediately\n";
/*
   // Dynamic power
            PhyControlInfo *phyControlInfo = dynamic_cast<PhyControlInfo *>(msg->getControlInfo());
            if (phyControlInfo)
                phyControlInfo->setAdaptiveSensitivity(true);
   // end dynamic power
 */
            sendDown(msg);
        }
        else {
            EV_DEBUG << "Delaying " << msg->getName() << " until next IDLE or DEFER state\n";
            pendingRadioConfigMsg = msg;
        }
    }
    else {
        throw cRuntimeError("Unrecognized command from mgmt layer: (%s)%s msgkind=%d", msg->getClassName(), msg->getName(), msg->getKind());
    }
}

void Ieee80211Mac::handleLowerPacket(cPacket *msg)
{
    EV_TRACE << "->Enter handleLowerMsg...\n";
    EV_DEBUG << "received message from lower layer: " << msg << endl;
    Radio80211aControlInfo *cinfo = dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo());
    if (cinfo && cinfo->getAirtimeMetric()) {
        double rtsTime = 0;
        if (rtsThreshold * 8 < cinfo->getTestFrameSize())
             rtsTime = controlFrameTxTime(LENGTH_CTS) + controlFrameTxTime(LENGTH_RTS);
        double frameDuration = cinfo->getTestFrameDuration() + controlFrameTxTime(LENGTH_ACK) + rtsTime;
        cinfo->setTestFrameDuration(frameDuration);
    }
    emit(NF_LINK_FULL_PROMISCUOUS, msg);
    validRecMode = false;
    if (msg->getControlInfo() && dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo())) {
        Radio80211aControlInfo *cinfo = dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo());
        recFrameModulationType = cinfo->getModulationType();
        if (recFrameModulationType.getDataRate() > 0)
            validRecMode = true;
    }

    if (rateControlMode == RATE_CR) {
        if (msg->getControlInfo())
            delete msg->removeControlInfo();
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame *>(msg);

    if (msg->getControlInfo() && dynamic_cast<Radio80211aControlInfo *>(msg->getControlInfo())) {
        Radio80211aControlInfo *cinfo = (Radio80211aControlInfo *)msg->removeControlInfo();
        if (contJ % 10 == 0) {
            snr = _snr;
            contJ = 0;
            _snr = 0;
        }
        contJ++;
        _snr += cinfo->getSnr() / 10;
        lossRate = cinfo->getLossRate();
        delete cinfo;
    }

    if (contI % samplingCoeff == 0) {
        contI = 0;
        recvdThroughput = 0;
    }
    contI++;

    frame = dynamic_cast<Ieee80211Frame *>(msg);
    if (timeStampLastMessageReceived == SIMTIME_ZERO)
        timeStampLastMessageReceived = simTime();
    else {
        if (frame)
            recvdThroughput += ((frame->getBitLength() / (simTime() - timeStampLastMessageReceived)) / 1000000) / samplingCoeff;
        timeStampLastMessageReceived = simTime();
    }
    if (frame && throughputTimer)
        recBytesOverPeriod += frame->getByteLength();

    if (!frame) {
        EV_ERROR << "message from physical layer (%s)%s is not a subclass of Ieee80211Frame" << msg->getClassName() << " " << msg->getName() << endl;
        delete msg;
        return;
        // throw cRuntimeError("message from physical layer (%s)%s is not a subclass of Ieee80211Frame",msg->getClassName(), msg->getName());
    }

    EV_DEBUG << "Self address: " << address
             << ", receiver address: " << frame->getReceiverAddress()
             << ", received frame is for us: " << isForUs(frame)
             << ", received frame was sent by us: " << isSentByUs(frame) << endl;

    Ieee80211TwoAddressFrame *twoAddressFrame = dynamic_cast<Ieee80211TwoAddressFrame *>(msg);
    ASSERT(!twoAddressFrame || twoAddressFrame->getTransmitterAddress() != address);

    handleWithFSM(msg);

    // if we are the owner then we did not send this message up
    if (msg->getOwner() == this)
        delete msg;
    EV_TRACE << "Leave handleLowerMsg...\n";
}

void Ieee80211Mac::receiveSignal(cComponent *source, simsignal_t signalID, long value)
{
    Enter_Method_Silent();
    if (signalID == IRadio::receptionStateChangedSignal)
        handleWithFSM(mediumStateChange);
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        handleWithFSM(mediumStateChange);
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        transmissionState = newRadioTransmissionState;
    }
}

/**
 * Msg can be upper, lower, self or NULL (when radio state changes)
 */
void Ieee80211Mac::handleWithFSM(cMessage *msg)
{
    removeOldTuplesFromDuplicateMap();
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (isUpperMessage(msg) && fsm.getState() != IDLE) {
        if (fsm.getState() == WAITAIFS && endDIFS->isScheduled()) {
            // a difs was schedule because all queues ware empty
            // change difs for aifs
            simtime_t remaint = getAIFS(currentAC) - getDIFS();
            scheduleAt(endDIFS->getArrivalTime() + remaint, endAIFS(currentAC));
            cancelEvent(endDIFS);
        }
        else if (fsm.getState() == BACKOFF && endBackoff(numCategories() - 1)->isScheduled() && transmissionQueue(numCategories() - 1)->empty()) {
            // a backoff was schedule with all the queues empty
            // reschedule the backoff with the appropriate AC
            backoffPeriod(currentAC) = backoffPeriod(numCategories() - 1);
            backoff(currentAC) = backoff(numCategories() - 1);
            backoff(numCategories() - 1) = false;
            scheduleAt(endBackoff(numCategories() - 1)->getArrivalTime(), endBackoff(currentAC));
            cancelEvent(endBackoff(numCategories() - 1));
        }
        EV_DEBUG << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
        return;
    }

    // Special case, is  endTimeout ACK and the radio state  is RECV, the system must wait until end reception (9.3.2.8 ACK procedure)
    if (msg == endTimeout && radio->getReceptionState() == IRadio::RECEPTION_STATE_RECEIVING && useModulationParameters && fsm.getState() == WAITACK)
    {
        EV << "Re-schedule WAITACK timeout \n";
        scheduleAt(simTime() + controlFrameTxTime(LENGTH_ACK), endTimeout);
        return;
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame*>(msg);
    int frameType = frame ? frame->getType() : -1;
    logState();
    stateVector.record(fsm.getState());

    bool receptionError = false;
    if (frame && isLowerMessage(frame)) {
        lastReceiveFailed = receptionError = frame ? frame->hasBitError() : false;
        scheduleReservePeriod(frame);
    }

    // TODO: fix bug according to the message: [omnetpp] A possible bug in the Ieee80211's FSM.
    FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            /*
               if (fixFSM)
               {
               FSMA_Event_Transition(Data-Ready,
                                  // isUpperMessage(msg),
                                  isUpperMessage(msg) && backoffPeriod[currentAC] > 0,
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
            FSMA_Event_Transition(Data - Ready,
                    isUpperMessage(msg),
                    DEFER,
                    ASSERT(isInvalidBackoffPeriod() || backoffPeriod() == SIMTIME_ZERO);
                    invalidateBackoffPeriod();
                    );
            FSMA_No_Event_Transition(Immediate - Data - Ready,
                    !transmissionQueueEmpty(),
                    DEFER,
                    );
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    );
        }
        FSMA_State(DEFER) {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Wait - AIFS,
                    isMediumStateChange(msg) && isMediumFree(),
                    WAITAIFS,
                    ;
                    );
            FSMA_No_Event_Transition(Immediate - Wait - AIFS,
                    isMediumFree() || (!isBackoffPending()),
                    WAITAIFS,
                    ;
                    );
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    ;
                    );
        }
        FSMA_State(WAITAIFS) {
            FSMA_Enter(scheduleAIFSPeriod());

            FSMA_Event_Transition(EDCAF - Do - Nothing,
                    isMsgAIFS(msg) && transmissionQueue()->empty(),
                    WAITAIFS,
                    ASSERT(0 == 1);
                    ;
                    );
            FSMA_Event_Transition(Immediate - Transmit - RTS,
                    isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                    && getCurrentTransmission()->getByteLength() >= rtsThreshold && !backoff(),
                    WAITCTS,
                    sendRTSFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    );
            FSMA_Event_Transition(Immediate - Transmit - Multicast,
                    isMsgAIFS(msg) && isMulticast(getCurrentTransmission()) && !backoff(),
                    WAITMULTICAST,
                    sendMulticastFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    );
            FSMA_Event_Transition(Immediate - Transmit - Data,
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
            FSMA_Event_Transition(AIFS - Over,
                    isMsgAIFS(msg),
                    BACKOFF,
                    if (isInvalidBackoffPeriod())
                        generateBackoffPeriod();
                    );
            // end the difs and no other packet has been received
            FSMA_Event_Transition(DIFS - Over,
                    msg == endDIFS && transmissionQueueEmpty(),
                    BACKOFF,
                    currentAC = numCategories() - 1;
                    if (isInvalidBackoffPeriod())
                        generateBackoffPeriod();
                    );
            FSMA_Event_Transition(DIFS - Over,
                    msg == endDIFS,
                    BACKOFF,
                    for (int i = numCategories() - 1; i >= 0; i--) {
                        if (!transmissionQueue(i)->empty()) {
                            currentAC = i;
                        }
                    }
                    if (isInvalidBackoffPeriod())
                        generateBackoffPeriod();
                    );
            FSMA_Event_Transition(Busy,
                    isMediumStateChange(msg) && !isMediumFree(),
                    DEFER,
                    for (int i = 0; i < numCategories(); i++) {
                        if (endAIFS(i)->isScheduled())
                            backoff(i) = true;
                    }
                    if (endDIFS->isScheduled())
                        backoff(numCategories() - 1) = true;
                    cancelAIFSPeriod();
                    );
            FSMA_No_Event_Transition(Immediate - Busy,
                    !isMediumFree(),
                    DEFER,
                    for (int i = 0; i < numCategories(); i++) {
                        if (endAIFS(i)->isScheduled())
                            backoff(i) = true;
                    }
                    if (endDIFS->isScheduled())
                        backoff(numCategories() - 1) = true;
                    cancelAIFSPeriod();

                    );
            // radio state changes before we actually get the message, so this must be here
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    cancelAIFSPeriod();
                    ;
                    );
        }
        FSMA_State(BACKOFF) {
            FSMA_Enter(scheduleBackoffPeriod());
            if (getCurrentTransmission()) {
                FSMA_Event_Transition(Transmit - RTS,
                        msg == endBackoff() && !isMulticast(getCurrentTransmission())
                        && getCurrentTransmission()->getByteLength() >= rtsThreshold,
                        WAITCTS,
                        sendRTSFrame(getCurrentTransmission());
                        oldcurrentAC = currentAC;
                        cancelAIFSPeriod();
                        decreaseBackoffPeriod();
                        cancelBackoffPeriod();
                        );
                FSMA_Event_Transition(Transmit - Multicast,
                        msg == endBackoff() && isMulticast(getCurrentTransmission()),
                        WAITMULTICAST,
                        sendMulticastFrame(getCurrentTransmission());
                        oldcurrentAC = currentAC;
                        cancelAIFSPeriod();
                        decreaseBackoffPeriod();
                        cancelBackoffPeriod();
                        );
                FSMA_Event_Transition(Transmit - Data,
                        msg == endBackoff() && !isMulticast(getCurrentTransmission()),
                        WAITACK,
                        sendDataFrame(getCurrentTransmission());
                        oldcurrentAC = currentAC;
                        cancelAIFSPeriod();
                        decreaseBackoffPeriod();
                        cancelBackoffPeriod();
                        );
            }
            FSMA_Event_Transition(AIFS - Over - backoff,
                    isMsgAIFS(msg) && backoff(),
                    BACKOFF,
                    if (isInvalidBackoffPeriod())
                        generateBackoffPeriod();
                    );
            FSMA_Event_Transition(AIFS - Immediate - Transmit - RTS,
                    isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                    && getCurrentTransmission()->getByteLength() >= rtsThreshold && !backoff(),
                    WAITCTS,
                    sendRTSFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    decreaseBackoffPeriod();
                    cancelBackoffPeriod();
                    );
            FSMA_Event_Transition(AIFS - Immediate - Transmit - Multicast,
                    isMsgAIFS(msg) && isMulticast(getCurrentTransmission()) && !backoff(),
                    WAITMULTICAST,
                    sendMulticastFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    decreaseBackoffPeriod();
                    cancelBackoffPeriod();
                    );
            FSMA_Event_Transition(AIFS - Immediate - Transmit - Data,
                    isMsgAIFS(msg) && !isMulticast(getCurrentTransmission()) && !backoff(),
                    WAITACK,
                    sendDataFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    decreaseBackoffPeriod();
                    cancelBackoffPeriod();
                    );
            FSMA_Event_Transition(Backoff - Idle,
                    isBackoffMsg(msg) && transmissionQueueEmpty(),
                    IDLE,
                    resetStateVariables();
                    );
            FSMA_Event_Transition(Backoff - Busy,
                    isMediumStateChange(msg) && !isMediumFree(),
                    DEFER,
                    cancelAIFSPeriod();
                    decreaseBackoffPeriod();
                    cancelBackoffPeriod();
                    );
        }
        FSMA_State(WAITACK) {
            FSMA_Enter(scheduleDataTimeoutPeriod(getCurrentTransmission()));
#ifndef DISABLEERRORACK
            FSMA_Event_Transition(Reception - ACK - failed,
                    isLowerMessage(msg) && receptionError && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                    IDLE,
                    currentAC = oldcurrentAC;
                    cancelTimeoutPeriod();
                    giveUpCurrentTransmission();
                    txop = false;
                    if (endTXOP->isScheduled())
                        cancelEvent(endTXOP);
                    );
            FSMA_Event_Transition(Reception - ACK - error,
                    isLowerMessage(msg) && receptionError,
                    DEFER,
                    currentAC = oldcurrentAC;
                    cancelTimeoutPeriod();
                    retryCurrentTransmission();
                    txop = false;
                    if (endTXOP->isScheduled())
                        cancelEvent(endTXOP);
                    );
#endif // ifndef DISABLEERRORACK
            FSMA_Event_Transition(Receive - ACK - TXOP - Empty,
                    isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK && txop && transmissionQueue(oldcurrentAC)->size() == 1,
                    DEFER,
                    currentAC = oldcurrentAC;
                    if (retryCounter() == 0)
                        numSentWithoutRetry()++;
                    numSent()++;
                    fr = getCurrentTransmission();
                    numBits += fr->getBitLength();
                    bits() += fr->getBitLength();
                    macDelay()->record(simTime() - fr->getMACArrive());
                    if (maxJitter() == SIMTIME_ZERO || maxJitter() < (simTime() - fr->getMACArrive()))
                        maxJitter() = simTime() - fr->getMACArrive();
                    if (minJitter() == SIMTIME_ZERO || minJitter() > (simTime() - fr->getMACArrive()))
                        minJitter() = simTime() - fr->getMACArrive();
                    EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() << endl;
                    numSentTXOP++;
                    cancelTimeoutPeriod();
                    finishCurrentTransmission();
                    resetCurrentBackOff();
                    txop = false;
                    if (endTXOP->isScheduled())
                        cancelEvent(endTXOP);
                    );
            FSMA_Event_Transition(Receive - ACK - TXOP,
                    isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK && txop,
                    WAITSIFS,
                    currentAC = oldcurrentAC;
                    if (retryCounter() == 0)
                        numSentWithoutRetry()++;
                    numSent()++;
                    fr = getCurrentTransmission();
                    numBits += fr->getBitLength();
                    bits() += fr->getBitLength();

                    macDelay()->record(simTime() - fr->getMACArrive());
                    if (maxJitter() == SIMTIME_ZERO || maxJitter() < (simTime() - fr->getMACArrive()))
                        maxJitter() = simTime() - fr->getMACArrive();
                    if (minJitter() == SIMTIME_ZERO || minJitter() > (simTime() - fr->getMACArrive()))
                        minJitter() = simTime() - fr->getMACArrive();
                    EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() << endl;
                    numSentTXOP++;
                    cancelTimeoutPeriod();
                    finishCurrentTransmission();
                    );
/*
            FSMA_Event_Transition(Receive-ACK,
                                  isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK,
                                  IDLE,
                                  currentAC=oldcurrentAC;
                                  if (retryCounter[currentAC] == 0) numSentWithoutRetry[currentAC]++;
                                  numSent[currentAC]++;
                                  fr=getCurrentTransmission();
                                  numBites += fr->getBitLength();
                                  bits[currentAC] += fr->getBitLength();

                                  macDelay[currentAC].record(simTime() - fr->getMACArrive());
                                  if (maxjitter[currentAC] == 0 || maxjitter[currentAC] < (simTime() - fr->getMACArrive())) maxjitter[currentAC]=simTime() - fr->getMACArrive();
                                      if (minjitter[currentAC] == 0 || minjitter[currentAC] > (simTime() - fr->getMACArrive())) minjitter[currentAC]=simTime() - fr->getMACArrive();
                                          EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;

                                          cancelTimeoutPeriod();
                                          finishCurrentTransmission();
                                         );

 */
            /*Ieee 802.11 2007 9.9.1.2 EDCA TXOPs*/
            FSMA_Event_Transition(Receive - ACK,
                    isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK,
                    DEFER,
                    currentAC = oldcurrentAC;
                    if (retryCounter() == 0)
                        numSentWithoutRetry()++;
                    numSent()++;
                    fr = getCurrentTransmission();
                    numBits += fr->getBitLength();
                    bits() += fr->getBitLength();

                    macDelay()->record(simTime() - fr->getMACArrive());
                    if (maxJitter() == SIMTIME_ZERO || maxJitter() < (simTime() - fr->getMACArrive()))
                        maxJitter() = simTime() - fr->getMACArrive();
                    if (minJitter() == SIMTIME_ZERO || minJitter() > (simTime() - fr->getMACArrive()))
                        minJitter() = simTime() - fr->getMACArrive();
                    EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() << endl;
                    cancelTimeoutPeriod();
                    finishCurrentTransmission();
                    resetCurrentBackOff();
                    );
            FSMA_Event_Transition(Transmit - Data - Failed,
                    msg == endTimeout && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                    IDLE,
                    currentAC = oldcurrentAC;
                    giveUpCurrentTransmission();
                    txop = false;
                    if (endTXOP->isScheduled())
                        cancelEvent(endTXOP);
                    );
            FSMA_Event_Transition(Receive - ACK - Timeout,
                    msg == endTimeout,
                    DEFER,
                    currentAC = oldcurrentAC;
                    retryCurrentTransmission();
                    txop = false;
                    if (endTXOP->isScheduled())
                        cancelEvent(endTXOP);
                    );
            FSMA_Event_Transition(Interrupted - ACK - Failure,
                    isLowerMessage(msg) && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                    RECEIVE,
                    currentAC = oldcurrentAC;
                    cancelTimeoutPeriod();
                    giveUpCurrentTransmission();
                    txop = false;
                    if (endTXOP->isScheduled())
                        cancelEvent(endTXOP);
                    );
            FSMA_Event_Transition(Retry - Interrupted - ACK,
                    isLowerMessage(msg),
                    RECEIVE,
                    currentAC = oldcurrentAC;
                    cancelTimeoutPeriod();
                    retryCurrentTransmission();
                    txop = false;
                    if (endTXOP->isScheduled())
                        cancelEvent(endTXOP);
                    );
        }
        // wait until multicast is sent
        FSMA_State(WAITMULTICAST) {
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
            FSMA_Event_Transition(Transmit - Multicast,
                    msg == endTimeout,
                    DEFER,
                    currentAC = oldcurrentAC;
                    fr = getCurrentTransmission();
                    numBits += fr->getBitLength();
                    bits() += fr->getBitLength();
                    finishCurrentTransmission();
                    numSentMulticast++;
                    resetCurrentBackOff();
                    );
        }
        // accoriding to 9.2.5.7 CTS procedure
        FSMA_State(WAITCTS) {
            FSMA_Enter(scheduleCTSTimeoutPeriod());
#ifndef DISABLEERRORACK
            FSMA_Event_Transition(Reception - CTS - Failed,
                    isLowerMessage(msg) && receptionError && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                    IDLE,
                    cancelTimeoutPeriod();
                    currentAC = oldcurrentAC;
                    giveUpCurrentTransmission();
                    );
            FSMA_Event_Transition(Reception - CTS - error,
                    isLowerMessage(msg) && receptionError,
                    DEFER,
                    cancelTimeoutPeriod();
                    currentAC = oldcurrentAC;
                    retryCurrentTransmission();
                    );
#endif // ifndef DISABLEERRORACK
            FSMA_Event_Transition(Receive - CTS,
                    isLowerMessage(msg) && isForUs(frame) && frameType == ST_CTS,
                    WAITSIFS,
                    cancelTimeoutPeriod();
                    );
            FSMA_Event_Transition(Transmit - RTS - Failed,
                    msg == endTimeout && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                    IDLE,
                    currentAC = oldcurrentAC;
                    giveUpCurrentTransmission();
                    );
            FSMA_Event_Transition(Receive - CTS - Timeout,
                    msg == endTimeout,
                    DEFER,
                    currentAC = oldcurrentAC;
                    retryCurrentTransmission();
                    );
        }
        FSMA_State(WAITSIFS) {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Transmit - Data - TXOP,
                    msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_ACK,
                    WAITACK,
                    sendDataFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    );
            FSMA_Event_Transition(Transmit - CTS,
                    msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_RTS,
                    IDLE,
                    sendCTSFrameOnEndSIFS();
                    finishReception();
                    );
            FSMA_Event_Transition(Transmit - DATA,
                    msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_CTS,
                    WAITACK,
                    sendDataFrameOnEndSIFS(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    );
            FSMA_Event_Transition(Transmit - ACK,
                    msg == endSIFS && isDataOrMgmtFrame(getFrameReceivedBeforeSIFS()),
                    IDLE,
                    sendACKFrameOnEndSIFS();
                    finishReception();
                    );
        }
        // this is not a real state
        FSMA_State(RECEIVE) {
            FSMA_No_Event_Transition(Immediate - Receive - Error,
                    isLowerMessage(msg) && receptionError,
                    IDLE,
                    EV_WARN << "received frame contains bit errors or collision, next wait period is EIFS\n";
                    numCollision++;
                    finishReception();
                    );
            FSMA_No_Event_Transition(Immediate - Receive - Multicast,
                    isLowerMessage(msg) && isMulticast(frame) && !isSentByUs(frame) && isDataOrMgmtFrame(frame),
                    IDLE,
                    sendUp(frame);
                    numReceivedMulticast++;
                    finishReception();
                    );
            FSMA_No_Event_Transition(Immediate - Receive - Data,
                    isLowerMessage(msg) && isForUs(frame) && isDataOrMgmtFrame(frame),
                    WAITSIFS,
                    sendUp(frame);
                    numReceived++;
                    );
            FSMA_No_Event_Transition(Immediate - Receive - RTS,
                    isLowerMessage(msg) && isForUs(frame) && frameType == ST_RTS,
                    WAITSIFS,
                    );
            FSMA_No_Event_Transition(Immediate - Receive - Other - backtobackoff,
                    isLowerMessage(msg) && isBackoffPending(),    //(backoff[0] || backoff[1] || backoff[2] || backoff[3]),
                    DEFER,
                    );

            FSMA_No_Event_Transition(Immediate - Promiscuous - Data,
                    isLowerMessage(msg) && !isForUs(frame) && isDataOrMgmtFrame(frame),
                    IDLE,
                    promiscousFrame(frame);
                    finishReception();
                    numReceivedOther++;
                    );
            FSMA_No_Event_Transition(Immediate - Receive - Other,
                    isLowerMessage(msg),
                    IDLE,
                    finishReception();
                    numReceivedOther++;
                    );
        }
    }
    EV_TRACE << "leaving handleWithFSM\n\t";
    logState();
    stateVector.record(fsm.getState());
    if (simTime() - last > 0.1) {
        for (int i = 0; i < numCategories(); i++) {
            throughput(i)->record(bits(i) / (simTime() - last));
            bits(i) = 0;
            if (maxJitter(i) > SIMTIME_ZERO && minJitter(i) > SIMTIME_ZERO) {
                jitter(i)->record(maxJitter(i) - minJitter(i));
                maxJitter(i) = SIMTIME_ZERO;
                minJitter(i) = SIMTIME_ZERO;
            }
        }
        last = simTime();
    }
}

void Ieee80211Mac::finishReception()
{
    if (getCurrentTransmission()) {
        backoff() = true;
    }
    else {
        resetStateVariables();
    }
}

/****************************************************************
 * Timing functions.
 */
simtime_t Ieee80211Mac::getSIFS()
{
// TODO:   return aRxRFDelay() + aRxPLCPDelay() + aMACProcessingDelay() + aRxTxTurnaroundTime();
    if (useModulationParameters) {
        ModulationType modType;
        modType = Ieee80211Descriptor::getModulationType(opMode, bitrate);
        return Ieee80211Modulation::getSifsTime(modType, wifiPreambleType);
    }

    return SIFS;
}

simtime_t Ieee80211Mac::getSlotTime()
{
// TODO:   return aCCATime() + aRxTxTurnaroundTime + aAirPropagationTime() + aMACProcessingDelay();
    if (useModulationParameters) {
        ModulationType modType;
        modType = Ieee80211Descriptor::getModulationType(opMode, bitrate);
        return Ieee80211Modulation::getSlotDuration(modType, wifiPreambleType);
    }
    return ST;
}

simtime_t Ieee80211Mac::getPIFS()
{
    return getSIFS() + getSlotTime();
}

simtime_t Ieee80211Mac::getDIFS(int category)
{
    if (category < 0 || category > (numCategories() - 1)) {
        int index = numCategories() - 1;
        if (index < 0)
            index = 0;
        return getSIFS() + ((double)AIFSN(index) * getSlotTime());
    }
    else {
        return getSIFS() + ((double)AIFSN(category)) * getSlotTime();
    }
}

simtime_t Ieee80211Mac::getHeaderTime(double bitrate)
{
    ModulationType modType;
    modType = Ieee80211Descriptor::getModulationType(opMode, bitrate);
    return Ieee80211Modulation::getPreambleAndHeader(modType, wifiPreambleType);
}

simtime_t Ieee80211Mac::getAIFS(int AccessCategory)
{
    return AIFSN(AccessCategory) * getSlotTime() + getSIFS();
}

simtime_t Ieee80211Mac::getEIFS()
{
    return getSIFS() + getDIFS() + controlFrameTxTime(LENGTH_ACK);
}

simtime_t Ieee80211Mac::computeBackoffPeriod(Ieee80211Frame *msg, int r)
{
    int cw;

    EV_DEBUG << "generating backoff slot number for retry: " << r << endl;
    if (msg && isMulticast(msg))
        cw = cwMinMulticast;
    else {
        ASSERT(0 <= r && r < transmissionLimit);

        cw = (cwMin() + 1) * (1 << r) - 1;

        if (cw > cwMax())
            cw = cwMax();
    }

    int c = intrand(cw + 1);

    EV_DEBUG << "generated backoff slot number: " << c << " , cw: " << cw << " ,cwMin:cwMax = " << cwMin() << ":" << cwMax() << endl;

    return ((double)c) * getSlotTime();
}

/****************************************************************
 * Timer functions.
 */
void Ieee80211Mac::scheduleSIFSPeriod(Ieee80211Frame *frame)
{
    EV_DEBUG << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + getSIFS(), endSIFS);
}

void Ieee80211Mac::scheduleDIFSPeriod()
{
    if (lastReceiveFailed) {
        EV_DEBUG << "reception of last frame failed, scheduling EIFS period\n";
        scheduleAt(simTime() + getEIFS(), endDIFS);
    }
    else {
        EV_DEBUG << "scheduling DIFS period\n";
        scheduleAt(simTime() + getDIFS(), endDIFS);
    }
}

void Ieee80211Mac::cancelDIFSPeriod()
{
    EV_DEBUG << "canceling DIFS period\n";
    cancelEvent(endDIFS);
}

void Ieee80211Mac::scheduleAIFSPeriod()
{
    bool schedule = false;
    for (int i = 0; i < numCategories(); i++) {
        if (!endAIFS(i)->isScheduled() && !transmissionQueue(i)->empty()) {
            if (lastReceiveFailed) {
                EV_DEBUG << "reception of last frame failed, scheduling EIFS-DIFS+AIFS period (" << i << ")\n";
                scheduleAt(simTime() + getEIFS() - getDIFS() + getAIFS(i), endAIFS(i));
            }
            else {
                EV_DEBUG << "scheduling AIFS period (" << i << ")\n";
                scheduleAt(simTime() + getAIFS(i), endAIFS(i));
            }
        }
        if (endAIFS(i)->isScheduled())
            schedule = true;
    }
    if (!schedule && !endDIFS->isScheduled()) {
        // schedule default DIFS
        currentAC = numCategories() - 1;
        scheduleDIFSPeriod();
    }
}

void Ieee80211Mac::rescheduleAIFSPeriod(int AccessCategory)
{
    ASSERT(1);
    EV_DEBUG << "rescheduling AIFS[" << AccessCategory << "]\n";
    cancelEvent(endAIFS(AccessCategory));
    scheduleAt(simTime() + getAIFS(AccessCategory), endAIFS(AccessCategory));
}

void Ieee80211Mac::cancelAIFSPeriod()
{
    EV_DEBUG << "canceling AIFS period\n";
    for (int i = 0; i < numCategories(); i++)
        cancelEvent(endAIFS(i));
    cancelEvent(endDIFS);
}

//XXXvoid Ieee80211Mac::checkInternalColision()
//{
//  EV_DEBUG << "We obtain endAIFS, so we have to check if there
//}

void Ieee80211Mac::scheduleDataTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    double tim;
    double bitRate = bitrate;
    if (dynamic_cast<RadioTransmissionRequest *>(frameToSend->getControlInfo())) {
        bitRate = dynamic_cast<RadioTransmissionRequest *>(frameToSend->getControlInfo())->getBitrate().get();
        if (bitRate == 0)
            bitRate = bitrate;
    }
    if (!endTimeout->isScheduled()) {
        EV_DEBUG << "scheduling data timeout period\n";
        if (useModulationParameters) {
            ModulationType modType;
            modType = Ieee80211Descriptor::getModulationType(opMode, bitRate);
            double duration = computeFrameDuration(frameToSend);
            double slot = SIMTIME_DBL(Ieee80211Modulation::getSlotDuration(modType, wifiPreambleType));
            double sifs = SIMTIME_DBL(Ieee80211Modulation::getSifsTime(modType, wifiPreambleType));
            double PHY_RX_START = SIMTIME_DBL(Ieee80211Modulation::get_aPHY_RX_START_Delay (modType, wifiPreambleType));
            tim = duration + slot + sifs + PHY_RX_START;
        }
        else
            tim = computeFrameDuration(frameToSend) + SIMTIME_DBL( getSlotTime()) +SIMTIME_DBL( getSIFS()) + controlFrameTxTime(LENGTH_ACK) + MAX_PROPAGATION_DELAY * 2;
        EV_DEBUG << " time out=" << tim*1e6 << "us" << endl;
        scheduleAt(simTime() + tim, endTimeout);
    }
}

void Ieee80211Mac::scheduleMulticastTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    if (!endTimeout->isScheduled()) {
        EV_DEBUG << "scheduling multicast timeout period\n";
        scheduleAt(simTime() + computeFrameDuration(frameToSend), endTimeout);
    }
}

void Ieee80211Mac::cancelTimeoutPeriod()
{
    EV_DEBUG << "canceling timeout period\n";
    cancelEvent(endTimeout);
}

void Ieee80211Mac::scheduleCTSTimeoutPeriod()
{
    if (!endTimeout->isScheduled()) {
        EV_DEBUG << "scheduling CTS timeout period\n";
        scheduleAt(simTime() + controlFrameTxTime(LENGTH_RTS) + getSIFS()
                   + controlFrameTxTime(LENGTH_CTS) + MAX_PROPAGATION_DELAY * 2, endTimeout);
    }
}

void Ieee80211Mac::scheduleReservePeriod(Ieee80211Frame *frame)
{
    simtime_t reserve = frame->getDuration();

    // see spec. 7.1.3.2
    if (!isForUs(frame) && reserve != 0 && reserve < 32768) {
        if (endReserve->isScheduled()) {
            simtime_t oldReserve = endReserve->getArrivalTime() - simTime();

            if (oldReserve > reserve)
                return;

            reserve = std::max(reserve, oldReserve);
            cancelEvent(endReserve);
        }
        else if (radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE) {
            // NAV: the channel just became virtually busy according to the spec
            scheduleAt(simTime(), mediumStateChange);
        }

        EV_DEBUG << "scheduling reserve period for: " << reserve << endl;

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
    ASSERT(backoffPeriod() >= SIMTIME_ZERO);
    EV_DEBUG << "backoff period set to " << backoffPeriod() << endl;
}

void Ieee80211Mac::decreaseBackoffPeriod()
{
    // see spec 9.9.1.5
    // decrase for every EDCAF
    // cancel event endBackoff after decrease or we don't know which endBackoff is scheduled
    for (int i = 0; i < numCategories(); i++) {
        if (backoff(i) && endBackoff(i)->isScheduled()) {
            EV_DEBUG << "old backoff[" << i << "] is " << backoffPeriod(i) << ", sim time is " << simTime()
                     << ", endbackoff sending period is " << endBackoff(i)->getSendingTime() << endl;
            simtime_t elapsedBackoffTime = simTime() - endBackoff(i)->getSendingTime();
            backoffPeriod(i) -= ((int)(elapsedBackoffTime / getSlotTime())) * getSlotTime();
            EV_DEBUG << "actual backoff[" << i << "] is " << backoffPeriod(i) << ", elapsed is " << elapsedBackoffTime << endl;
            ASSERT(backoffPeriod(i) >= SIMTIME_ZERO);
            EV_DEBUG << "backoff[" << i << "] period decreased to " << backoffPeriod(i) << endl;
        }
    }
}

void Ieee80211Mac::scheduleBackoffPeriod()
{
    EV_DEBUG << "scheduling backoff period\n";
    scheduleAt(simTime() + backoffPeriod(), endBackoff());
}

void Ieee80211Mac::cancelBackoffPeriod()
{
    EV_DEBUG << "cancelling Backoff period - only if some is scheduled\n";
    for (int i = 0; i < numCategories(); i++)
        cancelEvent(endBackoff(i));
}

/****************************************************************
 * Frame sender functions.
 */
void Ieee80211Mac::sendACKFrameOnEndSIFS()
{
    Ieee80211Frame *frameToACK = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendACKFrame(check_and_cast<Ieee80211DataOrMgmtFrame *>(frameToACK));
    delete frameToACK;
}

void Ieee80211Mac::sendACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    EV_INFO << "sending ACK frame\n";
    numAckSend++;
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(setControlBitrate(buildACKFrame(frameToACK)));
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
    std::list<Ieee80211DataOrMgmtFrame *>::iterator frame;

    frame = transmissionQueue()->begin();
    ASSERT(*frame == frameToSend);
    if (!txop && TXOP() > 0 && transmissionQueue()->size() >= 2) {
        //we start packet burst within TXOP time period
        txop = true;

        for (frame = transmissionQueue()->begin(); frame != transmissionQueue()->end(); ++frame) {
            count++;
            t = computeFrameDuration(*frame) + 2 * getSIFS() + controlFrameTxTime(LENGTH_ACK);
            EV_DEBUG << "t is " << t << endl;
            if (TXOP() > time + t) {
                time += t;
                EV_DEBUG << "adding t\n";
            }
            else {
                break;
            }
        }
        //to be sure we get endTXOP earlier then receive ACK and we have to minus SIFS time from first packet
        time -= getSIFS() / 2 + getSIFS();
        EV_DEBUG << "scheduling TXOP for AC" << currentAC << ", duration is " << time << ", count is " << count << endl;
        scheduleAt(simTime() + time, endTXOP);
    }
    EV_INFO << "sending Data frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(buildDataFrame(dynamic_cast<Ieee80211DataOrMgmtFrame *>(setBitrateFrame(frameToSend))));
}

void Ieee80211Mac::sendRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV_INFO << "sending RTS frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(setControlBitrate(buildRTSFrame(frameToSend)));
}

void Ieee80211Mac::sendMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV_INFO << "sending Multicast frame\n";
    if (frameToSend->getControlInfo())
        delete frameToSend->removeControlInfo();
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(buildDataFrame(dynamic_cast<Ieee80211DataOrMgmtFrame *>(setBasicBitrate(frameToSend))));
}

void Ieee80211Mac::sendCTSFrameOnEndSIFS()
{
    Ieee80211Frame *rtsFrame = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendCTSFrame(check_and_cast<Ieee80211RTSFrame *>(rtsFrame));
    delete rtsFrame;
}

void Ieee80211Mac::sendCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    EV_INFO << "sending CTS frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(setControlBitrate(buildCTSFrame(rtsFrame)));
}

/****************************************************************
 * Frame builder functions.
 */
Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    if (frameToSend->getControlInfo() != NULL) {
        cObject *ctr = frameToSend->getControlInfo();
        RadioTransmissionRequest *ctrl = dynamic_cast<RadioTransmissionRequest *>(ctr);
        if (ctrl == NULL)
            throw cRuntimeError("control info is not RadioTransmissionRequest but %s", ctr->getClassName());
        frame->setControlInfo(ctrl->dup());
    }
    if (isMulticast(frameToSend))
        frame->setDuration(0);
    else if (!frameToSend->getMoreFragments()) {
        if (txop && transmissionQueue()->size() > 1) {
            // ++ operation is safe because txop is true
            std::list<Ieee80211DataOrMgmtFrame *>::iterator nextframeToSend;
            nextframeToSend = transmissionQueue()->begin();
            nextframeToSend++;
            ASSERT(transmissionQueue()->end() != nextframeToSend);
            double bitRate = bitrate;
            int size = (*nextframeToSend)->getBitLength();
            if (transmissionQueue()->front()->getControlInfo() && dynamic_cast<RadioTransmissionRequest *>(transmissionQueue()->front()->getControlInfo())) {
                bitRate = dynamic_cast<RadioTransmissionRequest *>(transmissionQueue()->front()->getControlInfo())->getBitrate().get();
                if (bitRate == 0)
                    bitRate = bitrate;
            }
            frame->setDuration(3 * getSIFS() + 2 * controlFrameTxTime(LENGTH_ACK) + computeFrameDuration(size, bitRate));
        }
        else
            frame->setDuration(getSIFS() + controlFrameTxTime(LENGTH_ACK));
    }
    else
        // FIXME: shouldn't we use the next frame to be sent?
        frame->setDuration(3 * getSIFS() + 2 * controlFrameTxTime(LENGTH_ACK) + computeFrameDuration(frameToSend));

    return frame;
}

Ieee80211ACKFrame *Ieee80211Mac::buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    Ieee80211ACKFrame *frame = new Ieee80211ACKFrame("wlan-ack");
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());

    if (!frameToACK->getMoreFragments())
        frame->setDuration(0);
    else
        frame->setDuration(frameToACK->getDuration() - getSIFS() - controlFrameTxTime(LENGTH_ACK));

    return frame;
}

Ieee80211RTSFrame *Ieee80211Mac::buildRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211RTSFrame *frame = new Ieee80211RTSFrame("wlan-rts");
    frame->setTransmitterAddress(address);
    frame->setReceiverAddress(frameToSend->getReceiverAddress());
    frame->setDuration(3 * getSIFS() + controlFrameTxTime(LENGTH_CTS)
            + computeFrameDuration(frameToSend)
            + controlFrameTxTime(LENGTH_ACK));

    return frame;
}

Ieee80211CTSFrame *Ieee80211Mac::buildCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("wlan-cts");
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - getSIFS() - controlFrameTxTime(LENGTH_CTS));

    return frame;
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    RadioTransmissionRequest *oldTransmissionRequest = dynamic_cast<RadioTransmissionRequest *>(frameToSend->getControlInfo());
    if (oldTransmissionRequest) {
        EV_DEBUG << "Per frame1 params" << endl;
        RadioTransmissionRequest *newTransmissionRequest = new RadioTransmissionRequest();
        *newTransmissionRequest = *oldTransmissionRequest;
        frame->setControlInfo(newTransmissionRequest);
    }

    frame->setDuration(0);
    return frame;
}

Ieee80211Frame *Ieee80211Mac::setBasicBitrate(Ieee80211Frame *frame)
{
    ASSERT(frame->getControlInfo() == NULL);
    RadioTransmissionRequest *ctrl = new RadioTransmissionRequest();
    ctrl->setBitrate(bps(basicBitrate));
    frame->setControlInfo(ctrl);
    return frame;
}

Ieee80211Frame *Ieee80211Mac::setControlBitrate(Ieee80211Frame *frame)
{
    ASSERT(frame->getControlInfo()==NULL);
    RadioTransmissionRequest *ctrl = new RadioTransmissionRequest();
    ctrl->setBitrate((bps)controlBitRate);
    frame->setControlInfo(ctrl);
    return frame;
}

Ieee80211Frame *Ieee80211Mac::setBitrateFrame(Ieee80211Frame *frame)
{
    if (rateControlMode == RATE_CR && forceBitRate == false) {
        if (frame->getControlInfo())
            delete frame->removeControlInfo();
        return frame;
    }
    RadioTransmissionRequest *ctrl = NULL;
    if (frame->getControlInfo() == NULL) {
        ctrl = new RadioTransmissionRequest();
        frame->setControlInfo(ctrl);
    }
    else
        ctrl = dynamic_cast<RadioTransmissionRequest *>(frame->getControlInfo());
    if (ctrl)
        ctrl->setBitrate(bps(getBitrate()));
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
    Ieee80211DataOrMgmtFrame *temp = (Ieee80211DataOrMgmtFrame *)transmissionQueue()->front();
    emit(NF_LINK_BREAK, temp);
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
        retryCounter()++;
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
    if (pendingRadioConfigMsg != NULL) {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = NULL;
    }
}

void Ieee80211Mac::setMode(Mode mode)
{
    if (mode == PCF)
        throw cRuntimeError("PCF mode not yet supported");

    this->mode = mode;
}

void Ieee80211Mac::resetStateVariables()
{
    backoffPeriod() = SIMTIME_ZERO;
    if (rateControlMode == RATE_AARF || rateControlMode == RATE_ARF)
        reportDataOk();
    else
        retryCounter() = 0;

    if (!transmissionQueue()->empty()) {
        backoff() = true;
        getCurrentTransmission()->setRetry(false);
    }
    else {
        backoff() = false;
    }
}

bool Ieee80211Mac::isMediumStateChange(cMessage *msg)
{
    return msg == mediumStateChange || (msg == endReserve && radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE);
}

bool Ieee80211Mac::isMediumFree()
{
    return !endReserve->isScheduled() && radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
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
    if (dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
        //EV_DEBUG << "ad3 "<<((Ieee80211DataOrMgmtFrame *)frame)->getAddress3();
        //EV_DEBUG << "myad "<<address<<endl;
        if (((Ieee80211DataOrMgmtFrame *)frame)->getAddress3() == address) //received frame sent by us
            return 1;
    }
    else
        EV_ERROR << "Cast failed" << endl; // WTF? (levy)

    return 0;
}

bool Ieee80211Mac::isDataOrMgmtFrame(Ieee80211Frame *frame)
{
    return dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame);
}

bool Ieee80211Mac::isMsgAIFS(cMessage *msg)
{
    for (int i = 0; i < numCategories(); i++)
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
    EV_DEBUG << "dropping frame from transmission queue\n";
    Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *>(transmissionQueue()->front());
    ASSERT(!transmissionQueue()->empty());
    transmissionQueue()->pop_front();
    if (queueModule) {
        if (numCategories() == 1) {
            // the module are continuously asking for packets
            EV_DEBUG << "requesting another frame from queue module\n";
            queueModule->requestPacket();
        }
        else if (numCategories() > 1 && (int)transmissionQueueSize() == maxQueueSize - 1) {
            // Now exist a empty frame space
            // the module are continuously asking for packets
            EV_DEBUG << "requesting another frame from queue module\n";
            queueModule->requestPacket();
        }
    }
    delete temp;
}

double Ieee80211Mac::computeFrameDuration(Ieee80211Frame *msg)
{
    RadioTransmissionRequest *ctrl;
    double duration;
    EV_DEBUG << *msg;
    ctrl = dynamic_cast<RadioTransmissionRequest *>(msg->removeControlInfo());
    if (ctrl) {
        EV_DEBUG << "Per frame2 params bitrate " << ctrl->getBitrate().get() / 1e6 << "Mb" << endl;
        duration = computeFrameDuration(msg->getBitLength(), ctrl->getBitrate().get());
        delete ctrl;
        return duration;
    }
    else
        return computeFrameDuration(msg->getBitLength(), bitrate);
}

double Ieee80211Mac::computeFrameDuration(int bits, double bitrate)
{
    double duration;
    ModulationType modType;
    modType = Ieee80211Descriptor::getModulationType(opMode, bitrate);
    if (PHY_HEADER_LENGTH < 0)
        duration = SIMTIME_DBL(Ieee80211Modulation::calculateTxDuration(bits, modType, wifiPreambleType));
    else
        duration = SIMTIME_DBL(Ieee80211Modulation::getPayloadDuration(bits, modType)) + PHY_HEADER_LENGTH;

    EV_DEBUG << " duration=" << duration * 1e6 << "us(" << bits << "bits " << bitrate / 1e6 << "Mbps)" << endl;
    return duration;
}

void Ieee80211Mac::logState()
{
    int numCategs = numCategories();
    EV_TRACE << "# state information: mode = " << modeName(mode) << ", state = " << fsm.getStateName();
    EV_TRACE << ", backoff 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << edcCAF[i].backoff;
    EV_TRACE << "\n# backoffPeriod 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << edcCAF[i].backoffPeriod;
    EV_TRACE << "\n# retryCounter 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << edcCAF[i].retryCounter;
    EV_TRACE << ", radioMode = " << radio->getRadioMode()
             << ", receptionState = " << radio->getReceptionState()
             << ", transmissionState = " << radio->getTransmissionState()
             << ", nav = " << nav << ", txop is " << txop << "\n";
    EV_TRACE << "#queue size 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << transmissionQueue(i)->size();
    EV_TRACE << ", medium is " << (isMediumFree() ? "free" : "busy") << ", scheduled AIFS are";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << i << "(" << (edcCAF[i].endAIFS->isScheduled() ? "scheduled" : "") << ")";
    EV_TRACE << ", scheduled backoff are";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << i << "(" << (edcCAF[i].endBackoff->isScheduled() ? "scheduled" : "") << ")";
    EV_TRACE << "\n# currentAC: " << currentAC << ", oldcurrentAC: " << oldcurrentAC;
    if (getCurrentTransmission() != NULL)
        EV_TRACE << "\n# current transmission: " << getCurrentTransmission()->getId();
    else
        EV_TRACE << "\n# current transmission: none";
    EV_TRACE << endl;
}

const char *Ieee80211Mac::modeName(int mode)
{
#define CASE(x)    case x: \
        s = #x; break
    const char *s = "???";
    switch (mode) {
        CASE(DCF);
        CASE(PCF);
    }
    return s;
#undef CASE
}

bool Ieee80211Mac::transmissionQueueEmpty()
{
    for (int i = 0; i < numCategories(); i++)
        if (!transmissionQueue(i)->empty())
            return false;

    return true;
}

unsigned int Ieee80211Mac::transmissionQueueSize()
{
    unsigned int totalSize = 0;
    for (int i = 0; i < numCategories(); i++)
        totalSize += transmissionQueue(i)->size();
    return totalSize;
}

void Ieee80211Mac::flushQueue()
{
    if (queueModule) {
        while (!queueModule->isEmpty()) {
            cMessage *msg = queueModule->pop();
            //TODO emit(dropPkIfaceDownSignal, msg); -- 'pkDropped' signals are missing in this module!
            delete msg;
        }
        queueModule->clear();    // clear request count
    }

    for (int i = 0; i < numCategories(); i++) {
        while (!transmissionQueue(i)->empty()) {
            cMessage *msg = transmissionQueue(i)->front();
            transmissionQueue(i)->pop_front();
            //TODO emit(dropPkIfaceDownSignal, msg); -- 'pkDropped' signals are missing in this module!
            delete msg;
        }
    }
}

void Ieee80211Mac::clearQueue()
{
    if (queueModule) {
        queueModule->clear();    // clear request count
    }

    for (int i = 0; i < numCategories(); i++) {
        while (!transmissionQueue(i)->empty()) {
            cMessage *msg = transmissionQueue(i)->front();
            transmissionQueue(i)->pop_front();
            delete msg;
        }
    }
}

void Ieee80211Mac::reportDataOk()
{
    retryCounter() = 0;
    if (rateControlMode == RATE_CR)
        return;
    successCounter++;
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
    if (rateControlMode == RATE_CR)
        return;
    timer++;
    failedCounter++;
    successCounter = 0;
    if (recovery) {
        if (retryCounter() == 1) {
            reportRecoveryFailure();
            if (Ieee80211Descriptor::decIdx(rateIndex))
                setBitrate(Ieee80211Descriptor::getDescriptor(rateIndex).bitrate);
        }
        timer = 0;
    }
    else {
        if (needNormalFallback()) {
            reportFailure();
            if (Ieee80211Descriptor::decIdx(rateIndex))
                setBitrate(Ieee80211Descriptor::getDescriptor(rateIndex).bitrate);
        }
        if (retryCounter() >= 2) {
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
        throw cRuntimeError("timer_timeout is less than minTimerTimeout");
}

void Ieee80211Mac::setSuccessThreshold(int success_threshold)
{
    if (success_threshold >= minSuccessThreshold)
        successThreshold = success_threshold;
    else
        throw cRuntimeError("success_threshold is less than minSuccessThreshold");
}

void Ieee80211Mac::reportRecoveryFailure(void)
{
    if (rateControlMode == RATE_AARF) {
        setSuccessThreshold((int)(std::min((double)getSuccessThreshold() * successCoeff, (double)maxSuccessThreshold)));
        setTimerTimeout((int)(std::max((double)getMinTimerTimeout(), (double)(getSuccessThreshold() * timerCoeff))));
    }
}

void Ieee80211Mac::reportFailure(void)
{
    if (rateControlMode == RATE_AARF) {
        setTimerTimeout(getMinTimerTimeout());
        setSuccessThreshold(getMinSuccessThreshold());
    }
}

bool Ieee80211Mac::needRecoveryFallback(void)
{
    if (retryCounter() == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool Ieee80211Mac::needNormalFallback(void)
{
    int retryMod = (retryCounter() - 1) % 2;
    if (retryMod == 1) {
        return true;
    }
    else {
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
bool& Ieee80211Mac::backoff(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].backoff;
}

simtime_t& Ieee80211Mac::TXOP(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].TXOP;
}

simtime_t& Ieee80211Mac::backoffPeriod(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].backoffPeriod;
}

int& Ieee80211Mac::retryCounter(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].retryCounter;
}

int& Ieee80211Mac::AIFSN(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].AIFSN;
}

int& Ieee80211Mac::cwMax(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].cwMax;
}

int& Ieee80211Mac::cwMin(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].cwMin;
}

cMessage *Ieee80211Mac::endAIFS(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].endAIFS;
}

cMessage *Ieee80211Mac::endBackoff(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].endBackoff;
}

const bool Ieee80211Mac::isBackoffMsg(cMessage *msg)
{
    for (unsigned int i = 0; i < edcCAF.size(); i++) {
        if (msg == edcCAF[i].endBackoff)
            return true;
    }
    return false;
}

// Statistics
long& Ieee80211Mac::numRetry(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numRetry;
}

long& Ieee80211Mac::numSentWithoutRetry(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)numCategories())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numSentWithoutRetry;
}

long& Ieee80211Mac::numGivenUp(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numGivenUp;
}

long& Ieee80211Mac::numSent(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numSent;
}

long& Ieee80211Mac::numDropped(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numDropped;
}

long& Ieee80211Mac::bits(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].bits;
}

simtime_t& Ieee80211Mac::minJitter(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].minjitter;
}

simtime_t& Ieee80211Mac::maxJitter(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].maxjitter;
}

// out vectors

cOutVector *Ieee80211Mac::jitter(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAFOutVector[i].jitter;
}

cOutVector *Ieee80211Mac::macDelay(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAFOutVector[i].macDelay;
}

cOutVector *Ieee80211Mac::throughput(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAFOutVector[i].throughput;
}

Ieee80211Mac::Ieee80211DataOrMgmtFrameList *Ieee80211Mac::transmissionQueue(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return &(edcCAF[i].transmissionQueue);
}

ModulationType Ieee80211Mac::getControlAnswerMode(ModulationType reqMode)
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
    for (int idx = Ieee80211Descriptor::getMinIdx(opMode); idx < Ieee80211Descriptor::size(); idx++) {
        if (Ieee80211Descriptor::getDescriptor(idx).mode != opMode)
            break;
        ModulationType thismode;
        thismode = Ieee80211Descriptor::getModulationType(opMode, Ieee80211Descriptor::getDescriptor(idx).bitrate);

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
    if (!found) {
        throw cRuntimeError("Can't find response rate for reqMode. Check standard and selected rates match.");
    }

    return mode;
}

// This methods implemet the duplicate filter
void Ieee80211Mac::sendUp(cMessage *msg)
{
    EV_INFO << "sending up " << msg << "\n";

    if (!isDuplicated(msg)) {    // duplicate detection filter
        if (msg->isPacket())
            emit(packetSentToUpperSignal, msg);

        send(msg, upperLayerOutGateId);
    }
}

void Ieee80211Mac::removeOldTuplesFromDuplicateMap()
{
    if (duplicateDetect && lastTimeDelete + duplicateTimeOut >= simTime()) {
        lastTimeDelete = simTime();
        for (Ieee80211ASFTupleList::iterator it = asfTuplesList.begin(); it != asfTuplesList.end(); ) {
            if (it->second.receivedTime + duplicateTimeOut < simTime()) {
                Ieee80211ASFTupleList::iterator itAux = it;
                it++;
                asfTuplesList.erase(itAux);
            }
            else
                it++;
        }
    }
}

const MACAddress& Ieee80211Mac::isInterfaceRegistered()
{
    if (!par("multiMac").boolValue())
        return MACAddress::UNSPECIFIED_ADDRESS;

    IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (!ift)
        return MACAddress::UNSPECIFIED_ADDRESS;
    std::string interfaceName = utils::stripnonalnum(getParentModule()->getFullName());
    InterfaceEntry *e = ift->getInterfaceByName(interfaceName.c_str());
    if (e)
        return e->getMacAddress();
    return MACAddress::UNSPECIFIED_ADDRESS;
}

bool Ieee80211Mac::isDuplicated(cMessage *msg)
{
    if (duplicateDetect) {    // duplicate detection filter
        Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
        if (frame) {
            Ieee80211ASFTupleList::iterator it = asfTuplesList.find(frame->getTransmitterAddress());
            if (it == asfTuplesList.end()) {
                Ieee80211ASFTuple tuple;
                tuple.receivedTime = simTime();
                tuple.sequenceNumber = frame->getSequenceNumber();
                tuple.fragmentNumber = frame->getFragmentNumber();
                asfTuplesList.insert(std::pair<MACAddress, Ieee80211ASFTuple>(frame->getTransmitterAddress(), tuple));
            }
            else {
                // check if duplicate
                if (it->second.sequenceNumber == frame->getSequenceNumber()
                    && it->second.fragmentNumber == frame->getFragmentNumber())
                {
                    return true;
                }
                else {
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
        emit(NF_LINK_PROMISCUOUS, msg);
}

bool Ieee80211Mac::isBackoffPending()
{
    for (unsigned int i = 0; i < edcCAF.size(); i++) {
        if (edcCAF[i].backoff)
            return true;
    }
    return false;
}

double Ieee80211Mac::controlFrameTxTime(int bits)
{
     double duration;
     if (PHY_HEADER_LENGTH < 0)
         duration = SIMTIME_DBL(Ieee80211Modulation::calculateTxDuration(bits, controlFrameModulationType, wifiPreambleType));
     else
         duration = SIMTIME_DBL(Ieee80211Modulation::getPayloadDuration(bits, controlFrameModulationType)) + PHY_HEADER_LENGTH;

     EV_DEBUG << " duration=" << duration*1e6 << "us(" << bits << "bits " << controlFrameModulationType.getPhyRate()/1e6 << "Mbps)" << endl;
     return duration;
}

bool Ieee80211Mac::handleNodeStart(IDoneCallback *doneCallback)
{
    if (!doneCallback)
        return true; // do nothing when called from initialize() //FIXME It's a hack, should remove the initializeQueueModule() and setRadioMode() calls from initialize()

    bool ret = MACProtocolBase::handleNodeStart(doneCallback);
    initializeQueueModule();
    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
    return ret;
}

bool Ieee80211Mac::handleNodeShutdown(IDoneCallback *doneCallback)
{
    bool ret = MACProtocolBase::handleNodeStart(doneCallback);
    handleNodeCrash();
    return ret;
}

void Ieee80211Mac::handleNodeCrash()
{
    cancelEvent(endSIFS);
    cancelEvent(endDIFS);
    cancelEvent(endTimeout);
    cancelEvent(endReserve);
    cancelEvent(mediumStateChange);
    cancelEvent(endTXOP);
    for (unsigned int i = 0; i < edcCAF.size(); i++) {
        cancelEvent(endAIFS(i));
        cancelEvent(endBackoff(i));
        while (!transmissionQueue(i)->empty()) {
            Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *>(transmissionQueue(i)->front());
            transmissionQueue(i)->pop_front();
            delete temp;
        }
    }
}

} // namespace ieee80211

} // namespace inet

