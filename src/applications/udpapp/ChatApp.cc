//

/*
  Copyright Leonardo Maccari, 2011. This software has been developed
  for the PAF-FPE Project financed by EU and Provincia di Trento.
  See www.pervacy.eu or contact me at leonardo.maccari@unitn.it

  I'd be greatful if:
  - you keep the above copyright notice
  - you cite pervacy.eu if you reuse this code

  This file is part of ChatApp.

  ChatApp is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  ChatApp is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ChatApp.  If not, see <http://www.gnu.org/licenses/>.

  ChatApp is based on UDPBasicBurst by Alfonso Ariza and Andras Varga

*/



#include "ChatApp.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"


EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("ChooseDestAddrMode");
    if (!e) enums.getInstance()->add(e = new cEnum("ChooseDestAddrMode"));
    e->insert(ChatApp::ONCE, "once");
    e->insert(ChatApp::PER_BURST, "perBurst");
);

Define_Module(ChatApp);

int ChatApp::counter;

simsignal_t ChatApp::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t ChatApp::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t ChatApp::outOfOrderPkSignal = SIMSIGNAL_NULL;
simsignal_t ChatApp::dropPkSignal = SIMSIGNAL_NULL;
simsignal_t ChatApp::endToEndDelaySignal = SIMSIGNAL_NULL;
simsignal_t ChatApp::sendInterval = SIMSIGNAL_NULL;
simsignal_t ChatApp::bDuration = SIMSIGNAL_NULL;
simsignal_t ChatApp::burstInterval = SIMSIGNAL_NULL;
simsignal_t ChatApp::messageSize = SIMSIGNAL_NULL;
simsignal_t ChatApp::startedSessions = SIMSIGNAL_NULL;
simsignal_t ChatApp::answeredSessions = SIMSIGNAL_NULL;
simsignal_t ChatApp::numUDPErrorsSignal = SIMSIGNAL_NULL;
simsignal_t ChatApp::targetStatisticsSignal= SIMSIGNAL_NULL;


ChatApp::ChatApp()
{
    messageLengthPar = NULL;
    burstDurationPar = NULL;
    burstIntervalPar = NULL;
    messageFreqPar = NULL;
    timerNext = NULL;
}

ChatApp::~ChatApp()
{
    cancelAndDelete(timerNext);
}

void ChatApp::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;
    counter = 0;
    numSent = 0;
    numReceived = 0;
    numDeleted = 0;
    numDuplicated = 0;
    numUDPErrors = 0;
    maxSimTime = simtime_t().getMaxTime().dbl();

    delayLimit = par("delayLimit");
    simtime_t startTime = par("startTime");
    stopTime = par("stopTime");

    messageLengthPar = &par("messageLength");
    burstDurationPar = &par("burstDuration");
    burstIntervalPar = &par("burstInterval");
    messageFreqPar = &par("messageFreq");
    debugStats = par("debugStats").boolValue();
    nextSleep = startTime;
    nextBurst = startTime;
    nextPkt = startTime;
    graceTime = 1000; // grace time before erasing a burst from the list
    intervalUpperBound = par("intervalUpperBound").doubleValue();
    const char *addrModeStr = par("chooseDestAddrMode").stringValue();
    int addrMode = cEnum::get("ChooseDestAddrMode")->lookup(addrModeStr);
    if (addrMode == -1)
        throw cRuntimeError(this, "Invalid chooseDestAddrMode: '%s'", addrModeStr);
    chooseDestAddrMode = (ChooseDestAddrMode)addrMode;
    addressGeneratorModule = dynamic_cast<AddressGenerator*>(getParentModule()->getModuleByRelativePath("addressGenerator"));
    if (addressGeneratorModule == 0){
    	std::stringstream tmp;
    	tmp << getParentModule()->getFullPath() << " " ;
    	tmp << "Wrong path to the address generator!?!?";
    	error(tmp.str().c_str());
    }

    WATCH(numSent);
    WATCH(numReceived);
    WATCH(numDeleted);
    WATCH(numDuplicated);
    WATCH_MAP(burstList);


    localPort = par("localPort");
    destPort = par("destPort");
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    //bindToPort(localPort);

    isSource = (startTime >= 0); // Never start if startTime < 0

    timerNext = 0;
    if (isSource)
    {
        activeBurst = true;
        timerNext = new cMessage("ChatAppTimer");
        MsgContext * ctx = new MsgContext;
        ctx->start = true;
        timerNext->setContextPointer((void*)ctx);
        scheduleAt(startTime, timerNext);
    }

    sentPkSignal = registerSignal("sentPkBytes");
    rcvdPkSignal = registerSignal("rcvdPkBytes");
    outOfOrderPkSignal = registerSignal("outOfOrderPk");
    dropPkSignal = registerSignal("dropPkBytes");
    endToEndDelaySignal = registerSignal("endToEndDelay");
	startedSessions = registerSignal("startedSessions");
	answeredSessions = registerSignal("answeredSessions");
	numUDPErrorsSignal = registerSignal("numUDPErrorsSignal");

    if(debugStats){
    	sendInterval = registerSignal("sendInterval");
    	bDuration = registerSignal("bDuration");
    	burstInterval = registerSignal("bInterval");
    	messageSize = registerSignal("messageSize");
    	targetStatisticsSignal = registerSignal("targetStatisticsSignal");

    }

}


void ChatApp::chooseDestAddr(IPvXAddress & checkAddr)
{
	std::map<IPv4Address, int> currentList = addressGeneratorModule->gatherAddresses();
	if (currentList.empty()){
		checkAddr = IPv4Address();
		return;
	}

	if (currentList.find(checkAddr.get4()) != currentList.end())
		return;
	else{
		int rnd = uniform(0,currentList.size());
		std::map<IPv4Address, int>::iterator ii = currentList.begin();
		while (rnd > 0){
			ii++;
			rnd--;
		}
		checkAddr =  ii->first;
	}
	emit(targetStatisticsSignal, (checkAddr.get4().getInt()&0x000F));
}

IPvXAddress ChatApp::chooseDestAddr()
{
	std::map<IPv4Address, int> currentList = addressGeneratorModule->gatherAddresses();
	if (currentList.empty()){
		return IPvXAddress();
	}
	int rnd = uniform(0,currentList.size());
	std::map<IPv4Address, int>::iterator ii = currentList.begin();
	while (rnd > 0){
		ii++;
		rnd--;
	}
	emit(targetStatisticsSignal, (ii->first.getInt()&0x000F));
	return ii->first;

}

cPacket *ChatApp::createPacket()
{
    char msgName[32];
    sprintf(msgName, "ChatApp-%d", counter++);
    long msgByteLength = messageLengthPar->longValue();
    emit(messageSize, msgByteLength);
    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(msgByteLength);
    payload->addPar("sourceId") = getId();
    payload->addPar("msgId") = numSent;

    return payload;
}

void ChatApp::handleMessage(cMessage *msg)
{

	if (msg->isSelfMessage() && msg->getContextPointer() == 0)
		return;
	if (msg->isSelfMessage())
	{
		Burst currentBurst;
		IPvXAddress newAddr;
		BurstList::iterator ii;
		MsgContext* ctx = (MsgContext*)msg->getContextPointer();
		if (ctx->start == true){ // start a new burst
			newAddr = generateBurst(0); // chose target, add to burstList
			if(!newAddr.isUnspecified())
			{
				cMessage * shortTimer = new cMessage("Inter-packet-timer");
				MsgContext * newCtx = new MsgContext;
				newCtx->start = false;
				newCtx->target = newAddr;
				shortTimer->setContextPointer((void*)newCtx);
				simtime_t newintervalLength = messageFreqPar->doubleValue();
				simtime_t stoptime = burstList[newAddr].stopTime;
				if (newintervalLength > stoptime) // limit weibull,
					newintervalLength = stoptime;
				scheduleAt(simTime()+newintervalLength, shortTimer);
				emit(sendInterval, newintervalLength);
				emit(startedSessions,1);
			}
			// the papers show that the pareto distribution is matched only in a certain interval
			// roughly between 10m and 1d, I need to truncate it because it is quite slow fading.
			// The default upper bound is 2hours for realistic simulations. I'm also checking here
			// if time goes beyond time precision in omnet and give a clearer error
			int i = 0;
			double newintervalLength = burstIntervalPar->doubleValue();
			double now = simTime().dbl();
			while(!(newintervalLength<intervalUpperBound) || now+newintervalLength > maxSimTime){
				newintervalLength = burstIntervalPar->doubleValue();
				i++;
				if (i>100){
					error("You have a problem with the chosen parameters. I can't easily generate a random "
							"interval length lower than upper bound %d and below the time 	"
							"precision %f. Please use different parameters for traffic "
							"distributions or increase simulation time size", intervalUpperBound, maxSimTime);
				}
			} if (!newAddr.isUnspecified() && (now+newintervalLength > burstList[newAddr].stopTime.dbl()) )
				ev << "Warning! you chose burstInterval parameters that generate overlapping bursts! (" <<
				now+newintervalLength << "/" << burstList[newAddr].stopTime << ")";
			scheduleAt(simTime()+newintervalLength, msg);
			emit(burstInterval, newintervalLength);
			// reschedule start time

		}else // already present burst
		{
			ii = burstList.find(ctx->target); // this must exist
			simtime_t now = simTime();
			if (ii == burstList.end()){
				ev << "Not sending a message scheduled after the duration of its burst";
				delete msg;
			} else if (ii->second.stopTime <= now){ // this burst is dead
				// if we don't add a gracetime (period in which we keep receiving packets but
				// we don't send anymore), each received packet will generate a new burst from the receiver
				if (ii->second.stopTime + graceTime <= now){
					burstList.erase(ii);
					delete msg;
				} else
					scheduleAt(now+graceTime, msg); // wait for a graceTime, then erase the burst from the list
			} else{
				newAddr = generateBurst(&(ii->second));
				simtime_t newintervalLength = messageFreqPar->doubleValue();
				scheduleAt(simTime()+newintervalLength, msg);
				emit(sendInterval, newintervalLength);

			}
		}

	}
    else
    {        // process incoming packet
    	if (dynamic_cast<UDPDataIndication *>(msg->getControlInfo()) == 0){
    		ev << "Error Received" << std::endl;
    		numUDPErrors ++;
    		delete msg;
    		return;
    	}

    	UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    	IPvXAddress srcAddr = ctrl->getSrcAddr();
    	//IPvXAddress srcAddr = ((UDPControlInfo*)msg->getControlInfo())->getSrcAddr();
    	BurstList::iterator ii = burstList.find(srcAddr);
    	Burst * newBurst = 0;
    	if (ii == burstList.end()){ // first time we see this connection
    		simtime_t nextBurstD = burstDurationPar->doubleValue();
    		newBurst = new Burst(simTime() + nextBurstD,srcAddr); // new answer burst, give a non empty pointer because we need to specify the address
    		emit(bDuration, nextBurstD);
    		emit(answeredSessions,1);
    	    IPvXAddress newAddr = generateBurst(newBurst); // generate burst will add/remove to the map
    		if (!newAddr.isUnspecified()){ // may be a one-message answer burst
    			cMessage * tNext = new cMessage("ChatAppTimer");
    			MsgContext * ctx = new MsgContext;
    			ctx->start = false;
    			ctx->target = srcAddr;
    			tNext->setContextPointer((void*)ctx);
    			simtime_t newintervalLength = messageFreqPar->doubleValue();
    			simtime_t stoptime = burstList[newAddr].stopTime;
    			if (newintervalLength > stoptime) // limit weibull, variance is high in weibull
    				newintervalLength = stoptime;
    			scheduleAt(simTime()+newintervalLength, tNext);
    			emit(sendInterval, newintervalLength);
    			burstList[srcAddr] = *newBurst;
    		}
    	} else { // existing connection. We do nothing (bursts are stateless, each one has an indepentend SM)
    		// but we need to check if the burst bust be purged
    		simtime_t now = simTime();
    		if (ii->second.stopTime < now){ // this burst is dead
    			// if we don't add a gracetime (period in which we keep receiving packets but
    			// we don't send anymore), each received packet will generate a new burst from the receiver
    			if (ii->second.stopTime + graceTime < now)
    				burstList.erase(ii);// gracetime is over, from now on if we receive another
    			// packet, we consider it coming from a new burst, thus we
    			// generate a new answer burst.
    		}
    	}
    	processPacket(PK(msg));
    }


    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void ChatApp::processPacket(cPacket *msg)
{
    if (msg->getKind() == UDP_I_ERROR)
    {
        delete msg;
        return;
    }

    if (msg->hasPar("sourceId") && msg->hasPar("msgId"))
    {
        // duplicate control
        int moduleId = (int)msg->par("sourceId");
        int msgId = (int)msg->par("msgId");
        SourceSequence::iterator it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end())
        {
            if (it->second >= msgId)
            {
                EV << "Out of order packet: ";
                emit(outOfOrderPkSignal, msg);
                delete msg;
                numDuplicated++;
                return;
            }
            else
                it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
    }

    if (delayLimit > 0)
    {
        if (simTime() - msg->getTimestamp() > delayLimit)
        {
            EV << "Old packet: ";
            emit(dropPkSignal, msg);
            delete msg;
            numDeleted++;
            return;
        }
    }

    EV << "Received packet: ";
    emit(endToEndDelaySignal, simTime() - msg->getTimestamp());
    emit(rcvdPkSignal, msg);
    numReceived++;
    delete msg;
}

IPvXAddress ChatApp::generateBurst(struct Burst* currentBurst)
{
    simtime_t now = simTime();
    IPvXAddress retAddr;

    if (currentBurst == 0) // new burst
    {
        double burstDuration = burstDurationPar->doubleValue();
        if (burstDuration < 0.0)
            throw cRuntimeError(this, "The burstDuration parameter mustn't be smaller than 0");
        emit(bDuration, burstDuration);
        Burst * newBurst = new Burst(now+burstDuration);

        if (chooseDestAddrMode == ONCE)
        	 if(destAddr.isUnspecified()){
                 destAddr = chooseDestAddr();
        		 retAddr = destAddr;
        	 }
        	 else {
        		 IPvXAddress & destR = destAddr;
        		 chooseDestAddr(destR);
        		 retAddr = destAddr;
        	 }
        else if (chooseDestAddrMode == PER_BURST)
            retAddr = chooseDestAddr();
        if (!retAddr.isUnspecified()){
        	newBurst->destAddr = retAddr;
        	burstList[retAddr] = *newBurst;
        }
    } else
    	retAddr = currentBurst->destAddr;
    if (!retAddr.isUnspecified()){
    	cPacket *payload = createPacket();
    	payload->setTimestamp();
    	emit(sentPkSignal, payload);
        socket.sendTo(payload, retAddr, destPort);
    	numSent++;
    }
    return retAddr;

}

void ChatApp::finish()
{
 emit(numUDPErrorsSignal, numUDPErrors);
}

