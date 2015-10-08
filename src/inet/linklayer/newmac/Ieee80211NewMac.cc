//
// Copyright (C) 2006 Andras Varga and Levente M�sz�ros
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

#include <algorithm>

#include "Ieee80211NewMac.h"

#include "inet_old/util/opp_utils.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "Ieee80211UpperMac.h"
#include "Ieee80211MacReception.h"
#include "inet_old/linklayer/contract/PhyControlInfo_m.h"
#include "Ieee80211MacTransmission.h"

namespace inet {

Define_Module(Ieee80211NewMac);


// don't forget to keep synchronized the C++ enum and the runtime enum definition
//Register_Enum(RadioState,
//   (RadioState::IDLE,
//    RadioState::RECV,
//    RadioState::TRANSMIT,
//    RadioState::SLEEP));

simsignal_t Ieee80211NewMac::stateSignal = SIMSIGNAL_NULL;
simsignal_t Ieee80211NewMac::radioStateSignal = SIMSIGNAL_NULL;

/****************************************************************
 * Construction functions.
 */
Ieee80211NewMac::Ieee80211NewMac()
{
    pendingRadioConfigMsg = nullptr;
}

Ieee80211NewMac::~Ieee80211NewMac()
{
    if (pendingRadioConfigMsg)
        delete pendingRadioConfigMsg;

}

/****************************************************************
 * Initialization functions.
 */
void Ieee80211NewMac::initialize(int stage)
{
    WirelessMacBase::initialize(stage);

    if (stage == 0)
    {
        EV << "Initializing stage 0\n";

        // upper mac
        upperMac = new Ieee80211UpperMac(this);
        reception = new Ieee80211MacReception(this);
        transmission = new Ieee80211MacTransmission(this);
        endImmediateIFS = new cMessage("Immediate IFS");
        immediateFrameDuration = new cMessage("Immediate Frame Duration");
        // initialize parameters
        bitrate = par("bitrate");
        basicBitrate = par("basicBitrate");
        rtsThreshold = par("rtsThresholdBytes");

        // the variable is renamed due to a confusion in the standard
        // the name retry limit would be misleading, see the header file comment
        transmissionLimit = par("retryLimit");
        if (transmissionLimit == -1) transmissionLimit = 7;
        ASSERT(transmissionLimit > 0);

        const char *addressString = par("address");
        if (!strcmp(addressString, "auto")) {
            // assign automatic address
            address = MACAddress::generateAutoAddress();
            // change module parameter from "auto" to concrete address
            par("address").setStringValue(address.str().c_str());
        }
        else
            address.setAddress(addressString);

        // subscribe for the information of the carrier sense
        nb->subscribe(this, NF_RADIOSTATE_CHANGED);

        // initalize self messages
        stateSignal = registerSignal("state");
    //        stateSignal.setEnum("Ieee80211NewMac");
        radioStateSignal = registerSignal("radioState");
    //        radioStateSignal.setEnum("RadioState");
        // interface
        registerInterface();

        // statistics
        numRetry = 0;
        numSentWithoutRetry = 0;
        numGivenUp = 0;
        numCollision = 0;
        numSent = 0;
        numReceived = 0;
        numSentBroadcast = 0;
        numReceivedBroadcast = 0;

        WATCH(numRetry);
        WATCH(numSentWithoutRetry);
        WATCH(numGivenUp);
        WATCH(numCollision);
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numSentBroadcast);
        WATCH(numReceivedBroadcast);
    }
}

void Ieee80211NewMac::registerInterface()
{
    IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (!ift)
        return;

    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    e->setName(OPP_Global::stripnonalnum(getParentModule()->getFullName()).c_str());

    // address
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // FIXME: MTU on 802.11 = ?
    e->setMtu(par("mtu"));

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    ift->addInterface(e);
}


/****************************************************************
 * Message handling functions.
 */
void Ieee80211NewMac::handleSelfMsg(cMessage *msg)
{
    EV << "received self message: " << msg << endl;
    if (msg->getContextPointer() != nullptr)
        ((Ieee80211MacPlugin *)msg->getContextPointer())->handleMessage(msg);
    else if (msg == endImmediateIFS)
    { // TODO!!!
        scheduleAt(simTime() + immediateFrame->getBitLength() / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER, immediateFrameDuration);
        sendDown(immediateFrame);
    }
    else if (msg == immediateFrameDuration)
        upperMac->transmissionFinished();
}

void Ieee80211NewMac::handleUpperMsg(cPacket *msg)
{
    upperMac->upperFrameReceived(check_and_cast<Ieee80211DataOrMgmtFrame*>(msg));
}

void Ieee80211NewMac::handleLowerMsg(cPacket *msg)
{
    // TODO:
    Ieee80211Frame *frame = check_and_cast<Ieee80211Frame *>(msg);
    if (isForUs(frame))
        reception->handleLowerFrame(frame);
    else
        delete frame;
}

void Ieee80211NewMac::handleCommand(cMessage *msg)
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

        if (reception->isMediumFree()) // TODO
        {
            EV << "Sending it down immediately\n";
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
        error("Unrecognized command from mgmt layer: (%s)%s msgkind=%d",
                msg->getClassName(), msg->getName(), msg->getKind());
    }
}


void Ieee80211NewMac::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category == NF_RADIOSTATE_CHANGED)
    {
        RadioState::State newRadioState = check_and_cast<RadioState *>(details)->getState();

        emit(radioStateSignal, newRadioState);
        reception->radioStateChanged(newRadioState);
        transmission->mediumStateChanged(reception->isMediumFree());
    }
}

void Ieee80211NewMac::sendDataFrame(Ieee80211Frame *frameToSend)
{
    EV << "sending Data frame\n";
    sendDown(frameToSend->dup());
}

void Ieee80211NewMac::sendDownPendingRadioConfigMsg()
{
    if (pendingRadioConfigMsg != NULL)
    {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = NULL;
    }
}

void Ieee80211NewMac::transmitImmediateFrame(Ieee80211Frame* frame, simtime_t deferDuration)
{
    scheduleAt(simTime() + deferDuration, endImmediateIFS);
    immediateFrame = frame;
}

simtime_t Ieee80211NewMac::getSlotTime() const
{
// TODO:   return aCCATime() + aRxTxTurnaroundTime + aAirPropagationTime() + aMACProcessingDelay();
    return ST;
}

bool Ieee80211NewMac::isForUs(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress() == address;
}

} // namespace inet

