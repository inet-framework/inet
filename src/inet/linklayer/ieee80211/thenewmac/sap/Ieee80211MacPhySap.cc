//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "Ieee80211MacPhySap.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/thenewmac/Ieee80211NewFrame_m.h"
namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacPhySap);

void Ieee80211MacPhySap::handleMessage(cMessage* msg)
{
    if (msg->getArrivalGate() == phySapTxInput)
    {
        Ieee80211NewFrame *newFrame = check_and_cast<Ieee80211NewFrame *>(msg);
        cPacket *encapPacket = newFrame->decapsulate();
        Ieee80211DataFrame *oldFrame = new Ieee80211DataFrame();
        oldFrame->encapsulate(encapPacket);
        oldFrame->setBitLength(newFrame->getBitLength());
        oldFrame->setAddress3(newFrame->getAddr3());
        oldFrame->setAddress4(newFrame->getAddr4());
        oldFrame->setReceiverAddress(newFrame->getAddr1());
        oldFrame->setDuration(tuToSimtime(newFrame->getDurId()));
        send(oldFrame, radioOut);
    }
}

void Ieee80211MacPhySap::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        phySapTxInput = gate("phySapTx$i");
        phySapRxInput = gate("phySapRx$i");
        phySapTxOutput = gate("phySapTx$o");
        phySapRxOutput = gate("phySapRx$o");
        radioIn = gate("lowerLayerIn");
        radioOut = gate("lowerLayerOut");
        radio = check_and_cast<IRadio *>(radioOut->getNextGate()->getOwnerModule());
        configureRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    }
    if (stage == INITSTAGE_LINK_LAYER_2)
    {
        emitPhyCcaIndication();
    }
}

void Ieee80211MacPhySap::emitPhyCcaIndication()
{
    Ieee80211MacSignalPhyCcaIndication *signal = new Ieee80211MacSignalPhyCcaIndication();
    signal->setStatus(CcaStatus_idle);
    cMessage *phyCcaIndication = createSignal("PhyCcaIndication", signal);
    send(phyCcaIndication, phySapRxOutput);
}

void Ieee80211MacPhySap::configureRadioMode(IRadio::RadioMode radioMode)
{
    if (radio->getRadioMode() != radioMode) {
        ConfigureRadioCommand *configureCommand = new ConfigureRadioCommand();
        configureCommand->setRadioMode(radioMode);
        cMessage *message = new cMessage("configureRadioMode", RADIO_C_CONFIGURE);
        message->setControlInfo(configureCommand);
        send(message, radioOut);
    }
}

} /* namespace inet */
} /* namespace ieee80211 */
