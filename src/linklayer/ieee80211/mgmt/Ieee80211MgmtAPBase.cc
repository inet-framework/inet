//
// Copyright (C) 2006 Andras Varga
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


#include "Ieee80211MgmtAPBase.h"
#include "Ieee802Ctrl_m.h"
#include "EtherFrame_m.h"


void Ieee80211MgmtAPBase::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);

    if (stage==0)
    {
        hasRelayUnit = gate("uppergateOut")->getPathEndGate()->isConnected();
        WATCH(hasRelayUnit);
    }
}

void Ieee80211MgmtAPBase::distributeReceivedDataFrame(Ieee80211DataFrame *frame)
{
    // adjust toDS/fromDS bits, and shuffle addresses
    frame->setToDS(false);
    frame->setFromDS(true);

    // move destination address to address1 (receiver address),
    // and fill address3 with original source address;
    // sender address (address2) will be filled in by MAC
    ASSERT(!frame->getAddress3().isUnspecified());
    frame->setReceiverAddress(frame->getAddress3());
    frame->setAddress3(frame->getTransmitterAddress());

    sendOrEnqueue(frame);
}

EtherFrame *Ieee80211MgmtAPBase::convertToEtherFrame(Ieee80211DataFrame *frame)
{
    // create a matching ethernet frame
    EtherFrame *ethframe = new EthernetIIFrame(frame->getName()); //TODO option to use EtherFrameWithSNAP instead
    ethframe->setDest(frame->getAddress3());
    ethframe->setSrc(frame->getTransmitterAddress());
    //XXX set ethertype

    // encapsulate the payload in there
    cPacket *payload = frame->decapsulate();
    delete frame;
    ASSERT(payload!=NULL);
    ethframe->encapsulate(payload);

    // done
    return ethframe;
}

Ieee80211DataFrame *Ieee80211MgmtAPBase::convertFromEtherFrame(EtherFrame *ethframe)
{
    // create new frame
    Ieee80211DataFrame *frame = new Ieee80211DataFrame(ethframe->getName());
    frame->setFromDS(true);

    // copy addresses from ethernet frame (transmitter addr will be set to our addr by MAC)
    frame->setReceiverAddress(ethframe->getDest());
    frame->setAddress3(ethframe->getSrc());

    // encapsulate payload
    cPacket *payload = ethframe->decapsulate();
    if (!payload)
        error("received empty EtherFrame from upper layer");
    frame->encapsulate(payload);
    delete ethframe;

    // done
    return frame;
}

