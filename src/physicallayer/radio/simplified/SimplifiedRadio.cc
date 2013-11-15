//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Copyright (C) 2009 Juan-Carlos Maureira
//     - Multi Radio Support
//     - Drawing Interference and Sensitivity circles
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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

#include "SimplifiedRadio.h"
#include "ModuleAccess.h"
#include "PhyControlInfo_m.h"
#include "Radio80211aControlInfo_m.h"
#include "NodeStatus.h"
#include "NodeOperations.h"

simsignal_t SimplifiedRadio::bitrateSignal = registerSignal("bitrate");
simsignal_t SimplifiedRadio::lossRateSignal = registerSignal("lossRate");
simsignal_t SimplifiedRadio::changeLevelNoise = registerSignal("changeLevelNoise");

#define MIN_DISTANCE 0.001 // minimum distance 1 millimeter
#define BASE_NOISE_LEVEL (noiseGenerator?noiseLevel+noiseGenerator->noiseLevel():noiseLevel)

Define_Module(SimplifiedRadio);

SimplifiedRadio::SimplifiedRadio()
{
    obstacles = NULL;
    radioModel = NULL;
    receptionModel = NULL;
    transmitterConnected = true;
    receiverConnected = true;
    updateString = NULL;
    noiseGenerator = NULL;
}

void SimplifiedRadio::initialize(int stage)
{
    SimplifiedRadioChannelAccess::initialize(stage);

    EV << "Initializing Radio, stage=" << stage << endl;

    if (stage == INITSTAGE_LOCAL)
    {
        endTransmissionTimer = new cMessage("endTransmission");

        getSensitivityList(par("SensitivityTable").xmlValue());

        // read parameters
        transmitterPower = par("transmitterPower");
        if (transmitterPower > (double) (getRadioChannelPar("pMax")))
            error("transmitterPower cannot be bigger than pMax in SimplifiedRadioChannel!");
        bitrate = par("bitrate");
        thermalNoise = FWMath::dBm2mW(par("thermalNoise"));
        sensitivity = FWMath::dBm2mW(par("sensitivity"));
        carrierFrequency = par("carrierFrequency");

        // initialize noiseLevel
        noiseLevel = thermalNoise;
        std::string noiseModel =  par("NoiseGenerator").stdstringValue();
        if (noiseModel!="")
        {
            noiseGenerator = (INoiseGenerator *) createOne(noiseModel.c_str());
            noiseGenerator->initializeFrom(this);
            // register to get a notification when position changes
            subscribe(changeLevelNoise, this); // the INoiseGenerator must send a signal to this module
        }

        EV << "Initialized channel with noise: " << noiseLevel << " sensitivity: " << sensitivity <<
        endl;

        // initialize the pointer of the snrInfo with NULL to indicate
        // that currently no message is received
        snrInfo.ptr = NULL;

        // no channel switch pending
        newChannel = -1;

        // statistics
        numGivenUp = 0;
        numReceivedCorrectly = 0;

        WATCH(noiseLevel);

        obstacles = ObstacleControlAccess().getIfExists();
        if (obstacles) EV << "Found ObstacleControl" << endl;

        // this is the parameter of the radio channel (global)
        std::string propModel = getRadioChannelPar("propagationModel").stdstringValue();
        if (propModel == "")
            propModel = "FreeSpaceModel";
        doubleRayCoverage = false;
        if (propModel == "TwoRayGroundModel")
            doubleRayCoverage = true;

        receptionModel = (IReceptionModel *) createOne(propModel.c_str());
        receptionModel->initializeFrom(this);

        // adjust the sensitivity in function of maxDistance and reception model
        if (par("maxDistance").doubleValue() > 0)
        {
            if (sensitivityList.size() == 1 && sensitivityList.begin()->second == sensitivity)
            {
                sensitivity = receptionModel->calculateReceivedPower(transmitterPower, carrierFrequency, par("maxDistance").doubleValue());
                sensitivityList[0] = sensitivity;
            }
        }

        receptionThreshold = sensitivity;
        if (par("setReceptionThreshold").boolValue())
        {
            receptionThreshold = FWMath::dBm2mW(par("receptionThreshold").doubleValue());
            if (par("maxDistantReceptionThreshold").doubleValue() > 0)
            {
                receptionThreshold = receptionModel->calculateReceivedPower(transmitterPower, carrierFrequency, par("maxDistantReceptionThreshold").doubleValue());
            }
        }

        // radio model to handle frame length and reception success calculation (modulation, error correction etc.)
        std::string rModel = par("radioModel").stdstringValue();
        if (rModel=="")
            rModel = "GenericRadioModel";

        radioModel = (IRadioModel *) createOne(rModel.c_str());
        radioModel->initializeFrom(this);

        if (this->hasPar("drawCoverage"))
            drawCoverage = par("drawCoverage");
        else
            drawCoverage = false;
        if (this->hasPar("refreshCoverageInterval"))
            updateStringInterval = par("refreshCoverageInterval");
        else
            updateStringInterval = 0;
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (isOperational)
        {
            setRadioChannel(par("channelNumber"));

            // statistics
            emit(bitrateSignal, bitrate);
        }
        // draw the interference distance
        this->updateDisplayString();
    }
}

void SimplifiedRadio::finish()
{
}

SimplifiedRadio::~SimplifiedRadio()
{
    delete radioModel;
    delete receptionModel;
    if (noiseGenerator)
        delete noiseGenerator;
    if (endTransmissionTimer)
        cancelAndDelete(endTransmissionTimer);
    if (updateString)
        cancelAndDelete(updateString);
    // delete messages being received
    for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
        delete it->first;
}

bool SimplifiedRadio::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_PHYSICAL_LAYER)
            setRadioMode(RADIO_MODE_OFF);  //FIXME only if the interface is up, too; also: connectReceiver(), etc.
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_PHYSICAL_LAYER)
            setRadioMode(RADIO_MODE_OFF);
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_LOCAL)  // crash is immediate
            setRadioMode(RADIO_MODE_OFF);
    }
    else
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    return true;
}

bool SimplifiedRadio::processRadioFrame(SimplifiedRadioFrame *radioFrame)
{
    return radioFrame->getChannelNumber() == radioChannel;
}

/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * Messages from the channel are also buffered here in order to
 * simulate a transmission delay
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleUpperMsg, handleLowerMsgStart, handleLowerMsgEnd,
 * handleSelfMsg
 */
void SimplifiedRadio::handleMessage(cMessage *msg)
{
    if (radioMode == RADIO_MODE_SLEEP || radioMode == RADIO_MODE_OFF)
    {
        if (msg->getArrivalGate() == upperLayerIn || msg->isSelfMessage())  //XXX can we ensure we don't receive pk from upper in OFF state?? (race condition)
            throw cRuntimeError("Radio is turned off");
        else {
            EV << "Radio is turned off, dropping packet\n";
            delete msg;
            return;
        }
    }
    // handle commands
    if (updateString && updateString==msg)
    {
        this->updateDisplayString();
        return;
    }
    if (msg->getArrivalGate() == upperLayerIn && !msg->isPacket() /*FIXME XXX ENSURE REALLY PLAIN cMessage ARE SENT AS COMMANDS!!! && msg->getBitLength()==0*/)
    {
        cObject *ctrl = msg->removeControlInfo();
        if (msg->getKind()==0)
            error("Message '%s' with length==0 is supposed to be a command, but msg kind is also zero", msg->getName());
        handleCommand(msg->getKind(), ctrl);
        delete msg;
        return;
    }

    if (msg->getArrivalGate() == upperLayerIn)
    {
        SimplifiedRadioFrame *radioFrame = encapsulatePacket(PK(msg));
        handleUpperMsg(radioFrame);
    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
    else if (processRadioFrame(check_and_cast<SimplifiedRadioFrame*>(msg)))
    {
        if (receiverConnected)
        {
            SimplifiedRadioFrame *radioFrame = (SimplifiedRadioFrame *) msg;
            handleLowerMsgStart(radioFrame);
            bufferMsg(radioFrame);
            updateRadioChannelState();
        }
        else
        {
            EV << "Radio disabled. ignoring radio frame" << endl;
            delete msg;
        }
    }
    else
    {
        EV << "listening to different channel when receiving message -- dropping it\n";
        delete msg;
    }
}

/**
 * The packet is put in a buffer for the time the transmission would
 * last in reality. A timer indicates when the transmission is
 * complete. So, look at unbufferMsg to see what happens when the
 * transmission is complete..
 */
void SimplifiedRadio::bufferMsg(SimplifiedRadioFrame *radioFrame) //FIXME: add explicit simtime_t atTime arg?
{
    // set timer to indicate transmission is complete
    cMessage *endReceptionTimer = new cMessage("endReception");
    endReceptionTimer->setContextPointer(radioFrame);
    radioFrame->setContextPointer(endReceptionTimer);
    endReceptionTimers.push_back(endReceptionTimer);

    // NOTE: use arrivalTime instead of simTime, because we might be calling this
    // function during a channel change, when we're picking up ongoing transmissions
    // on the channel -- and then the message's arrival time is in the past!
    scheduleAt(radioFrame->getArrivalTime() + radioFrame->getDuration(), endReceptionTimer);
}

SimplifiedRadioFrame *SimplifiedRadio::encapsulatePacket(cPacket *frame)
{
    PhyControlInfo *ctrl = dynamic_cast<PhyControlInfo *>(frame->removeControlInfo());
    ASSERT(!ctrl || ctrl->getChannelNumber()==-1); // per-packet channel switching not supported

    // Note: we don't set length() of the SimplifiedRadioFrame, because duration will be used everywhere instead
    //if (ctrl && ctrl->getAdaptiveSensitivity()) updateSensitivity(ctrl->getBitrate());
    SimplifiedRadioFrame *radioFrame = createRadioFrame();
    radioFrame->setName(frame->getName());
    radioFrame->setPSend(transmitterPower);
    radioFrame->setChannelNumber(radioChannel);
    radioFrame->encapsulate(frame);
    radioFrame->setBitrate(ctrl ? ctrl->getBitrate() : bitrate);
    radioFrame->setDuration(radioModel->calculateDuration(radioFrame));
    radioFrame->setSenderPos(getRadioPosition());
    radioFrame->setCarrierFrequency(carrierFrequency);
    delete ctrl;

    EV << "Frame (" << frame->getClassName() << ")" << frame->getName()
    << " will be transmitted at " << (radioFrame->getBitrate()/1e6) << "Mbps\n";
    return radioFrame;
}

void SimplifiedRadio::sendUp(SimplifiedRadioFrame *radioFrame)
{
    cPacket *frame = radioFrame->decapsulate();
    Radio80211aControlInfo * cinfo = new Radio80211aControlInfo;
    if (radioModel->haveTestFrame())
    {
        cinfo->setAirtimeMetric(true);
        cinfo->setTestFrameDuration(radioModel->calculateDurationTestFrame(radioFrame));
        double snirMin = pow(10.0, (radioFrame->getSnr()/ 10));
        cinfo->setTestFrameError(radioModel->getTestFrameError(snirMin,radioFrame->getBitrate()));
        cinfo->setTestFrameSize(radioModel->getTestFrameSize());
    }
    cinfo->setSnr(radioFrame->getSnr());
    cinfo->setLossRate(radioFrame->getLossRate());
    cinfo->setRecPow(radioFrame->getPowRec());
    cinfo->setModulationType(radioFrame->getModulationType());
    frame->setControlInfo(cinfo);

    delete radioFrame;
    EV << "sending up frame " << frame->getName() << endl;
    send(frame, upperLayerOut);
}

void SimplifiedRadio::sendDown(SimplifiedRadioFrame *radioFrame)
{
    updateRadioChannelState();
    if (transmitterConnected)
        sendToChannel(radioFrame);
    else
        delete radioFrame;
}

/**
 * Get the context pointer to the now completely received radio frame and
 * delete the self message
 */
SimplifiedRadioFrame *SimplifiedRadio::unbufferMsg(cMessage *msg)
{
    SimplifiedRadioFrame *radioFrame = (SimplifiedRadioFrame *) msg->getContextPointer();
    //delete the self message
    delete msg;

    return radioFrame;
}

/**
 * If a message is already being transmitted, an error is raised.
 *
 * Otherwise the RadioState is set to TRANSMIT and a timer is
 * started. When this timer expires the RadioState will be set back to RECV
 * (or IDLE respectively) again.
 *
 * If the host is receiving a packet this packet is from now on only
 * considered as noise.
 */
void SimplifiedRadio::handleUpperMsg(SimplifiedRadioFrame *radioFrame)
{
    if (getRadioChannelState() == RADIO_CHANNEL_STATE_TRANSMITTING)
        error("Trying to send a message while already transmitting -- MAC should "
              "take care this does not happen");

    // if a packet was being received, it is corrupted now as should be treated as noise
    if (snrInfo.ptr != NULL)
    {
        EV << "Sending a message while receiving another. The received one is now corrupted.\n";

        // remove the snr information stored for the message currently being
        // received. This message is treated as noise now and the
        // receive power has to be added to the noiseLevel

        // delete the pointer to indicate that no message is being received
        snrInfo.ptr = NULL;
        // clear the snr list
        snrInfo.sList.clear();
        // add the receive power to the noise level
        noiseLevel += snrInfo.rcvdPower;
    }

    // now we are done with all the exception handling and can take care
    // about the "real" stuff

    // change radio status
    EV << "sending down frame\n";

    scheduleAt(simTime() + radioFrame->getDuration(), endTransmissionTimer);
    sendDown(radioFrame);
}

void SimplifiedRadio::handleCommand(int msgkind, cObject *ctrl)
{
    if (msgkind==PHY_C_CONFIGURERADIO)
    {
        // extract new channel number
        PhyControlInfo *phyCtrl = check_and_cast<PhyControlInfo *>(ctrl);
        int newChannel = phyCtrl->getChannelNumber();
        double newBitrate = phyCtrl->getBitrate();
        delete ctrl;

        if (newChannel!=-1)
        {
            EV << "Command received: change to channel #" << newChannel << "\n";

            // do it
            if (radioChannel == newChannel)
                EV << "Right on that channel, nothing to do\n"; // fine, nothing to do
            else if (getRadioChannelState()==RADIO_CHANNEL_STATE_TRANSMITTING)
            {
                EV << "We're transmitting right now, remembering to change after it's completed\n";
                this->newChannel = newChannel;
            }
            else
                setRadioChannel(newChannel); // change channel right now
        }
        if (newBitrate!=-1)
        {
            EV << "Command received: change bitrate to " << (newBitrate/1e6) << "Mbps\n";

            // do it
            if (bitrate == newBitrate)
                EV << "Right at that bitrate, nothing to do\n"; // fine, nothing to do
            else if (getRadioChannelState()==RADIO_CHANNEL_STATE_TRANSMITTING)
            {
                EV << "We're transmitting right now, remembering to change after it's completed\n";
                this->newBitrate = newBitrate;
            }
            else
                setBitrate(newBitrate); // change bitrate right now
        }
    }
    else if (msgkind==PHY_C_CHANGETRANSMITTERPOWER)
    {
        PhyControlInfo *phyCtrl = check_and_cast<PhyControlInfo *>(ctrl);
        double newTransmitterPower = phyCtrl->getTransmitterPower();
        if (newTransmitterPower!=-1)
        {
            if (newTransmitterPower > (double)getRadioChannelPar("pMax"))
                transmitterPower = (double) getRadioChannelPar("pMax");
            else
                transmitterPower = newTransmitterPower;
        }
    }
    else
        error("unknown command (msgkind=%d)", msgkind);
}

void SimplifiedRadio::handleSelfMsg(cMessage *msg)
{
    if (msg == endTransmissionTimer)
    {
        EV << "Transmission successfully completed.\n";
        updateRadioChannelState();
        // switch channel if it needs be
        if (newChannel != -1)
        {
            setRadioChannel(newChannel);
            newChannel = -1;
        }
    }
    else
    {
        EV << "Frame is completely received now.\n";
        for (EndReceptionTimers::iterator it = endReceptionTimers.begin(); it != endReceptionTimers.end(); ++it)
        {
            if (*it == msg)
            {
                endReceptionTimers.erase(it);
                handleLowerMsgEnd(unbufferMsg(msg));
                updateRadioChannelState();
                return;
            }
        }
        throw cRuntimeError("Self message not found in endReceptionTimers.");
    }
}


/**
 * This function is called right after a packet arrived, i.e. right
 * before it is buffered for 'transmission time'.
 *
 * First the receive power of the packet has to be calculated and is
 * stored in the recvBuff. Afterwards it has to be decided whether the
 * packet is just noise or a "real" packet that needs to be received.
 *
 * The message is not treated as noise if all of the following
 * conditions apply:
 *
 * -# the power of the received signal is higher than the sensitivity.
 * -# the host is currently not sending a message
 * -# no other packet is already being received
 *
 * If all conditions apply a new SnrList is created and the RadioState
 * is changed to RECV.
 *
 * If the packet is just noise the receive power is added to the noise
 * Level of the channel. Additionally the snr information of the
 * currently being received message (if any) has to be updated as
 * well as the RadioState.
 */
void SimplifiedRadio::handleLowerMsgStart(SimplifiedRadioFrame* radioFrame)
{
    // Calculate the receive power of the message

    // calculate distance
    const Coord& framePos = radioFrame->getSenderPos();
    double distance = getRadioPosition().distance(framePos);

    // calculate receive power
    double frequency = carrierFrequency;
    if (radioFrame && radioFrame->getCarrierFrequency()>0.0)
        frequency = radioFrame->getCarrierFrequency();

    if (distance<MIN_DISTANCE)
        distance = MIN_DISTANCE;

    double rcvdPower = receptionModel->calculateReceivedPower(radioFrame->getPSend(), frequency, distance);
    if (obstacles && distance > MIN_DISTANCE)
        rcvdPower = obstacles->calculateReceivedPower(rcvdPower, carrierFrequency, framePos, 0, getRadioPosition(), 0);
    radioFrame->setPowRec(rcvdPower);
    // store the receive power in the recvBuff
    recvBuff[radioFrame] = rcvdPower;
    updateSensitivity(radioFrame->getBitrate());

    // if receive power is bigger than sensitivity and if not sending
    // and currently not receiving another message and the message has
    // arrived in time
    // NOTE: a message may have arrival time in the past here when we are
    // processing ongoing transmissions during a channel change
    if (radioFrame->getArrivalTime() == simTime() && rcvdPower >= sensitivity && getRadioChannelState() != RADIO_CHANNEL_STATE_TRANSMITTING && snrInfo.ptr == NULL)
    {
        EV << "receiving frame " << radioFrame->getName() << endl;

        // Put frame and related SnrList in receive buffer
        SnrList snrList;
        snrInfo.ptr = radioFrame;
        snrInfo.rcvdPower = rcvdPower;
        snrInfo.sList = snrList;

        // add initial snr value
        addNewSnr();
    }
    // receive power is too low or another message is being sent or received
    else
    {
        EV << "frame " << radioFrame->getName() << " is just noise\n";
        //add receive power to the noise level
        noiseLevel += rcvdPower;

        // if a message is being received add a new snr value
        if (snrInfo.ptr != NULL)
        {
            // update snr info for currently being received message
            EV << "adding new snr value to snr list of message being received\n";
            addNewSnr();
        }
    }
}


/**
 * This function is called right after the transmission is over,
 * i.e. right after unbuffering.  The noise level of the channel and
 * the snr information of the buffered messages have to be updated.
 *
 * Additionally the RadioState has to be updated.
 *
 * If the corresponding SimplifiedRadioFrame was not only noise the corresponding
 * SnrList and the SimplifiedRadioFrame are sent to the decider.
 */
void SimplifiedRadio::handleLowerMsgEnd(SimplifiedRadioFrame *radioFrame)
{
    // check if message has to be send to the decider
    if (snrInfo.ptr == radioFrame)
    {
        EV << "reception of frame over, preparing to send packet to upper layer\n";
        // get Packet and list out of the receive buffer:
        SnrList list;
        list = snrInfo.sList;

        // delete the pointer to indicate that no message is currently
        // being received and clear the list

        double snirMin = snrInfo.sList.begin()->snr;
        for (SnrList::const_iterator iter = snrInfo.sList.begin(); iter != snrInfo.sList.end(); iter++)
            if (iter->snr < snirMin)
                snirMin = iter->snr;
        snrInfo.ptr = NULL;
        snrInfo.sList.clear();
        radioFrame->setSnr(10*log10(snirMin)); //ahmed
        radioFrame->setLossRate(lossRate);
        // delete the frame from the recvBuff
        recvBuff.erase(radioFrame);

        //XXX send up the frame:
        //if (radioModel->isReceivedCorrectly(radioFrame, list))
        //    sendUp(radioFrame);
        //else
        //    delete radioFrame;
        if (!radioModel->isReceivedCorrectly(radioFrame, list))
        {
            radioFrame->getEncapsulatedPacket()->setKind(list.size()>1 ? COLLISION : BITERROR);
            radioFrame->setName(list.size()>1 ? "COLLISION" : "BITERROR");

            numGivenUp++;
        }
        else
            numReceivedCorrectly++;

        if ( (numReceivedCorrectly+numGivenUp)%50 == 0)
        {
            lossRate = (double)numGivenUp/((double)numReceivedCorrectly+(double)numGivenUp);
            emit(lossRateSignal, lossRate);
            numReceivedCorrectly = 0;
            numGivenUp = 0;
        }
        sendUp(radioFrame);
    }
    // all other messages are noise
    else
    {
        EV << "reception of noise message over, removing recvdPower from noiseLevel....\n";
        // get the rcvdPower and subtract it from the noiseLevel
        noiseLevel -= recvBuff[radioFrame];

        // delete message from the recvBuff
        recvBuff.erase(radioFrame);

        // update snr info for message currently being received if any
        if (snrInfo.ptr != NULL)
        {
            addNewSnr();
        }

        // message should be deleted
        delete radioFrame;
        EV << "message deleted\n";
    }
}

void SimplifiedRadio::addNewSnr()
{
    SnrListEntry listEntry;     // create a new entry
    listEntry.time = simTime();
    listEntry.snr = snrInfo.rcvdPower / (BASE_NOISE_LEVEL);
    snrInfo.sList.push_back(listEntry);
}

void SimplifiedRadio::setRadioChannel(int channel)
{
    Enter_Method_Silent();
    if (channel == radioChannel)
        return;
    if (getRadioChannelState() == RADIO_CHANNEL_STATE_TRANSMITTING)
        error("changing channel while transmitting is not allowed");

   // Clear the recvBuff
   for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
   {
        SimplifiedRadioFrame *radioFrame = it->first;
        cMessage *endReceptionTimer = (cMessage *)radioFrame->getContextPointer();
        delete radioFrame;
        delete cancelEvent(endReceptionTimer);
    }
    recvBuff.clear();

    // clear snr info
    snrInfo.ptr = NULL;
    snrInfo.sList.clear();

    // reset the noiseLevel
    noiseLevel = thermalNoise;
    updateRadioChannelState();

    // do channel switch
    EV << "Changing from channel " << radioChannel << " to " << channel << "\n";
    radioChannel = channel;
    emit(radioChannelChangedSignal, channel);

    cc->setRadioChannel(myRadioRef, radioChannel);

    cModule *myHost = getContainingNode(this);

    //cGate *radioGate = myHost->gate("radioIn");

    cGate* radioGate = this->gate("radioIn")->getPathStartGate();

    EV << "RadioGate :" << radioGate->getFullPath() << " " << radioGate->getFullName() << endl;

    // pick up ongoing transmissions on the new channel
    EV << "Picking up ongoing transmissions on new channel:\n";
    ISimplifiedRadioChannel::TransmissionList tlAux = cc->getOngoingTransmissions(channel);
    for (ISimplifiedRadioChannel::TransmissionList::const_iterator it = tlAux.begin(); it != tlAux.end(); ++it)
    {
        SimplifiedRadioFrame *radioFrame = check_and_cast<SimplifiedRadioFrame *> (*it);
        if (radioFrame->getSenderModule() == this)
            continue;

        // time for the message to reach us
        double distance = getRadioPosition().distance(radioFrame->getSenderPos());
        simtime_t propagationDelay = distance / 3.0E+8;

        // if this transmission is on our new channel and it would reach us in the future, then schedule it
        if (channel == radioFrame->getChannelNumber())
        {
            EV << " - (" << radioFrame->getClassName() << ")" << radioFrame->getName() << ": ";
        }

        // if there is a message on the air which will reach us in the future
        if (radioFrame->getTimestamp() + propagationDelay >= simTime())
        {
            EV << "will arrive in the future, scheduling it\n";

            // we need to send to each radioIn[] gate of this host
            //for (int i = 0; i < radioGate->size(); i++)
            //    sendDirect(radioFrame->dup(), radioFrame->getTimestamp() + propagationDelay - simTime(), radioFrame->getDuration(), myHost, radioGate->getId() + i);

            // JcM Fix: we need to this radio only. no need to send the packet to each radioIn
            // since other radios might be not in the same channel
            sendDirect(radioFrame->dup(), radioFrame->getTimestamp() + propagationDelay - simTime(), radioFrame->getDuration(), myHost, radioGate->getId() );
        }
        // if we hear some part of the message
        else if (radioFrame->getTimestamp() + radioFrame->getDuration() + propagationDelay > simTime())
        {
            EV << "missed beginning of frame, processing it as noise\n";

            SimplifiedRadioFrame *frameDup = radioFrame->dup();
            frameDup->setArrivalTime(radioFrame->getTimestamp() + propagationDelay);
            handleLowerMsgStart(frameDup);
            bufferMsg(frameDup);
            updateRadioChannelState();
        }
        else
        {
            EV << "in the past\n";
        }
    }
}

void SimplifiedRadio::setBitrate(double bitrate)
{
    if (this->bitrate == bitrate)
        return;
    if (bitrate < 0)
        error("setBitrate(): bitrate cannot be negative (%g)", bitrate);
    if (getRadioChannelState() == RADIO_CHANNEL_STATE_TRANSMITTING)
        error("changing the bitrate while transmitting is not allowed");

    EV << "Setting bitrate to " << (bitrate/1e6) << "Mbps\n";
    emit(bitrateSignal, bitrate);
    this->bitrate = bitrate;

    //XXX fire some notification?
}

void SimplifiedRadio::setRadioMode(RadioMode newRadioMode)
{
    Enter_Method_Silent();
    if (radioMode != newRadioMode)
    {
        if (newRadioMode == RADIO_MODE_SLEEP || newRadioMode == RADIO_MODE_OFF)
        {
            disconnectReceiver();
            disconnectTransmitter();
        }
        else if (newRadioMode == RADIO_MODE_RECEIVER)
        {
            connectReceiver();
            disconnectTransmitter();
        }
        else if (newRadioMode == RADIO_MODE_TRANSMITTER)
        {
            disconnectReceiver();
            connectTransmitter();
        }
        else
            throw cRuntimeError("Unknown radio mode");

        EV << "Changing radio mode from " << getRadioModeName(radioMode) << " to " << getRadioModeName(newRadioMode) << ".\n";
        radioMode = newRadioMode;
        emit(radioModeChangedSignal, newRadioMode);
        updateRadioChannelState();
    }
}

void SimplifiedRadio::updateRadioChannelState()
{
    RadioChannelState newRadioChannelState;
    if (radioMode == RADIO_MODE_OFF || radioMode == RADIO_MODE_SLEEP)
        newRadioChannelState = RADIO_CHANNEL_STATE_UNKNOWN;
    else if (endTransmissionTimer->isScheduled())
        newRadioChannelState = RADIO_CHANNEL_STATE_TRANSMITTING;
    else if (snrInfo.ptr != NULL && endReceptionTimers.size() > 0)
        newRadioChannelState = RADIO_CHANNEL_STATE_RECEIVING;
    else if (BASE_NOISE_LEVEL > receptionThreshold) // TODO: how?
        newRadioChannelState = RADIO_CHANNEL_STATE_BUSY;
    else
        newRadioChannelState = RADIO_CHANNEL_STATE_FREE;
    if (radioChannelState != newRadioChannelState)
    {
        EV << "Changing radio channel state from " << getRadioChannelStateName(radioChannelState) << " to " << getRadioChannelStateName(newRadioChannelState) << ".\n";
        radioChannelState = newRadioChannelState;
        emit(radioChannelStateChangedSignal, newRadioChannelState);
    }
}

/*
void SimplifiedRadio::updateSensitivity(double rate)
{
    EV<<"bitrate = "<<rate<<endl;
    EV <<" sensitivity: "<<sensitivity<<endl;
    if (rate == 6E+6)
    {
        sensitivity = FWMath::dBm2mW(-82);
    }
    else if (rate == 9E+6)
    {
        sensitivity = FWMath::dBm2mW(-81);
    }
    else if (rate == 12E+6)
    {
        sensitivity = FWMath::dBm2mW(-79);
    }
    else if (rate == 18E+6)
    {
        sensitivity = FWMath::dBm2mW(-77);
    }
    else if (rate == 24E+6)
    {
        sensitivity = FWMath::dBm2mW(-74);
    }
    else if (rate == 36E+6)
    {
        sensitivity = FWMath::dBm2mW(-70);
    }
    else if (rate == 48E+6)
    {
        sensitivity = FWMath::dBm2mW(-66);
    }
    else if (rate == 54E+6)
    {
        sensitivity = FWMath::dBm2mW(-65);
    }
    EV <<" sensitivity after updateSensitivity: "<<sensitivity<<endl;
}
*/

void SimplifiedRadio::updateSensitivity(double rate)
{
    if (sensitivityList.empty())
    {
        return;
    }
    SensitivityList::iterator it = sensitivityList.find(rate);
    if (it != sensitivityList.end())
        sensitivity = it->second;
    else
        sensitivity = sensitivityList[0.0];
    if (!par("setReceptionThreshold").boolValue())
        receptionThreshold = sensitivity;
    EV<<"bitrate = "<<rate<<endl;
    EV <<" sensitivity after updateSensitivity: "<<sensitivity<<endl;
}

void SimplifiedRadio::updateDisplayString() {
    // draw the interference area and sensitivity area
    // according pathloss propagation only
    // we use the radio channel method to calculate interference distance
    // it should be the methods provided by propagation models, but to
    // avoid a big modification, we reuse those methods.

    if (!ev.isGUI() || !drawCoverage) // nothing to do
        return;
    if (myRadioRef) {
        cDisplayString& d = hostModule->getDisplayString();

        // communication area (up to sensitivity)
        // FIXME this overrides the ranges if more than one radio is present is a host
        double sensitivity_limit = cc->getInterferenceRange(myRadioRef);
        d.removeTag("r1");
        d.insertTag("r1");
        d.setTagArg("r1", 0, (long) sensitivity_limit);
        d.setTagArg("r1", 2, "gray");
        d.removeTag("r2");
        d.insertTag("r2");
        if (doubleRayCoverage)
            d.setTagArg("r2", 0, (long) calcDistDoubleRay());
        else
            d.setTagArg("r2", 0, (long) calcDistFreeSpace());
        d.setTagArg("r2", 2, "blue");
    }
    if (updateString==NULL && updateStringInterval>0)
        updateString = new cMessage("refresh timer");
    if (updateStringInterval>0)
        scheduleAt(simTime()+updateStringInterval, updateString);
}

double SimplifiedRadio::calcDistFreeSpace()
{
    //the carrier frequency used
    double carrierFrequency = getRadioChannelPar("carrierFrequency");
    //signal attenuation threshold
    //path loss coefficient
    double alpha = getRadioChannelPar("alpha");

    double waveLength = (SPEED_OF_LIGHT / carrierFrequency);
    //minimum power level to be able to physically receive a signal
    double minReceivePower = sensitivity;

    double interfDistance = pow(waveLength * waveLength * transmitterPower /
                         (16.0 * M_PI * M_PI * minReceivePower), 1.0 / alpha);
    return interfDistance;
}



double SimplifiedRadio::calcDistDoubleRay()
{
    //the carrier frequency used
    //minimum power level to be able to physically receive a signal
    double minReceivePower = sensitivity;

    double interfDistance = pow(transmitterPower/minReceivePower, 1.0 / 4);

    return interfDistance;
}

void SimplifiedRadio::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    SimplifiedRadioChannelAccess::receiveSignal(source,signalID, obj);
    if (signalID == changeLevelNoise)
        updateRadioChannelState();
}

void SimplifiedRadio::disconnectReceiver()
{
    receiverConnected = false;
    if (getRadioChannelState() == RADIO_CHANNEL_STATE_TRANSMITTING)
        error("changing channel while transmitting is not allowed");

   // Clear the recvBuff
   for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
   {
        SimplifiedRadioFrame *radioFrame = it->first;
        cMessage *endReceptionTimer = (cMessage *)radioFrame->getContextPointer();
        delete radioFrame;
        delete cancelEvent(endReceptionTimer);
    }
    recvBuff.clear();

    // clear snr info
    snrInfo.ptr = NULL;
    snrInfo.sList.clear();
}

void SimplifiedRadio::connectReceiver()
{
    receiverConnected = true;
    cc->setRadioChannel(myRadioRef, radioChannel);
    cModule *myHost = getContainingNode(this);

    //cGate *radioGate = myHost->gate("radioIn");

    cGate* radioGate = this->gate("radioIn")->getPathStartGate();

    EV << "RadioGate :" << radioGate->getFullPath() << " " << radioGate->getFullName() << endl;

    // pick up ongoing transmissions on the new channel
    EV << "Picking up ongoing transmissions on new channel:\n";
    ISimplifiedRadioChannel::TransmissionList tlAux = cc->getOngoingTransmissions(radioChannel);
    for (ISimplifiedRadioChannel::TransmissionList::const_iterator it = tlAux.begin(); it != tlAux.end(); ++it)
    {
        SimplifiedRadioFrame *radioFrame = check_and_cast<SimplifiedRadioFrame *> (*it);
        if (radioFrame->getSenderModule() == this)
            continue;

        // time for the message to reach us
        double distance = getRadioPosition().distance(radioFrame->getSenderPos());
        simtime_t propagationDelay = distance / 3.0E+8;

        // if there is a message on the air which will reach us in the future
        if (radioFrame->getTimestamp() + propagationDelay >= simTime())
        {
            EV << " - (" << radioFrame->getClassName() << ")" << radioFrame->getName() << ": ";
            EV << "will arrive in the future, scheduling it\n";

            // we need to send to each radioIn[] gate of this host
            //for (int i = 0; i < radioGate->size(); i++)
            //    sendDirect(radioFrame->dup(), radioFrame->getTimestamp() + propagationDelay - simTime(), radioFrame->getDuration(), myHost, radioGate->getId() + i);

            // JcM Fix: we need to this radio only. no need to send the packet to each radioIn
            // since other radios might be not in the same channel
            sendDirect(radioFrame->dup(), radioFrame->getTimestamp() + propagationDelay - simTime(), radioFrame->getDuration(), myHost, radioGate->getId() );
        }
        // if we hear some part of the message
        else if (radioFrame->getTimestamp() + radioFrame->getDuration() + propagationDelay > simTime())
        {
            EV << "missed beginning of frame, processing it as noise\n";

            SimplifiedRadioFrame *frameDup = radioFrame->dup();
            frameDup->setArrivalTime(radioFrame->getTimestamp() + propagationDelay);
            handleLowerMsgStart(frameDup);
            bufferMsg(frameDup);
            updateRadioChannelState();
        }
    }
}


void SimplifiedRadio::getSensitivityList(cXMLElement* xmlConfig)
{
    sensitivityList.empty();

    if (xmlConfig == 0)
    {
        sensitivityList[0] = FWMath::dBm2mW(par("sensitivity").doubleValue());
        return;
    }

    cXMLElementList sensitivityXmlList = xmlConfig->getElementsByTagName("SensitivityTable");

    if (sensitivityXmlList.empty())
    {
        sensitivityList[0] = FWMath::dBm2mW(par("sensitivity").doubleValue());
        return;
    }

    // iterate over all sensitivity-entries, get a new instance and add
    // it to sensitivityList
    for (cXMLElementList::const_iterator it = sensitivityXmlList.begin(); it != sensitivityXmlList.end(); it++)
    {

        cXMLElement* data = *it;

        cXMLElementList parameters = data->getElementsByTagName("Entry");

        for(cXMLElementList::const_iterator it = parameters.begin();
            it != parameters.end(); it++)
        {
            const char* bitRate = (*it)->getAttribute("BitRate");
            const char* sensitivityStr = (*it)->getAttribute("Sensitivity");
            double rate = atof(bitRate);
            if (rate == 0)
                error("invalid bit rate");
            double sens = atof(sensitivityStr);
            sensitivityList[rate] = FWMath::dBm2mW(sens);

        }
        parameters = data->getElementsByTagName("Default");
        for(cXMLElementList::const_iterator it = parameters.begin();
            it != parameters.end(); it++)
        {
            const char* sensitivityStr = (*it)->getAttribute("Sensitivity");
            double sens = atof(sensitivityStr);
            sensitivityList[0.0] = FWMath::dBm2mW(sens);
        }

        SensitivityList::iterator sensitivityIt = sensitivityList.find(0.0);
        if (sensitivityIt == sensitivityList.end())
        {
            sensitivityList[0] = FWMath::dBm2mW(par("sensitivity").doubleValue());
        }
    } // end iterator loop
}

