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


#include "Radio.h"
#include "FWMath.h"
#include "PhyControlInfo_m.h"
#include "Radio80211aControlInfo_m.h"
#include "BasicBattery.h"


#define MK_TRANSMISSION_OVER  1
#define MK_RECEPTION_COMPLETE 2

simsignal_t Radio::bitrateSignal = SIMSIGNAL_NULL;
simsignal_t Radio::radioStateSignal = SIMSIGNAL_NULL;
simsignal_t Radio::channelNumberSignal = SIMSIGNAL_NULL;
simsignal_t Radio::lossRateSignal = SIMSIGNAL_NULL;
simsignal_t Radio::changeLevelNoise = SIMSIGNAL_NULL;

#define MIN_DISTANCE 0.001 // minimum distance 1 millimeter
#define BASE_NOISE_LEVEL (noiseGenerator?noiseLevel+noiseGenerator->noiseLevel():noiseLevel)

Define_Module(Radio);
Radio::Radio() : rs(this->getId())
{
    obstacles = NULL;
    radioModel = NULL;
    receptionModel = NULL;
    transceiverConnect = true;
    receiverConnect = true;
    updateString = NULL;
    noiseGenerator = NULL;
}

void Radio::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    EV << "Initializing AbstractRadio, stage=" << stage << endl;

    if (stage == 0)
    {
        gate("radioIn")->setDeliverOnReceptionStart(true);

        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");

        getSensitivityList(par("SensitivityTable").xmlValue());

        // read parameters
        transmitterPower = par("transmitterPower");
        if (transmitterPower > (double) (getChannelControlPar("pMax")))
            error("transmitterPower cannot be bigger than pMax in ChannelControl!");
        rs.setBitrate(par("bitrate"));
        rs.setChannelNumber(par("channelNumber"));
        rs.setRadioId(this->getId());
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
            changeLevelNoise = registerSignal("changeLevelNoise");
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

        // Initialize radio state. If thermal noise is already to high, radio
        // state has to be initialized as RECV
        rs.setState(RadioState::IDLE);
        if (BASE_NOISE_LEVEL >= sensitivity)
            rs.setState(RadioState::RECV);

        WATCH(noiseLevel);
        WATCH(rs);

        obstacles = ObstacleControlAccess().getIfExists();
        if (obstacles) EV << "Found ObstacleControl" << endl;

        // this is the parameter of the channel controller (global)
        std::string propModel = getChannelControlPar("propagationModel").stdstringValue();
        if (propModel == "")
            propModel = "FreeSpaceModel";

        receptionModel = (IReceptionModel *) createOne(propModel.c_str());
        receptionModel->initializeFrom(this);

        // radio model to handle frame length and reception success calculation (modulation, error correction etc.)
        std::string rModel = par("radioModel").stdstringValue();
        if (rModel=="")
            rModel = "GenericRadioModel";

        radioModel = (IRadioModel *) createOne(rModel.c_str());
        radioModel->initializeFrom(this);

        // statistics
        bitrateSignal = registerSignal("bitrate");
        radioStateSignal = registerSignal("radioState");
        channelNumberSignal = registerSignal("channelNo");
        lossRateSignal = registerSignal("lossRate");

        if (this->hasPar("drawCoverage"))
            drawCoverage = par("drawCoverage");
        else
            drawCoverage = false;
        if (this->hasPar("refreshCoverageInterval"))
            updateStringInterval = par("refreshCoverageInterval");
        else
            updateStringInterval = 0;

    }
    else if (stage == 1)
    {
        registerBattery();
        // tell initial values to MAC; must be done in stage 1, because they
        // subscribe in stage 0
        nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
        nb->fireChangeNotification(NF_RADIO_CHANNEL_CHANGED, &rs);
    }
    else if (stage == 2)
    {
        // tell initial channel number to ChannelControl; should be done in
        // stage==2 or later, because base class initializes myRadioRef in that stage
        cc->setRadioChannel(myRadioRef, rs.getChannelNumber());

        // statistics
        emit(bitrateSignal, rs.getBitrate());
        emit(radioStateSignal, rs.getState());
        emit(channelNumberSignal, rs.getChannelNumber());

        // draw the interference distance
        this->updateDisplayString();
    }
}

void Radio::finish()
{
}

Radio::~Radio()
{
    delete radioModel;
    delete receptionModel;
    if (noiseGenerator)
        delete noiseGenerator;

    if (updateString)
        cancelAndDelete(updateString);
    // delete messages being received
    for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
        delete it->first;
}


bool Radio::processAirFrame(AirFrame *airframe)
{
    return airframe->getChannelNumber() == getChannelNumber();
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
void Radio::handleMessage(cMessage *msg)
{
    // handle commands
    if (updateString && updateString==msg)
    {
        this->updateDisplayString();
        return;
    }
    if (msg->getArrivalGateId()==upperLayerIn && !msg->isPacket() /*FIXME XXX ENSURE REALLY PLAIN cMessage ARE SENT AS COMMANDS!!! && msg->getBitLength()==0*/)
    {
        cObject *ctrl = msg->removeControlInfo();
        if (msg->getKind()==0)
            error("Message '%s' with length==0 is supposed to be a command, but msg kind is also zero", msg->getName());
        handleCommand(msg->getKind(), ctrl);
        delete msg;
        return;
    }

    if (msg->getArrivalGateId() == upperLayerIn)
    {
        if (this->isEnabled())
        {
            AirFrame *airframe = encapsulatePacket(PK(msg));
            handleUpperMsg(airframe);
        }
        else
        {
            EV << "Radio disabled. ignoring frame" << endl;
            delete msg;
        }
    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
    else if (processAirFrame(check_and_cast<AirFrame*>(msg)))
    {
        if (this->isEnabled() && receiverConnect)
        {
        // must be an AirFrame
            AirFrame *airframe = (AirFrame *) msg;
            handleLowerMsgStart(airframe);
            bufferMsg(airframe);
        }
        else
        {
            EV << "Radio disabled. ignoring airframe" << endl;
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
void Radio::bufferMsg(AirFrame *airframe) //FIXME: add explicit simtime_t atTime arg?
{
    // set timer to indicate transmission is complete
    cMessage *endRxTimer = new cMessage("endRx", MK_RECEPTION_COMPLETE);
    endRxTimer->setContextPointer(airframe);
    airframe->setContextPointer(endRxTimer);

    // NOTE: use arrivalTime instead of simTime, because we might be calling this
    // function during a channel change, when we're picking up ongoing transmissions
    // on the channel -- and then the message's arrival time is in the past!
    scheduleAt(airframe->getArrivalTime() + airframe->getDuration(), endRxTimer);
}

AirFrame *Radio::encapsulatePacket(cPacket *frame)
{
    PhyControlInfo *ctrl = dynamic_cast<PhyControlInfo *>(frame->removeControlInfo());
    ASSERT(!ctrl || ctrl->getChannelNumber()==-1); // per-packet channel switching not supported

    // Note: we don't set length() of the AirFrame, because duration will be used everywhere instead
    //if (ctrl && ctrl->getAdaptiveSensitivity()) updateSensitivity(ctrl->getBitrate());
    AirFrame *airframe = createAirFrame();
    airframe->setName(frame->getName());
    airframe->setPSend(transmitterPower);
    airframe->setChannelNumber(getChannelNumber());
    airframe->encapsulate(frame);
    airframe->setBitrate(ctrl ? ctrl->getBitrate() : rs.getBitrate());
    airframe->setDuration(radioModel->calculateDuration(airframe));
    airframe->setSenderPos(getRadioPosition());
    airframe->setCarrierFrequency(carrierFrequency);
    delete ctrl;

    EV << "Frame (" << frame->getClassName() << ")" << frame->getName()
    << " will be transmitted at " << (airframe->getBitrate()/1e6) << "Mbps\n";
    return airframe;
}

void Radio::sendUp(AirFrame *airframe)
{
    cPacket *frame = airframe->decapsulate();
    Radio80211aControlInfo * cinfo = new Radio80211aControlInfo;
    if (radioModel->haveTestFrame())
    {
        cinfo->setAirtimeMetric(true);
        cinfo->setTestFrameDuration(radioModel->calculateDurationTestFrame(airframe));
        double snirMin = pow(10.0, (airframe->getSnr()/ 10));
        cinfo->setTestFrameError(radioModel->getTestFrameError(snirMin,airframe->getBitrate()));
        cinfo->setTestFrameSize(radioModel->getTestFrameSize());
    }
    cinfo->setSnr(airframe->getSnr());
    cinfo->setLossRate(airframe->getLossRate());
    cinfo->setRecPow(airframe->getPowRec());
    cinfo->setModulationType(airframe->getModulationType());
    frame->setControlInfo(cinfo);

    delete airframe;
    EV << "sending up frame " << frame->getName() << endl;
    send(frame, upperLayerOut);
}

void Radio::sendDown(AirFrame *airframe)
{
    if (transceiverConnect)
        sendToChannel(airframe);
    else
        delete airframe;
}

/**
 * Get the context pointer to the now completely received AirFrame and
 * delete the self message
 */
AirFrame *Radio::unbufferMsg(cMessage *msg)
{
    AirFrame *airframe = (AirFrame *) msg->getContextPointer();
    //delete the self message
    delete msg;

    return airframe;
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
void Radio::handleUpperMsg(AirFrame *airframe)
{
    if (rs.getState() == RadioState::TRANSMIT)
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
    EV << "sending, changing RadioState to TRANSMIT\n";
    setRadioState(RadioState::TRANSMIT);

    cMessage *timer = new cMessage(NULL, MK_TRANSMISSION_OVER);
    scheduleAt(simTime() + airframe->getDuration(), timer);
    sendDown(airframe);
}

void Radio::handleCommand(int msgkind, cObject *ctrl)
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
            if (rs.getChannelNumber()==newChannel)
                EV << "Right on that channel, nothing to do\n"; // fine, nothing to do
            else if (rs.getState()==RadioState::TRANSMIT)
            {
                EV << "We're transmitting right now, remembering to change after it's completed\n";
                this->newChannel = newChannel;
            }
            else
                changeChannel(newChannel); // change channel right now
        }
        if (newBitrate!=-1)
        {
            EV << "Command received: change bitrate to " << (newBitrate/1e6) << "Mbps\n";

            // do it
            if (rs.getBitrate()==newBitrate)
                EV << "Right at that bitrate, nothing to do\n"; // fine, nothing to do
            else if (rs.getState()==RadioState::TRANSMIT)
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
            if (newTransmitterPower > (double)getChannelControlPar("pMax"))
                transmitterPower = (double) getChannelControlPar("pMax");
            else
                transmitterPower = newTransmitterPower;
        }
    }
    else
        error("unknown command (msgkind=%d)", msgkind);
}

void Radio::handleSelfMsg(cMessage *msg)
{
    EV<<"AbstractRadio::handleSelfMsg"<<msg->getKind()<<endl;
    if (msg->getKind()==MK_RECEPTION_COMPLETE)
    {
        EV << "frame is completely received now\n";

        // unbuffer the message
        AirFrame *airframe = unbufferMsg(msg);

        handleLowerMsgEnd(airframe);
    }
    else if (msg->getKind() == MK_TRANSMISSION_OVER)
    {
        // Transmission has completed. The RadioState has to be changed
        // to IDLE or RECV, based on the noise level on the channel.
        // If the noise level is bigger than the sensitivity switch to receive mode,
        // otherwise to idle mode.
        RadioState::State newState;
        if (BASE_NOISE_LEVEL < sensitivity)
        {
            // set the RadioState to IDLE
            EV << "transmission over, switch to idle mode (state:IDLE)\n";
            // setRadioState(RadioState::IDLE);
            newState = RadioState::IDLE;
        }
        else
        {
            // set the RadioState to RECV
            EV << "transmission over but noise level too high, switch to recv mode (state:RECV)\n";
            // setRadioState(RadioState::RECV);
            newState = RadioState::RECV;
        }

        // delete the timer
        delete msg;

        // switch channel if it needs be
        if (newChannel!=-1)
        {
            if (newChannel == rs.getChannelNumber())
            {
                setRadioState(newState); // nothing to do change the state
            }
            else
            {
                // if change the channel the method changeChannel must set the correct radio state
                rs.setState(newState);
                changeChannel(newChannel);
            }
            newChannel = -1;
        }
        else
        {
            // newChannel==-1 the channel doesn't change
            setRadioState(newState); // now the radio changes the state and sends the signal
        }
    }
    else
    {
        error("Internal error: unknown self-message `%s'", msg->getName());
    }
    EV<<"AbstractRadio::handleSelfMsg END"<<endl;
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
void Radio::handleLowerMsgStart(AirFrame* airframe)
{
    // Calculate the receive power of the message

    // calculate distance
    const Coord& framePos = airframe->getSenderPos();
    double distance = getRadioPosition().distance(framePos);

    // calculate receive power
    double frequency = carrierFrequency;
    if (airframe && airframe->getCarrierFrequency()>0.0)
        frequency = airframe->getCarrierFrequency();

    if (distance<MIN_DISTANCE)
        distance = MIN_DISTANCE;

    double rcvdPower = receptionModel->calculateReceivedPower(airframe->getPSend(), frequency, distance);
    if (obstacles && distance > MIN_DISTANCE)
        rcvdPower = obstacles->calculateReceivedPower(rcvdPower, carrierFrequency, framePos, 0, getRadioPosition(), 0);
    airframe->setPowRec(rcvdPower);
    // store the receive power in the recvBuff
    recvBuff[airframe] = rcvdPower;
    updateSensitivity(airframe->getBitrate());

    // if receive power is bigger than sensitivity and if not sending
    // and currently not receiving another message and the message has
    // arrived in time
    // NOTE: a message may have arrival time in the past here when we are
    // processing ongoing transmissions during a channel change
    if (airframe->getArrivalTime() == simTime() && rcvdPower >= sensitivity && rs.getState() != RadioState::TRANSMIT && snrInfo.ptr == NULL)
    {
        EV << "receiving frame " << airframe->getName() << endl;

        // Put frame and related SnrList in receive buffer
        SnrList snrList;
        snrInfo.ptr = airframe;
        snrInfo.rcvdPower = rcvdPower;
        snrInfo.sList = snrList;

        // add initial snr value
        addNewSnr();

        if (rs.getState() != RadioState::RECV)
        {
            // publish new RadioState
            EV << "publish new RadioState:RECV\n";
            setRadioState(RadioState::RECV);
        }
    }
    // receive power is too low or another message is being sent or received
    else
    {
        EV << "frame " << airframe->getName() << " is just noise\n";
        //add receive power to the noise level
        noiseLevel += rcvdPower;

        // if a message is being received add a new snr value
        if (snrInfo.ptr != NULL)
        {
            // update snr info for currently being received message
            EV << "adding new snr value to snr list of message being received\n";
            addNewSnr();
        }

        // update the RadioState if the noiseLevel exceeded the threshold
        // and the radio is currently not in receive or in send mode
        if (BASE_NOISE_LEVEL >= sensitivity && rs.getState() == RadioState::IDLE)
        {
            EV << "setting radio state to RECV\n";
            setRadioState(RadioState::RECV);
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
 * If the corresponding AirFrame was not only noise the corresponding
 * SnrList and the AirFrame are sent to the decider.
 */
void Radio::handleLowerMsgEnd(AirFrame * airframe)
{
    // check if message has to be send to the decider
    if (snrInfo.ptr == airframe)
    {
        EV << "reception of frame over, preparing to send packet to upper layer\n";
        // get Packet and list out of the receive buffer:
        SnrList list;
        list = snrInfo.sList;

        // delete the pointer to indicate that no message is currently
        // being received and clear the list
        snrInfo.ptr = NULL;
        snrInfo.sList.clear();

        airframe->setSnr(10*log10(recvBuff[airframe]/ (BASE_NOISE_LEVEL))); //ahmed
        airframe->setLossRate(lossRate);
        // delete the frame from the recvBuff
        recvBuff.erase(airframe);

        //XXX send up the frame:
        //if (radioModel->isReceivedCorrectly(airframe, list))
        //    sendUp(airframe);
        //else
        //    delete airframe;
        if (!radioModel->isReceivedCorrectly(airframe, list))
        {
            airframe->getEncapsulatedPacket()->setKind(list.size()>1 ? COLLISION : BITERROR);
            airframe->setName(list.size()>1 ? "COLLISION" : "BITERROR");

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
        sendUp(airframe);
    }
    // all other messages are noise
    else
    {
        EV << "reception of noise message over, removing recvdPower from noiseLevel....\n";
        // get the rcvdPower and subtract it from the noiseLevel
        noiseLevel -= recvBuff[airframe];

        // delete message from the recvBuff
        recvBuff.erase(airframe);

        // update snr info for message currently being received if any
        if (snrInfo.ptr != NULL)
        {
            addNewSnr();
        }

        // message should be deleted
        delete airframe;
        EV << "message deleted\n";
    }

    // check the RadioState and update if necessary
    // change to idle if noiseLevel smaller than threshold and state was
    // not idle before
    // do not change state if currently sending or receiving a message!!!
    if (BASE_NOISE_LEVEL < sensitivity && rs.getState() == RadioState::RECV && snrInfo.ptr == NULL)
    {
        // publish the new RadioState:
        EV << "new RadioState is IDLE\n";
        setRadioState(RadioState::IDLE);
    }
}

void Radio::addNewSnr()
{
    SnrListEntry listEntry;     // create a new entry
    listEntry.time = simTime();
    listEntry.snr = snrInfo.rcvdPower / (BASE_NOISE_LEVEL);
    snrInfo.sList.push_back(listEntry);
}

void Radio::changeChannel(int channel)
{
    if (channel == rs.getChannelNumber())
        return;
    if (rs.getState() == RadioState::TRANSMIT)
        error("changing channel while transmitting is not allowed");

   // Clear the recvBuff
   for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
   {
        AirFrame *airframe = it->first;
        cMessage *endRxTimer = (cMessage *)airframe->getContextPointer();
        delete airframe;
        delete cancelEvent(endRxTimer);
    }
    recvBuff.clear();

    // clear snr info
    snrInfo.ptr = NULL;
    snrInfo.sList.clear();

    // reset the noiseLevel
    noiseLevel = thermalNoise;

    if (rs.getState()!=RadioState::IDLE)
        rs.setState(RadioState::IDLE); // Force radio to Idle

    // do channel switch
    EV << "Changing to channel #" << channel << "\n";

    emit(channelNumberSignal, channel);
    rs.setChannelNumber(channel);

    cc->setRadioChannel(myRadioRef, rs.getChannelNumber());

    cModule *myHost = findHost();

    //cGate *radioGate = myHost->gate("radioIn");

    cGate* radioGate = this->gate("radioIn")->getPathStartGate();

    EV << "RadioGate :" << radioGate->getFullPath() << " " << radioGate->getFullName() << endl;

    // pick up ongoing transmissions on the new channel
    EV << "Picking up ongoing transmissions on new channel:\n";
    IChannelControl::TransmissionList tlAux = cc->getOngoingTransmissions(channel);
    for (IChannelControl::TransmissionList::const_iterator it = tlAux.begin(); it != tlAux.end(); ++it)
    {
        AirFrame *airframe = check_and_cast<AirFrame *> (*it);
        // time for the message to reach us
        double distance = getRadioPosition().distance(airframe->getSenderPos());
        simtime_t propagationDelay = distance / 3.0E+8;

        // if this transmission is on our new channel and it would reach us in the future, then schedule it
        if (channel == airframe->getChannelNumber())
        {
            EV << " - (" << airframe->getClassName() << ")" << airframe->getName() << ": ";
        }

        // if there is a message on the air which will reach us in the future
        if (airframe->getTimestamp() + propagationDelay >= simTime())
        {
            EV << "will arrive in the future, scheduling it\n";

            // we need to send to each radioIn[] gate of this host
            //for (int i = 0; i < radioGate->size(); i++)
            //    sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() + i);

            // JcM Fix: we need to this radio only. no need to send the packet to each radioIn
            // since other radios might be not in the same channel
            sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() );
        }
        // if we hear some part of the message
        else if (airframe->getTimestamp() + airframe->getDuration() + propagationDelay > simTime())
        {
            EV << "missed beginning of frame, processing it as noise\n";

            AirFrame *frameDup = airframe->dup();
            frameDup->setArrivalTime(airframe->getTimestamp() + propagationDelay);
            handleLowerMsgStart(frameDup);
            bufferMsg(frameDup);
        }
        else
        {
            EV << "in the past\n";
        }
    }

    // notify other modules about the channel switch; and actually, radio state has changed too
    nb->fireChangeNotification(NF_RADIO_CHANNEL_CHANGED, &rs);
    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
}

void Radio::setBitrate(double bitrate)
{
    if (rs.getBitrate() == bitrate)
        return;
    if (bitrate < 0)
        error("setBitrate(): bitrate cannot be negative (%g)", bitrate);
    if (rs.getState() == RadioState::TRANSMIT)
        error("changing the bitrate while transmitting is not allowed");

    EV << "Setting bitrate to " << (bitrate/1e6) << "Mbps\n";
    emit(bitrateSignal, bitrate);
    rs.setBitrate(bitrate);

    //XXX fire some notification?
}

void Radio::setRadioState(RadioState::State newState)
{
    if (rs.getState() != newState)
    {
        emit(radioStateSignal, newState);
        if (rs.getState() != newState)
        {
            emit(radioStateSignal, newState);
            if (newState == RadioState::SLEEP)
            {
                disconnectTransceiver();
                disconnectReceiver();
            }
            else if (rs.getState() == RadioState::SLEEP)
            {
                connectTransceiver();
                connectReceiver(); // the connection change the state
                if (rs.getState() == newState)
                {
                    rs.setState(newState);
                    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
                    return;
                }
            }
        }
    }

    rs.setState(newState);
    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
}
/*
void Radio::updateSensitivity(double rate)
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

void Radio::updateSensitivity(double rate)
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
    EV<<"bitrate = "<<rate<<endl;
    EV <<" sensitivity after updateSensitivity: "<<sensitivity<<endl;
}

void Radio::registerBattery()
{
    BasicBattery *bat = BatteryAccess().getIfExists();
    if (bat)
    {
        //int id,double mUsageRadioIdle,double mUsageRadioRecv,double mUsageRadioSend,double mUsageRadioSleep)=0;
        // read parameters
        double mUsageRadioIdle = par("usage_radio_idle");
        double mUsageRadioRecv = par("usage_radio_recv");
        double mUsageRadioSleep = par("usage_radio_sleep");
        double mUsageRadioSend = par("usage_radio_send");
        if (mUsageRadioIdle<0 || mUsageRadioRecv<0 || mUsageRadioSleep<0 || mUsageRadioSend < 0)
            return;
        bat->registerWirelessDevice(rs.getRadioId(), mUsageRadioIdle, mUsageRadioRecv, mUsageRadioSend, mUsageRadioSleep);
    }
}

void Radio::updateDisplayString() {
    // draw the interference area and sensitivity area
    // according pathloss propagation only
    // we use the channel controller method to calculate interference distance
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
        d.setTagArg("r2", 0, (long) calcDistFreeSpace());
        d.setTagArg("r2", 2, "blue");
    }
    if (updateString==NULL && updateStringInterval>0)
        updateString = new cMessage("refresh timer");
    if (updateStringInterval>0)
        scheduleAt(simTime()+updateStringInterval, updateString);


}

void Radio::enablingInitialization() {
    this->connectReceiver();
    this->connectTransceiver();
}

void Radio::disablingInitialization() {
    this->disconnectReceiver();
    this->disconnectTransceiver();
}

double Radio::calcDistFreeSpace()
{
    //the carrier frequency used
    double carrierFrequency = getChannelControlPar("carrierFrequency");
    //signal attenuation threshold
    //path loss coefficient
    double alpha = getChannelControlPar("alpha");

    double waveLength = (SPEED_OF_LIGHT / carrierFrequency);
    //minimum power level to be able to physically receive a signal
    double minReceivePower = sensitivity;

    double interfDistance = pow(waveLength * waveLength * transmitterPower /
                         (16.0 * M_PI * M_PI * minReceivePower), 1.0 / alpha);
    return interfDistance;
}

void Radio::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    ChannelAccess::receiveSignal(source,signalID, obj);
    if (signalID == changeLevelNoise)
    {
        if (BASE_NOISE_LEVEL<sensitivity)
        {
            if (rs.getState()==RadioState::RECV && snrInfo.ptr==NULL)
                setRadioState(RadioState::IDLE);
        }
        else
        {
            if (rs.getState()!=RadioState::IDLE)
                setRadioState(RadioState::RECV);
        }
    }
}

void Radio::disconnectReceiver()
{
    receiverConnect = false;
    cc->disableReception(this->myRadioRef);
    if (rs.getState() == RadioState::TRANSMIT)
        error("changing channel while transmitting is not allowed");

   // Clear the recvBuff
   for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
   {
        AirFrame *airframe = it->first;
        cMessage *endRxTimer = (cMessage *)airframe->getContextPointer();
        delete airframe;
        delete cancelEvent(endRxTimer);
    }
    recvBuff.clear();

    // clear snr info
    snrInfo.ptr = NULL;
    snrInfo.sList.clear();
}

void Radio::connectReceiver()
{
    receiverConnect = true;
    cc->enableReception(this->myRadioRef);

    if (rs.getState()!=RadioState::IDLE)
        rs.setState(RadioState::IDLE); // Force radio to Idle

    cc->setRadioChannel(myRadioRef, rs.getChannelNumber());
    cModule *myHost = findHost();

    //cGate *radioGate = myHost->gate("radioIn");

    cGate* radioGate = this->gate("radioIn")->getPathStartGate();

    EV << "RadioGate :" << radioGate->getFullPath() << " " << radioGate->getFullName() << endl;

    // pick up ongoing transmissions on the new channel
    EV << "Picking up ongoing transmissions on new channel:\n";
    IChannelControl::TransmissionList tlAux = cc->getOngoingTransmissions(rs.getChannelNumber());
    for (IChannelControl::TransmissionList::const_iterator it = tlAux.begin(); it != tlAux.end(); ++it)
    {
        AirFrame *airframe = check_and_cast<AirFrame *> (*it);
        // time for the message to reach us
        double distance = getRadioPosition().distance(airframe->getSenderPos());
        simtime_t propagationDelay = distance / 3.0E+8;

        // if there is a message on the air which will reach us in the future
        if (airframe->getTimestamp() + propagationDelay >= simTime())
        {
            EV << " - (" << airframe->getClassName() << ")" << airframe->getName() << ": ";
            EV << "will arrive in the future, scheduling it\n";

            // we need to send to each radioIn[] gate of this host
            //for (int i = 0; i < radioGate->size(); i++)
            //    sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() + i);

            // JcM Fix: we need to this radio only. no need to send the packet to each radioIn
            // since other radios might be not in the same channel
            sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() );
        }
        // if we hear some part of the message
        else if (airframe->getTimestamp() + airframe->getDuration() + propagationDelay > simTime())
        {
            EV << "missed beginning of frame, processing it as noise\n";

            AirFrame *frameDup = airframe->dup();
            frameDup->setArrivalTime(airframe->getTimestamp() + propagationDelay);
            handleLowerMsgStart(frameDup);
            bufferMsg(frameDup);
        }
    }

    // notify other modules about the channel switch; and actually, radio state has changed too
    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
}


void Radio::getSensitivityList(cXMLElement* xmlConfig)
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
            const char* sensitivity = (*it)->getAttribute("Sensitivity");
            double rate = atof(bitRate);
            if (rate == 0)
                error("invalid bit rate");
            double sens = atof(sensitivity);
            sensitivityList[rate] = FWMath::dBm2mW(sens);

        }
        parameters = data->getElementsByTagName("Default");
        for(cXMLElementList::const_iterator it = parameters.begin();
            it != parameters.end(); it++)
        {
            const char* sensitivity = (*it)->getAttribute("Sensitivity");
            double sens = atof(sensitivity);
            sensitivityList[0.0] = FWMath::dBm2mW(sens);
        }

        SensitivityList::iterator it = sensitivityList.find(0.0);
        if (it == sensitivityList.end())
        {
            sensitivityList[0] = FWMath::dBm2mW(par("sensitivity").doubleValue());
        }
    } // end iterator loop
}

