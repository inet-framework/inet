//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/ieee80211/mac/aggregation/MsduDeaggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MsduDeaggregation);

void MsduDeaggregation::setExplodedFrameAddress(Ieee80211DataFrame* frame, Ieee80211MsduSubframe* subframe, Ieee80211DataFrame *aMsduFrame)
{
    bool toDS = aMsduFrame->getToDS();
    bool fromDS = aMsduFrame->getFromDS();
    if (fromDS == 0 && toDS == 0) // STA to STA
    {
        frame->setTransmitterAddress(aMsduFrame->getTransmitterAddress());
        frame->setReceiverAddress(aMsduFrame->getReceiverAddress());
    }
    else if (fromDS == 1 && toDS == 0) // AP to STA
    {
        frame->setTransmitterAddress(frame->getTransmitterAddress());
        frame->setReceiverAddress(subframe->getDa());
        frame->setAddress3(subframe->getSa());
    }
    else if (fromDS == 0 && toDS == 1) // STA to AP
    {
        frame->setTransmitterAddress(subframe->getSa());
        frame->setReceiverAddress(aMsduFrame->getReceiverAddress());
        frame->setAddress3(subframe->getDa());
    }
    else if (fromDS == 1 && toDS == 1) // AP to AP
    {
        frame->setReceiverAddress(aMsduFrame->getReceiverAddress());
        frame->setTransmitterAddress(aMsduFrame->getTransmitterAddress());
        frame->setAddress3(subframe->getDa());
        frame->setAddress4(subframe->getSa());
    }
}

std::vector<Ieee80211DataFrame*> *MsduDeaggregation::deaggregateFrame(Ieee80211DataFrame* frame)
{
    std::vector<Ieee80211DataFrame *> *frames = new std::vector<Ieee80211DataFrame *>();
    Ieee80211AMsdu *aMsdu = check_and_cast<Ieee80211AMsdu *>(frame->getEncapsulatedPacket());
    int tid = frame->getTid();
    int numOfSubframes = aMsdu->getSubframesArraySize();
    for (int i = 0; i < numOfSubframes; i++)
    {
        Ieee80211MsduSubframe msduSubframe = aMsdu->getSubframes(i);
        cPacket *msdu = msduSubframe.decapsulate();
        Ieee80211DataFrame *dataFrame = nullptr;
        // TODO: review, restore snap header, see Ieee80211MsduSubframe
        dataFrame = (msduSubframe.getEtherType() != -1) ? new Ieee80211DataFrameWithSNAP(msduSubframe.getName()) : new Ieee80211DataFrame(msduSubframe.getName());
        dataFrame->setType(ST_DATA_WITH_QOS);
        dataFrame->addBitLength(QOSCONTROL_BITS);
        dataFrame->setTransmitterAddress(msduSubframe.getSa());
        dataFrame->setToDS(frame->getToDS());
        dataFrame->setFromDS(frame->getFromDS());
        dataFrame->setTid(tid);
        // TODO: review, restore snap header, see Ieee80211MsduSubframe
        if (auto dataFrameWithSnap = dynamic_cast<Ieee80211DataFrameWithSNAP*>(dataFrame))
            dataFrameWithSnap->setEtherType(msduSubframe.getEtherType());
        dataFrame->encapsulate(msdu);
        setExplodedFrameAddress(dataFrame, &msduSubframe, frame);
        frames->push_back(dataFrame);
    }
    delete frame;
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

