//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "BMacLayer.h"
#include "FWMath.h"
#include "Ieee802Ctrl_m.h"
#include "MACAddress.h"
#include "Ieee802Ctrl_m.h"
#include "PhyControlInfo_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"


static uint64_t MacToUint64(const MACAddress &add)
{
    uint64_t aux;
    uint64_t lo=0;
    for (int i=0; i<MAC_ADDRESS_BYTES; i++)
    {
        aux  = add.getAddressByte(MAC_ADDRESS_BYTES-i-1);
        aux <<= 8*i;
        lo  |= aux ;
    }
    return lo;
}

static MACAddress Uint64ToMac(uint64_t lo)
{
    MACAddress add;
    add.setAddressByte(0, (lo>>40)&0xff);
    add.setAddressByte(1, (lo>>32)&0xff);
    add.setAddressByte(2, (lo>>24)&0xff);
    add.setAddressByte(3, (lo>>16)&0xff);
    add.setAddressByte(4, (lo>>8)&0xff);
    add.setAddressByte(5, lo&0xff);
    return add;
}

Define_Module(BMacLayer)
BMacLayer::BMacLayer()
{
    queueModule=NULL;
    wakeup=NULL;
    data_timeout=NULL;
    data_tx_over=NULL;
    stop_preambles=NULL;
    send_preamble=NULL;
    ack_tx_over=NULL;
    cca_timeout=NULL;
    send_ack=NULL;
    start_bmac=NULL;
    ack_timeout=NULL;
    resend_data=NULL;
}

BMacLayer::~BMacLayer()
{

	if (wakeup) cancelAndDelete(wakeup);
    if (data_timeout) cancelAndDelete(data_timeout);
    if (data_tx_over) cancelAndDelete(data_tx_over);
    if (stop_preambles) cancelAndDelete(stop_preambles);
    if (send_preamble) cancelAndDelete(send_preamble);
    if (ack_tx_over) cancelAndDelete(ack_tx_over);
    if (cca_timeout) cancelAndDelete(cca_timeout);
    if (send_ack) cancelAndDelete(send_ack);
    if (start_bmac) cancelAndDelete(start_bmac);
    if (ack_timeout) cancelAndDelete(ack_timeout);
    if (resend_data) cancelAndDelete(resend_data);

    wakeup=NULL;
    data_timeout=NULL;
    data_tx_over=NULL;
    stop_preambles=NULL;
    send_preamble=NULL;
    ack_tx_over=NULL;
    cca_timeout=NULL;
    send_ack=NULL;
    start_bmac=NULL;
    ack_timeout=NULL;
    resend_data=NULL;

    while (!macQueue.empty())
    {
        delete macQueue.front();
        macQueue.pop_front();
    }
    macQueue.clear();

}


void BMacLayer::registerInterface()
{


    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;
    InterfaceEntry *e = new InterfaceEntry();

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
    e->setMACAddress(macAddress);
    e->setInterfaceToken(macAddress.formInterfaceIdentifier());

    // FIXME: MTU on 802.11 = ?
    e->setMtu(aMaxMACFrameSize);

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    ift->addInterface(e, this);
}
/**
 * Initialize method of BMacLayer. Init all parameters, schedule timers.
 */
void BMacLayer::initialize(int stage)
{
    WirelessMacBase::initialize(stage);

    if (stage == 0) {
    	queueModule=NULL;
        L2BROADCAST = MacToUint64(MACAddress::BROADCAST_ADDRESS);

        queueLength = hasPar("queueLength") ? par("queueLength") : 10;
        animation = hasPar("animation") ? par("animation") : true;
        animationBubble = hasPar("animationBubble") ? par("animationBubble") : true;
        slotDuration = hasPar("slotDuration") ? par("slotDuration") : 1;
        bitrate = hasPar("bitrate") ? par("bitrate") : 15360;
        headerLength = hasPar("headerLength") ? par("headerLength") : 10;
        checkInterval = hasPar("checkInterval") ? par("checkInterval") : 0.1;
        txPower = hasPar("txPower") ? par("txPower") : 50;
        useMacAcks = hasPar("useMACAcks") ? par("useMACAcks") : false;
        maxTxAttempts = hasPar("maxTxAttempts") ? par("maxTxAttempts") : 2;
        EV << "headerLength: " << headerLength << ", bitrate: " << bitrate << endl;

        useIeee802Ctrl = par("useIeee802Ctrl");
        stats = par("stats");
        nbTxDataPackets = 0;
        nbTxPreambles = 0;
        nbRxDataPackets = 0;
        nbRxPreambles = 0;
        nbMissedAcks = 0;
        nbRecvdAcks=0;
        nbDroppedDataPackets=0;
        nbTxAcks=0;

        txAttempts = 0;
        lastDataPktDestAddr = L2BROADCAST;
        lastDataPktSrcAddr = L2BROADCAST;

        const char *addressString = par("address");
        if (!strcmp(addressString, "auto"))
        {
            // assign automatic address
            macAddress = MACAddress::generateAutoAddress();
            // change module parameter from "auto" to concrete address
            par("address").setStringValue(macAddress.str().c_str());
        }
        else
            macAddress.setAddress(addressString);
        myMacAddr = MacToUint64(macAddress);
        EV << "Mac Address " <<  macAddress << "My Mac Address " << myMacAddr << endl;

        // get a pointer to the NotificationBoard module
        mpNb = NotificationBoardAccess().get();
        // subscribe for the information of the carrier sense
        mpNb->subscribe(this, NF_RADIOSTATE_CHANGED);
        //mpNb->subscribe(this, NF_BITRATE_CHANGED);
        mpNb->subscribe(this, NF_RADIO_CHANNEL_CHANGED);
        radioState = RadioState::IDLE;

        macState = INIT;
        initializeQueueModule();
        registerInterface();

        // init the dropped packet info
        //droppedPacket.setReason(DroppedPacket::NONE);
        nicId = getParentModule()->getId();

        //catDroppedPacket = utility->getCategory(&droppedPacket);
        WATCH(macState);
        WATCH(myMacAddr);
    }

    else if(stage == 1) {

        wakeup = new cMessage("wakeup");
        wakeup->setKind(BMAC_WAKE_UP);

        data_timeout = new cMessage("data_timeout");
        data_timeout->setKind(BMAC_DATA_TIMEOUT);

        data_tx_over = new cMessage("data_tx_over");
        data_tx_over->setKind(BMAC_DATA_TX_OVER);

        stop_preambles = new cMessage("stop_preambles");
        stop_preambles->setKind(BMAC_STOP_PREAMBLES);

        send_preamble = new cMessage("send_preamble");
        send_preamble->setKind(BMAC_SEND_PREAMBLE);

        ack_tx_over = new cMessage("ack_tx_over");
        ack_tx_over->setKind(BMAC_ACK_TX_OVER);

        cca_timeout = new cMessage("cca_timeout");
        cca_timeout->setKind(BMAC_CCA_TIMEOUT);

        send_ack = new cMessage("send_ack");
        send_ack->setKind(BMAC_SEND_ACK);

        start_bmac = new cMessage("start_bmac");
        start_bmac->setKind(BMAC_START_BMAC);

        ack_timeout = new cMessage("ack_timeout");
        ack_timeout->setKind(BMAC_ACK_TIMEOUT);

        resend_data = new cMessage("resend_data");
        resend_data->setKind(BMAC_RESEND_DATA);
        double maxFrameTime = (aMaxMACFrameSize*8)/bitrate;
        if (slotDuration<=2*maxFrameTime)
        {
           EV << " !!!" <<endl;
           EV << " !!! slotDuration" << slotDuration << " and the maximum time need to transmit a frame is " <<maxFrameTime <<endl;
           EV << " !!! Possibility of problems" <<endl;
        }


        scheduleAt(0.0, start_bmac);
    }
}

void BMacLayer::finish() {
    MacQueue::iterator it;
    WirelessMacBase::finish();

    cancelAndDelete(wakeup);
    cancelAndDelete(data_timeout);
    cancelAndDelete(data_tx_over);
    cancelAndDelete(stop_preambles);
    cancelAndDelete(send_preamble);
    cancelAndDelete(ack_tx_over);
    cancelAndDelete(cca_timeout);
    cancelAndDelete(send_ack);
    cancelAndDelete(start_bmac);
    cancelAndDelete(ack_timeout);
    cancelAndDelete(resend_data);

    wakeup=NULL;
    data_timeout=NULL;
    data_tx_over=NULL;
    stop_preambles=NULL;
    send_preamble=NULL;
    ack_tx_over=NULL;
    cca_timeout=NULL;
    send_ack=NULL;
    start_bmac=NULL;
    ack_timeout=NULL;
    resend_data=NULL;

    for(it = macQueue.begin(); it != macQueue.end(); ++it)
    {
        delete (*it);
    }
    macQueue.clear();
    if (stats)
    {
        recordScalar("nbTxDataPackets", nbTxDataPackets);
        recordScalar("nbTxPreambles", nbTxPreambles);
        recordScalar("nbRxDataPackets", nbRxDataPackets);
        recordScalar("nbRxPreambles", nbRxPreambles);
        recordScalar("nbMissedAcks", nbMissedAcks);
        recordScalar("nbRecvdAcks", nbRecvdAcks);
        recordScalar("nbTxAcks", nbTxAcks);
        recordScalar("nbDroppedDataPackets", nbDroppedDataPackets);
        //recordScalar("timeSleep", timeSleep);
        //recordScalar("timeRX", timeRX);
        //recordScalar("timeTX", timeTX);
    }
}

/**
 * Check whether the queue is not full: if yes, print a warning and drop the packet.
 * Then initiate sending of the packet, if the node is sleeping. Do nothing, if node is working.
 */
void BMacLayer::handleUpperMsg(cPacket *msg)
{
    reqtMsgFromQueue();
    bool pktAdded = addToQueue(msg);
    if (!pktAdded)
        return;
    // force wakeup now
    if (wakeup->isScheduled() && (macState == SLEEP))
    {
        cancelEvent(wakeup);
        scheduleAt(simTime() + dblrand()*0.1f, wakeup);
    }
}

/**
 * Send one short preamble packet immediately.
 */
void BMacLayer::sendPreamble()
{
    BmacPkt* preamble = new BmacPkt();
    preamble->setSrcAddr(myMacAddr);
    preamble->setDestAddr(L2BROADCAST);
    preamble->setKind(BMAC_PREAMBLE);
    preamble->setBitLength(headerLength);
    preamble->setPacketType(BMAC_PREAMBLE);

    //attach signal and send down
    attachSignal(preamble);
    sendDown(preamble);
    nbTxPreambles++;
}

/**
 * Send one short preamble packet immediately.
 */
void BMacLayer::sendMacAck()
{
    BmacPkt* ack = new BmacPkt();
    ack->setSrcAddr(myMacAddr);
    ack->setDestAddr(lastDataPktSrcAddr);
    ack->setKind(BMAC_ACK);
    ack->setBitLength(headerLength);
    ack->setPacketType(BMAC_ACK);

    //attach signal and send down
    attachSignal(ack);
    sendDown(ack);
    nbTxAcks++;
    //endSimulation();
}



/**
 * Handle own messages:
 * BMAC_WAKEUP: wake up the node, check the channel for some time.
 * BMAC_CHECK_CHANNEL: if the channel is free, check whether there is something in the queue and switch teh radio to TX.
 * when switched to TX, (in receiveBBItem), the node will start sending preambles for a full slot duration.
 * if the channel is busy, stay awake to receive message. Schedule a timeout to handle false alarms.
 * BMAC_SEND_PREAMBLES: sending of preambles over. Next time the data packet will be send out (single one).
 * BMAC_TIMEOUT_DATA: timeout the node after a false busy channel alarm. Go back to sleep.
 */
void BMacLayer::handleSelfMsg(cMessage *msg)
{
    switch (macState)
    {
    case INIT:
        if (msg->getKind() == BMAC_START_BMAC)
        {
            EV << "State INIT, message BMAC_START, new state SLEEP" << endl;
            changeDisplayColor(BLACK);
            showBuble(B_SLEEP);
            //phy->setRadioState(Radio::SLEEP);
            PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
            macState = SLEEP;
            scheduleAt(simTime()+dblrand()*slotDuration, wakeup);
            return;
        }
        break;
    case SLEEP:
        if (msg->getKind() == BMAC_WAKE_UP)
        {
            EV << "State SLEEP, message BMAC_WAKEUP, new state CCA" << endl;
            scheduleAt(simTime() + checkInterval, cca_timeout);
            //phy->setRadioState(Radio::RX);
            PLME_SET_TRX_STATE_request(phy_RX_ON);
            showBuble(B_RX);
            changeDisplayColor(GREEN);
            macState = CCA;
            return;
        }
        break;
    case CCA:
    	if (msg->getKind() == BMAC_CCA_TIMEOUT && radioState==RadioState::RECV)
    	{
            scheduleAt(simTime() + checkInterval, cca_timeout);
            return;
    	}
        if (msg->getKind() == BMAC_CCA_TIMEOUT)
        {
            // channel is clear
            // something waiting in eth queue?
            if (macQueue.size() > 0)
            {
                EV << "State CCA, message CCA_TIMEOUT, new state SEND_PREAMBLE" << endl;
                //phy->setRadioState(Radio::TX);
                PLME_SET_TRX_STATE_request(phy_TX_ON);
                changeDisplayColor(BLUE);
                showBuble(B_TXPREAMBLE);
                macState = SEND_PREAMBLE;
                scheduleAt(simTime() + slotDuration, stop_preambles);
                return;
            }
            // if not, go back to sleep and wake up after a full period
            else
            {
                EV << "State CCA, message CCA_TIMEOUT, new state SLEEP" << endl;
                scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                //phy->setRadioState(Radio::SLEEP);
                PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                showBuble(B_SLEEP);
                changeDisplayColor(BLACK);
                return;
            }
        }
        // during CCA, we received a preamble. Go to state WAIT_DATA and schedule the timeout.
        if (msg->getKind() == BMAC_PREAMBLE)
        {
            nbRxPreambles++;
            EV << "State CCA, message BMAC_PREAMBLE received, new state WAIT_DATA" << endl;
            macState = WAIT_DATA;
            showBuble(B_RXPREAMBLE);
            changeDisplayColor(YELLOW);
            cancelEvent(cca_timeout);
            double maxFrameTime = (aMaxMACFrameSize*8)/bitrate;
            if (maxFrameTime>checkInterval)
                scheduleAt(simTime() + slotDuration + maxFrameTime, data_timeout);
            else
                scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
            return;
        }
        // this case is very, very, very improbable, but let's do it.
        // if in CCA and the node receives directly the data packet, switch to state WAIT_DATA
        // and re-send the message
        if (msg->getKind() == BMAC_DATA)
        {
            nbRxDataPackets++;
            EV << "State CCA, message BMAC_DATA, new state WAIT_DATA" << endl;
            macState = WAIT_DATA;
            showBuble(B_RXDATA);
            changeDisplayColor(YELLOW);
            cancelEvent(cca_timeout);
            scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
            scheduleAt(simTime(), msg->dup());
            return;
        }
        //in case we get an ACK, we simply dicard it, because it means the end of another communication
        if (msg->getKind() == BMAC_ACK)
        {
            EV << "State CCA, message BMAC_ACK, new state CCA" << endl;
            return;
        }
        break;

    case SEND_PREAMBLE:
    	changeDisplayColor(BLUE);
        if (msg->getKind() == BMAC_SEND_PREAMBLE)
        {
            EV << "State SEND_PREAMBLE, message BMAC_SEND_PREAMBLE, new state SEND_PREAMBLE" << endl;
            sendPreamble();
            showBuble(B_TXPREAMBLE);
            scheduleAt(simTime() + 0.5f*checkInterval, send_preamble);
            macState = SEND_PREAMBLE;
            return;
        }
        // simply change the state to SEND_DATA
        if (msg->getKind() == BMAC_STOP_PREAMBLES)
        {
            EV << "State SEND_PREAMBLE, message BMAC_STOP_PREAMBLES, new state SEND_DATA" << endl;
            macState = SEND_DATA;
            txAttempts = 1;
            return;
        }
        break;
    case SEND_DATA:
    	changeDisplayColor(BLUE);
        if ((msg->getKind() == BMAC_SEND_PREAMBLE) || (msg->getKind() == BMAC_RESEND_DATA))
        {
            EV << "State SEND_DATA, message BMAC_SEND_PREAMBLE or BMAC_RESEND_DATA, new state WAIT_TX_DATA_OVER" << endl;
            // send the data packet
            sendDataPacket();
            showBuble(B_TXDATA);
            macState = WAIT_TX_DATA_OVER;
            return;
        }
        break;

    case WAIT_TX_DATA_OVER:
        if (msg->getKind() == BMAC_DATA_TX_OVER)
        {
            if ((useMacAcks) && (lastDataPktDestAddr != L2BROADCAST))
            {
                EV << "State WAIT_TX_DATA_OVER, message BMAC_DATA_TX_OVER, new state WAIT_ACK" << endl;
                macState = WAIT_ACK;
                //phy->setRadioState(Radio::RX);
                PLME_SET_TRX_STATE_request(phy_RX_ON);
                showBuble(B_RX);
                changeDisplayColor(GREEN);
                scheduleAt(simTime()+checkInterval, ack_timeout);
            }
            else
            {
                EV << "State WAIT_TX_DATA_OVER, message BMAC_DATA_TX_OVER, new state  SLEEP" << endl;
                macQueue.pop_front();
                // if something in the queue, wakeup soon.
                if (macQueue.size() > 0)
                    scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
                else
                    scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                showBuble(B_SLEEP);
                //phy->setRadioState(Radio::SLEEP);
                PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                changeDisplayColor(BLACK);
            }
            return;
        }
        break;
      case WAIT_ACK:
    	showBuble(B_WAITACK);
        if (msg->getKind() == BMAC_ACK_TIMEOUT)
        {
            // No ACK received. try again or drop.
            if (txAttempts < maxTxAttempts)
            {
                EV << "State WAIT_ACK, message BMAC_ACK_TIMEOUT, new state SEND_DATA" << endl;
                txAttempts++;
                macState = SEND_PREAMBLE;
                scheduleAt(simTime() + slotDuration, stop_preambles);
                //phy->setRadioState(Radio::TX);
                PLME_SET_TRX_STATE_request(phy_TX_ON);
                changeDisplayColor(BLUE);
            }
            else
            {
                EV << "State WAIT_ACK, message BMAC_ACK_TIMEOUT, new state SLEEP" << endl;
                //drop the packet
                cMessage * mac = macQueue.front();
                macQueue.pop_front();
                txAttempts = 0;
                mpNb->fireChangeNotification(NF_LINK_BREAK, mac);
                delete mac;
                // if something in the queue, wakeup soon.
                if (macQueue.size() > 0)
                    scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
                else
                    scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                showBuble(B_SLEEP);
                //phy->setRadioState(Radio::SLEEP);
                PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                changeDisplayColor(BLACK);
                nbMissedAcks++;
            }
            return;
        }
        //ignore and other packets
        if ((msg->getKind() == BMAC_DATA) || (msg->getKind() == BMAC_PREAMBLE))
        {
            EV << "State WAIT_ACK, message BMAC_DATA or BMAC_PREMABLE, new state WAIT_ACK" << endl;
            return;
        }
        if (msg->getKind() == BMAC_ACK)
        {
            EV << "State WAIT_ACK, message BMAC_ACK" << endl;
            BmacPkt *mac = static_cast<BmacPkt *>(msg);
            uint64_t src = mac->getSrcAddr();
            // the right ACK is received..
            EV << "We are waiting for ACK from : " << lastDataPktDestAddr << ", and ACK came from : " << src << endl;
            if (src == lastDataPktDestAddr)
            {
                EV << "New state SLEEP" << endl;
                nbRecvdAcks++;
                lastDataPktDestAddr = L2BROADCAST;
                cancelEvent(ack_timeout);
                macQueue.pop_front();
                // if something in the queue, wakeup soon.
                if (macQueue.size() > 0)
                    scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
                else
                    scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                showBuble(B_SLEEP);
                //phy->setRadioState(Radio::SLEEP);
                PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                changeDisplayColor(BLACK);
                lastDataPktDestAddr = L2BROADCAST;
            }
            return;
        }
        break;
    case WAIT_DATA:
        if(msg->getKind() == BMAC_PREAMBLE)
        {
            //nothing happens
        	showBuble(B_RXPREAMBLE);
            EV << "State WAIT_DATA, message BMAC_PREAMBLE, new state WAIT_DATA" << endl;
            nbRxPreambles++;
            return;
        }
        if(msg->getKind() == BMAC_ACK)
        {
            //nothing happens
        	showBuble(B_RXACK);
            EV << "State WAIT_DATA, message BMAC_ACK, new state WAIT_DATA" << endl;
            return;
        }
        if (msg->getKind() == BMAC_DATA)
        {
            nbRxDataPackets++;
            showBuble(B_RXDATA);
            BmacPkt *mac = static_cast<BmacPkt *>(msg);
            uint64_t dest = mac->getDestAddr();
            uint64_t src = mac->getSrcAddr();
            if ((dest == myMacAddr) || (dest == L2BROADCAST))
            {
                mpNb->fireChangeNotification(NF_LINK_PROMISCUOUS, mac);
                sendUp(decapsMsg(mac));
            }

            cancelEvent(data_timeout);
            if ((useMacAcks) && (dest == myMacAddr))
            {
                EV << "State WAIT_DATA, message BMAC_DATA, new state SEND_ACK" << endl;
                macState = SEND_ACK;
                lastDataPktSrcAddr = src;
                //phy->setRadioState(Radio::TX);
                PLME_SET_TRX_STATE_request(phy_TX_ON);
                changeDisplayColor(YELLOW);
            }
            else
            {
                EV << "State WAIT_DATA, message BMAC_DATA, new state SLEEP" << endl;
                // if something in the queue, wakeup soon.
                if (macQueue.size() > 0)
                    scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
                else
                    scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                showBuble(B_SLEEP);
                //phy->setRadioState(Radio::SLEEP);
                PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                changeDisplayColor(BLACK);
            }
            return;
        }
        if (msg->getKind() == BMAC_DATA_TIMEOUT)
        {
            EV << "State WAIT_DATA, message BMAC_DATA_TIMEOUT, new state SLEEP" << endl;
            // if something in the queue, wakeup soon.
            if (macQueue.size() > 0)
                scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
            else
                scheduleAt(simTime() + slotDuration, wakeup);
            macState = SLEEP;
            showBuble(B_SLEEP);
            //phy->setRadioState(Radio::SLEEP);
            PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
            changeDisplayColor(BLACK);
            return;
        }
        break;
    case SEND_ACK:
        if (msg->getKind() == BMAC_SEND_ACK)
        {
            EV << "State SEND_ACK, message BMAC_SEND_ACK, new state WAIT_ACK_TX" << endl;
            // send now the ack packet
            showBuble(B_TXACK);
            sendMacAck();
            macState = WAIT_ACK_TX;
            return;
        }
        break;
    case WAIT_ACK_TX:
        if (msg->getKind() == BMAC_ACK_TX_OVER)
        {
            EV << "State WAIT_ACK_TX, message BMAC_ACK_TX_OVER, new state SLEEP" << endl;
            // ack sent, go to sleep now.
            // if something in the queue, wakeup soon.
            if (macQueue.size() > 0)
                scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
            else
                scheduleAt(simTime() + slotDuration, wakeup);
            macState = SLEEP;
            showBuble(B_SLEEP);
            //phy->setRadioState(Radio::SLEEP);
            PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
            changeDisplayColor(BLACK);
            lastDataPktSrcAddr = L2BROADCAST;
            return;
        }
        break;
    }
    EV << "Undefined event  of type " << msg->getKind() << "in state " << macState << endl;
    endSimulation();
}


/**
 * Handle BMAC preambles and received data packets.
 */
void BMacLayer::handleLowerMsg(cPacket *msg)
{
    mpNb->fireChangeNotification(NF_LINK_FULL_PROMISCUOUS, msg);
    // simply pass the massage as self message, to be processed by the FSM.
    if (msg->isPacket())
    {
       if (msg->getKind()!=PACKETOK)
       {
    	   EV << " Packet received with errors \n" ;
    	   delete msg;
    	   return;
       }
       BmacPkt *pkt = dynamic_cast<BmacPkt*>(msg);
       if (pkt==NULL)
       {
    	   EV << " not BmacPkt \n" ;
    	   delete msg;
    	   return;
       }
       pkt->setKind(pkt->getPacketType());
    }
    handleSelfMsg(msg);
    delete msg;
}

void BMacLayer::sendDataPacket()
{
    nbTxDataPackets++;
    BmacPkt *pkt = macQueue.front()->dup();
    attachSignal(pkt);
    lastDataPktDestAddr = pkt->getDestAddr();
    pkt->setKind(BMAC_DATA);
    pkt->setPacketType(BMAC_DATA);
    sendDown(pkt);
}

/**
 * Handle transmission over messages: either send another preambles or the data packet itself.
 */
void BMacLayer::handleCommand(cMessage *msg)
{
    // Transmission of one packet is over
    Ieee802154MacPhyPrimitives *primitive = dynamic_cast<Ieee802154MacPhyPrimitives*>(msg);
    if (!primitive)
    {
        delete msg;
        return;
    }
    int kind =  primitive->getKind();
    int status = primitive->getStatus();
    //int atribute =  primitive->getAttribute();
    if(kind == PD_DATA_CONFIRM && status==phy_SUCCESS  && primitive->getAdditional()==TX_OVER) {
        if (macState == WAIT_TX_DATA_OVER)
        {
            scheduleAt(simTime(), data_tx_over);
        }
        if (macState == WAIT_ACK_TX)
        {
            scheduleAt(simTime(), ack_tx_over);
        }
    }
    else if (msg->getKind() == PLME_SET_TRX_STATE_CONFIRM)
    {
        Ieee802154MacPhyPrimitives* primitive = check_and_cast<Ieee802154MacPhyPrimitives *>(msg);
        phystatus = PHYenum(primitive->getStatus());
        if (primitive->getStatus()==phy_TX_ON)
        {
        	//if (radioState == RadioState::TRANSMIT)
        	{
               if (macState == SEND_PREAMBLE)
               {
                   scheduleAt(simTime(), send_preamble);
           	   }
               if (macState == SEND_ACK)
               {
                   scheduleAt(simTime(), send_ack);
               }
           	// we were waiting for acks, but none came. we switched to TX and now need to resend data
               if (macState == SEND_DATA)
               {
                   scheduleAt(simTime(), resend_data);
               }
            }
        }
    }
    else {
        EV << "control message with wrong kind -- deleting\n";
    }
    delete msg;
}




/**
 * Encapsulates the received network-layer packet into a BmacPkt and set all needed
 * header fields.
 */
bool BMacLayer::addToQueue(cMessage *msg)
{
    BmacPkt *macPkt = new BmacPkt(msg->getName());
    macPkt->setBitLength(headerLength);
    uint64_t dest;
    cObject *controlInfo = msg->removeControlInfo();
    if (dynamic_cast<Ieee802Ctrl *>(controlInfo))
    {
          Ieee802Ctrl* cInfo = check_and_cast<Ieee802Ctrl *>(controlInfo);
          MACAddress destination = cInfo->getDest();
          dest = MacToUint64(destination);
    }
    else
    {
        Ieee802154NetworkCtrlInfo* cInfo = check_and_cast<Ieee802154NetworkCtrlInfo *>(controlInfo);
        dest=cInfo->getNetwAddr();
    }
    delete controlInfo;
    //EV<<"CSMA received a message from upper layer, name is " << msg->getName() <<", CInfo removed, mac addr="<< cInfo->getNextHopMac()<<endl;
    macPkt->setDestAddr(dest);
    macPkt->setSrcAddr(myMacAddr);

    ASSERT(static_cast<cPacket*>(msg));
    macPkt->encapsulate(static_cast<cPacket*>(msg));

    if (macQueue.size() < queueLength) {
        macQueue.push_back(macPkt);
        EV << "Max queue length: " << queueLength << ", packet put in queue\n  queue size: " << macQueue.size() << " macState: " << macState << endl;
        return true;
    }
    else {
        // queue is full, message has to be deleted
        EV << "New packet arrived, but queue is FULL, so new packet is deleted\n";
        delete macPkt;
        //msg->setName("MAC ERROR");
        //msg->setKind(PACKET_DROPPED);
        //sendControlUp(msg);
        //droppedPacket.setReason(DroppedPacket::QUEUE);
        //utility->publishBBItem(catDroppedPacket, &droppedPacket, nicId);
        nbDroppedDataPackets++;
    }
    return false;

}

void BMacLayer::attachSignal(BmacPkt *macPkt)
{
    //calc signal duration
    //create and initialize control info
    PhyControlInfo* ctrl = new PhyControlInfo();
    ctrl->setBitrate(bitrate);
    ctrl->setTransmitterPower(txPower);
    macPkt->setControlInfo(ctrl);
}

/**
 * Change the color of the node for animation purposes.
 */
void BMacLayer::showBuble(BMAC_BUBBLE bubble)
{
    if (!animationBubble)
        return;
    if (!ev.isGUI())
        return;

    cDisplayString& dispStr = getParentModule()->getParentModule()->getDisplayString();
    //b=40,40,rect,black,black,2"
    switch(bubble)
    {
     case B_RX:
        getParentModule()->getParentModule()->bubble("RX State");
        break;
     case B_TXPREAMBLE:
        getParentModule()->getParentModule()->bubble("TX preamble");
        break;
     case B_TXDATA:
        getParentModule()->getParentModule()->bubble("TX data");
        break;
     case B_TXACK:
        getParentModule()->getParentModule()->bubble("TX ACK");
        break;
     case B_RXPREAMBLE:
        getParentModule()->getParentModule()->bubble("RX preamble");
        break;
     case B_RXDATA:
        getParentModule()->getParentModule()->bubble("RX Data");
        break;
     case B_RXACK:
        getParentModule()->getParentModule()->bubble("RX ACK");
        break;
     case B_WAITACK:
        getParentModule()->getParentModule()->bubble("WAIT ACK");
        break;
     case B_SLEEP:
        getParentModule()->getParentModule()->bubble("Sleep");
        break;
    }
}

void BMacLayer::changeDisplayColor(BMAC_COLORS color)
{
    if (!animation)
        return;
    cDisplayString& dispStr = getParentModule()->getParentModule()->getDisplayString();
    //b=40,40,rect,black,black,2"
    if (color == GREEN)
        dispStr.parse("b=40,40,rect,green,green,2");
    if (color == BLUE)
            dispStr.parse("b=40,40,rect,blue,blue,2");
    if (color == RED)
            dispStr.parse("b=40,40,rect,red,red,2");
    if (color == BLACK)
            dispStr.parse("b=40,40,rect,black,black,2");
    if (color == YELLOW)
                dispStr.parse("b=40,40,rect,yellow,yellow,2");
}

/*void BMacLayer::changeMacState(States newState)
{
    switch (macState)
    {
    case RX:
        timeRX += (simTime() - lastTime);
        break;
    case TX:
        timeTX += (simTime() - lastTime);
        break;
    case SLEEP:
        timeSleep += (simTime() - lastTime);
        break;
    case CCA:
        timeRX += (simTime() - lastTime);
    }
    lastTime = simTime();

    switch (newState)
    {
    case CCA:
        changeDisplayColor(GREEN);
        break;
    case TX:
        changeDisplayColor(BLUE);
        break;
    case SLEEP:
        changeDisplayColor(BLACK);
        break;
    case RX:
        changeDisplayColor(YELLOW);
        break;
    }

    macState = newState;
}*/

void BMacLayer::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    switch (category)
    {

        if (check_and_cast<RadioState *>(details)->getRadioId()!=getRadioModuleId())
            return;
    case NF_RADIOSTATE_CHANGED:
           radioState = check_and_cast<RadioState *>(details)->getState();
           if ((macState == SEND_PREAMBLE) && (radioState == RadioState::TRANSMIT))
            {
        	    if (!send_preamble->isScheduled())
                scheduleAt(simTime(), send_preamble);
            }
            if ((macState == SEND_ACK) && (radioState== RadioState::TRANSMIT))
            {
            	if (!send_ack->isScheduled())
                  scheduleAt(simTime(), send_ack);
            }
            // we were waiting for acks, but none came. we switched to TX and now need to resend data
            if ((macState == SEND_DATA) && (radioState== RadioState::TRANSMIT))
            {
            	if (resend_data->isScheduled())
                  scheduleAt(simTime(), resend_data);
            }

        break;

        /*case NF_CHANNELS_SUPPORTED_CHANGED:
            ppib.phyChannelsSupported = check_and_cast<Ieee802154RadioState *>(details)->getPhyChannelsSupported();
            break;

        case NF_TRANSMIT_POWER_CHANGED:
            ppib.phyTransmitPower = check_and_cast<Ieee802154RadioState *>(details)->getPhyTransmitPower();
            break;

        case NF_CCA_MODE_CHANGED:
            ppib.phyCCAMode = check_and_cast<Ieee802154RadioState *>(details)->getPhyCCAMode();
            break;*/

    default:
        break;
    }
}

void BMacLayer::PLME_SET_TRX_STATE_request(PHYenum state)
{
    EV << "[MAC]: sending PLME_SET_TRX_STATE_request <" << state <<"> to PHY layer" << endl;
    // construct PLME_SET_TRX_STATE_request primitive
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_SET_TRX_STATE_REQUEST);
    primitive->setStatus(state);
    sendDown(primitive);
 }

cPacket *BMacLayer::decapsMsg(BmacPkt * macPkt)
{
    cPacket * msg = macPkt->decapsulate();
    if (useIeee802Ctrl)
    {
        Ieee802Ctrl* cinfo = new Ieee802Ctrl();
        MACAddress destination = Uint64ToMac (macPkt->getSrcAddr());
        cinfo->setSrc(destination);
        msg->setControlInfo(cinfo);
    }
    else
    {
        Ieee802154NetworkCtrlInfo * cinfo = new Ieee802154NetworkCtrlInfo();
        cinfo->setNetwAddr(macPkt->getSrcAddr());
        msg->setControlInfo(cinfo);
    }
    return msg;
}

void BMacLayer::reqtMsgFromQueue()
{
    if (queueModule)
    {
        // tell queue module that we've become idle
        EV << "[MAC]: requesting another frame from queue module" << endl;
        queueModule->requestPacket();
    }
}

void BMacLayer::initializeQueueModule()
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

