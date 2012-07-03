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
    if (stage == 0)
    {
        isConnectedToHL = gate("upperLayerOut")->getPathEndGate()->isConnected();
        const char * encDec = par("encapDecap").stringValue();
        if (!strcmp(encDec, "true"))
            encapDecap = ENCAP_DECAP_TRUE;
        else if (!strcmp(encDec, "false"))
            encapDecap = ENCAP_DECAP_FALSE;
        else if (!strcmp(encDec, "eth"))
            encapDecap = ENCAP_DECAP_ETH;
        else
            throw cRuntimeError("Unknown encapDecap parameter value: '%s'! Must be 'true','false' or 'eth'.", encDec);

        WATCH(isConnectedToHL);
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

void Ieee80211MgmtAPBase::sendToUpperLayer(Ieee80211DataFrame *frame)
{
    if (!isConnectedToHL)
    {
        delete frame;
        return;
    }
    cPacket *outFrame = NULL;
    switch (encapDecap)
    {
        case ENCAP_DECAP_ETH:
#ifdef WITH_ETHERNET
            outFrame = convertToEtherFrame(frame);
#else
            throw cRuntimeError("INET compiled without ETHERNET feature, but the 'encapDecap' parameter is set to 'eth'!");
#endif
            break;
        case ENCAP_DECAP_TRUE:
            {
                cPacket* payload = frame->decapsulate();
                Ieee802Ctrl *ctrl = new Ieee802Ctrl();
                ctrl->setSrc(frame->getTransmitterAddress());
                ctrl->setDest(frame->getAddress3());
                Ieee80211DataFrameWithSNAP *frameWithSNAP = dynamic_cast<Ieee80211DataFrameWithSNAP *>(frame);
                if (frameWithSNAP)
                    ctrl->setEtherType(frameWithSNAP->getEtherType());
                payload->setControlInfo(ctrl);
                delete frame;
                outFrame = payload;
            }
            break;
        case ENCAP_DECAP_FALSE:
            outFrame = frame;
            break;
        default:
            throw cRuntimeError("Unknown encapDecap value: %d", encapDecap);
            break;
    }
    send(outFrame, "upperLayerOut");
}

EtherFrame *Ieee80211MgmtAPBase::convertToEtherFrame(Ieee80211DataFrame *frame_)
{
#ifdef WITH_ETHERNET
    Ieee80211DataFrameWithSNAP *frame = check_and_cast<Ieee80211DataFrameWithSNAP *>(frame_);

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
        frame->setEtherType(((EthernetIIFrame *) ethframe)->getEtherType());
    else if (dynamic_cast<EtherFrameWithSNAP *>(ethframe))
        frame->setEtherType(((EtherFrameWithSNAP *) ethframe)->getLocalcode());
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

Ieee80211DataFrame *Ieee80211MgmtAPBase::encapsulate(cPacket *msg)
{
    switch (encapDecap)
    {
        case ENCAP_DECAP_ETH:
#ifdef WITH_ETHERNET
            return convertFromEtherFrame(check_and_cast<EtherFrame *>(msg));
#else
            throw cRuntimeError("INET compiled without ETHERNET feature, but the 'encapDecap' parameter is set to 'eth'!");
#endif
            break;
        case ENCAP_DECAP_TRUE:
            {
                Ieee802Ctrl* ctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
                Ieee80211DataFrameWithSNAP *frame = new Ieee80211DataFrameWithSNAP(msg->getName());
                frame->setFromDS(true);

                // copy addresses from ethernet frame (transmitter addr will be set to our addr by MAC)
                frame->setAddress3(ctrl->getSrc());
                frame->setReceiverAddress(ctrl->getDest());
                frame->setEtherType(ctrl->getEtherType());
                delete ctrl;

                // encapsulate payload
                frame->encapsulate(msg);
                return frame;
            }
            break;
        case ENCAP_DECAP_FALSE:
            return check_and_cast<Ieee80211DataFrame *>(msg);
            break;
        default:
            throw cRuntimeError("Unknown encapDecap value: %d", encapDecap);
            break;
    }
    return NULL;
}

