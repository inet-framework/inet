//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#include "UpperMacContext.h"
#include "ITxCallback.h"
#include "IImmediateTx.h"
#include "IContentionTx.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {
namespace ieee80211 {

UpperMacContext::UpperMacContext(const MACAddress& address,
        const IIeee80211Mode *dataFrameMode, const IIeee80211Mode *basicFrameMode, const IIeee80211Mode *controlFrameMode,
        int shortRetryLimit, int rtsThreshold, bool useEDCA, IImmediateTx *immediateTx, IContentionTx **contentionTx) :
    address(address),
    dataFrameMode(dataFrameMode), basicFrameMode(basicFrameMode), controlFrameMode(controlFrameMode),
    shortRetryLimit(shortRetryLimit), rtsThreshold(rtsThreshold), useEDCA(useEDCA), immediateTx(immediateTx), contentionTx(contentionTx)
{
}

const char *UpperMacContext::getName() const
{
    return "MAC parameters";
}

std::string UpperMacContext::info() const
{
    std::stringstream os;
    os << "dataBitrate: " << dataFrameMode->getDataMode()->getGrossBitrate();
    os << ", basicBitrate: " << basicFrameMode->getDataMode()->getGrossBitrate();
    return os.str().c_str();
}

const MACAddress& UpperMacContext::getAddress() const
{
    return address;
}

int UpperMacContext::getNumAccessCategories() const
{
    return useEDCA ? 4 : 1;
}

inline AccessCategory UpperMacContext::mapAC(int accessCategory) const
{
    ASSERT(accessCategory >= 0 && accessCategory < getNumAccessCategories());
    return useEDCA ? (AccessCategory)accessCategory : AC_LEGACY;
}

simtime_t UpperMacContext::getSlotTime() const
{
    return dataFrameMode->getSlotTime();
}

simtime_t UpperMacContext::getAifsTime(int accessCategory) const
{
    return dataFrameMode->getAifsTime(mapAC(accessCategory));
}

simtime_t UpperMacContext::getSifsTime() const
{
    return dataFrameMode->getSifsTime();
}

simtime_t UpperMacContext::getDifsTime() const
{
    return dataFrameMode->getDifsTime();
}

simtime_t UpperMacContext::getEifsTime(int accessCategory) const
{
    return dataFrameMode->getEifsTime(basicFrameMode, mapAC(accessCategory), LENGTH_ACK);
}

simtime_t UpperMacContext::getPifsTime() const
{
    return dataFrameMode->getPifsTime();
}

simtime_t UpperMacContext::getRifsTime() const
{
    return dataFrameMode->getRifsTime();
}

int UpperMacContext::getCwMin(int accessCategory) const
{
    return dataFrameMode->getCwMin(mapAC(accessCategory));
}

int UpperMacContext::getCwMax(int accessCategory) const
{
    return dataFrameMode->getCwMax(mapAC(accessCategory));
}

int UpperMacContext::getCwMulticast(int accessCategory) const
{
    return dataFrameMode->getCwMin(mapAC(accessCategory));  //TODO check
}

int UpperMacContext::getShortRetryLimit() const
{
    return shortRetryLimit;
}

int UpperMacContext::getRtsThreshold() const
{
    return rtsThreshold;
}

simtime_t UpperMacContext::getTxopLimit(int accessCategory) const
{
    return dataFrameMode->getTxopLimit(mapAC(accessCategory));
}

simtime_t UpperMacContext::getAckTimeout() const
{
    return basicFrameMode->getPhyRxStartDelay() + getSifsTime() + getAckDuration();
}

simtime_t UpperMacContext::getCtsTimeout() const
{
    return basicFrameMode->getPhyRxStartDelay() + getSifsTime() + getCtsDuration();
}

simtime_t UpperMacContext::getAckDuration() const
{
    return basicFrameMode->getDuration(LENGTH_ACK) + basicFrameMode->getSlotTime();
}

simtime_t UpperMacContext::getCtsDuration() const
{
    return basicFrameMode->getDuration(LENGTH_CTS) + basicFrameMode->getSlotTime();
}

Ieee80211RTSFrame *UpperMacContext::buildRtsFrame(Ieee80211DataOrMgmtFrame *frame) const
{
    Ieee80211RTSFrame *rtsFrame = new Ieee80211RTSFrame("RTS");
    rtsFrame->setTransmitterAddress(address);

    rtsFrame->setReceiverAddress(frame->getReceiverAddress());
    rtsFrame->setDuration(3 * getSifsTime() + basicFrameMode->getDuration(LENGTH_CTS)
            + dataFrameMode->getDuration(frame->getBitLength())    //TODO maybe not always with dataFrameMode
            + basicFrameMode->getDuration(LENGTH_ACK));
    return rtsFrame;
}

Ieee80211CTSFrame *UpperMacContext::buildCtsFrame(Ieee80211RTSFrame *rtsFrame) const
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("CTS");
    setBasicBitrate(rtsFrame);
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - getSifsTime() - basicFrameMode->getDuration(LENGTH_CTS));
    return frame;
}

Ieee80211ACKFrame *UpperMacContext::buildAckFrame(Ieee80211DataOrMgmtFrame *frameToAck) const
{
    Ieee80211ACKFrame *ackFrame = new Ieee80211ACKFrame("ACK");
    setBasicBitrate(ackFrame);
    ackFrame->setReceiverAddress(frameToAck->getTransmitterAddress());

    if (!frameToAck->getMoreFragments())
        ackFrame->setDuration(0);
    else
        ackFrame->setDuration(frameToAck->getDuration() - getSifsTime() - basicFrameMode->getDuration(LENGTH_ACK));
    return ackFrame;
}

double UpperMacContext::computeFrameDuration(int bits, double bitrate) const
{
    const IIeee80211Mode *modType = modeSet->getMode(bps(bitrate));
    double duration = SIMTIME_DBL(modType->getDuration(bits));
    EV_DEBUG << " duration=" << duration * 1e6 << "us(" << bits << "bits " << bitrate / 1e6 << "Mbps)" << endl;
    return duration;
}

Ieee80211Frame *UpperMacContext::setControlBitrate(Ieee80211Frame *frame) const
{
    return setBitrate(frame, controlFrameMode);
}

Ieee80211Frame *UpperMacContext::setBasicBitrate(Ieee80211Frame *frame) const
{
    return setBitrate(frame, basicFrameMode);
}

Ieee80211Frame *UpperMacContext::setDataBitrate(Ieee80211Frame *frame) const
{
    return setBitrate(frame, dataFrameMode);
}

Ieee80211Frame *UpperMacContext::setBitrate(Ieee80211Frame *frame, const IIeee80211Mode *mode) const
{
    ASSERT(frame->getControlInfo() == nullptr);
    TransmissionRequest *ctrl = new TransmissionRequest();
    ctrl->setBitrate(bps(mode->getDataMode()->getNetBitrate()));
    frame->setControlInfo(ctrl);
    return frame;
}

bool UpperMacContext::isForUs(Ieee80211Frame *frame) const
{
    return frame->getReceiverAddress() == address
            || frame->getReceiverAddress().isBroadcast()
            || frame->getReceiverAddress().isMulticast();       //TODO may need to filter for locally joined mcast groups
}

bool UpperMacContext::isMulticast(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress().isMulticast();
}

bool UpperMacContext::isBroadcast(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress().isBroadcast();
}

bool UpperMacContext::isCts(Ieee80211Frame *frame) const
{
    return dynamic_cast<Ieee80211CTSFrame *>(frame);
}

bool UpperMacContext::isAck(Ieee80211Frame *frame) const
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame);
}

void UpperMacContext::transmitContentionFrame(int txIndex, Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ITxCallback *completionCallback) const
{
    //TODO assert txIndex < N
    contentionTx[txIndex]->transmitContentionFrame(frame, ifs, eifs, cwMin, cwMax, slotTime, retryCount, completionCallback);
}

void UpperMacContext::transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ITxCallback *completionCallback) const
{
    immediateTx->transmitImmediateFrame(frame, ifs, completionCallback);
}

} // namespace ieee80211
} // namespace inet

