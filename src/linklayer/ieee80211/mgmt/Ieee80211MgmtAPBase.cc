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
#include <string.h>

#ifdef WITH_ETHERNET
#include "EtherFrame_m.h"
#endif

void Ieee80211MgmtAPBase::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
#ifdef WITH_DHCP
    // JcM fix: Check if really the module connected in upperLayerOut is a relay unit
    // or a network layer. This is important to encap/decap the packet correctly in the Mgmt module
    if (stage==0)
    {
        if (gate("upperLayerOut")->getPathEndGate()->isConnected() &&
                (strcmp(gate("upperLayerOut")->getPathEndGate()->getOwnerModule()->getName(),"relayUnit")==0 || par("forceRelayUnit").boolValue()))
        {
            hasRelayUnit = true;
        }
        else
        {
            hasRelayUnit = false;
        }
        convertToEtherFrameFlag = par("convertToEtherFrame").boolValue();
        WATCH(hasRelayUnit);
    }
#else
    if (stage==0)
    {
        hasRelayUnit = gate("upperLayerOut")->getPathEndGate()->isConnected();
        convertToEtherFrameFlag = par("convertToEtherFrame").boolValue();
        WATCH(hasRelayUnit);
    }
#endif
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

void Ieee80211MgmtAPBase::sendToUpperLayer(Ieee80211DataFrame *frame)
{
    cPacket *outFrame = frame;
    if (convertToEtherFrameFlag)
        outFrame = (cPacket *) convertToEtherFrame(frame);
    send(outFrame, "upperLayerOut");
}

EtherFrame *Ieee80211MgmtAPBase::convertToEtherFrame(Ieee80211DataFrame *frame_)
{
    Ieee80211DataFrameWithSNAP *frame = check_and_cast<Ieee80211DataFrameWithSNAP *>(frame_);

#ifdef WITH_ETHERNET
    // create a matching ethernet frame
    EthernetIIFrame *ethframe = new EthernetIIFrame(frame->getName()); //TODO option to use EtherFrameWithSNAP instead
    ethframe->setDest(frame->getAddress3());
    ethframe->setSrc(frame->getTransmitterAddress());
    ethframe->setEtherType(frame->getEtherType());

    // encapsulate the payload in there
    cPacket *payload = frame->decapsulate();
    delete frame;
    ASSERT(payload!=NULL);
    ethframe->encapsulate(payload);
    if (ethframe->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        ethframe->setByteLength(MIN_ETHERNET_FRAME_BYTES);

    // done
    return ethframe;
#else
    throw cRuntimeError("INET compiled without ETHERNET feature!");
#endif
}

Ieee80211DataFrame *Ieee80211MgmtAPBase::convertFromEtherFrame(EtherFrame *ethframe)
{
#ifdef WITH_ETHERNET
    // create new frame
    Ieee80211DataFrameWithSNAP *frame = new Ieee80211DataFrameWithSNAP(ethframe->getName());
    frame->setFromDS(true);

    // copy addresses from ethernet frame (transmitter addr will be set to our addr by MAC)
    frame->setReceiverAddress(ethframe->getDest());
    frame->setAddress3(ethframe->getSrc());

    // copy EtherType from original frame
    if (dynamic_cast<EthernetIIFrame *>(ethframe))
        frame->setEtherType(((EthernetIIFrame *)ethframe)->getEtherType());
    else if (dynamic_cast<EtherFrameWithSNAP *>(ethframe))
        frame->setEtherType(((EtherFrameWithSNAP *)ethframe)->getLocalcode());
    else
        error("Unaccepted EtherFrame type: %s, contains no EtherType", ethframe->getClassName());

    // encapsulate payload
    cPacket *payload = ethframe->decapsulate();
    if (!payload)
        error("received empty EtherFrame from upper layer");
    frame->encapsulate(payload);
    delete ethframe;

    // done
    return frame;
#else
    throw cRuntimeError("INET compiled without ETHERNET feature!");
#endif
}

