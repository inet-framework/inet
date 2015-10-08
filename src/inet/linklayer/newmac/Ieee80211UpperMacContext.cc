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

#include "Ieee80211UpperMacContext.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "IIeee80211MacTx.h"

namespace inet {
namespace ieee80211 {

Ieee80211UpperMacContext::Ieee80211UpperMacContext(const MACAddress& address,
        const IIeee80211Mode *dataFrameMode, const IIeee80211Mode *basicFrameMode, const IIeee80211Mode *controlFrameMode,
        int shortRetryLimit,  int rtsThreshold, IIeee80211MacTx *tx) :
                    address(address),
                    dataFrameMode(dataFrameMode), basicFrameMode(basicFrameMode), controlFrameMode(controlFrameMode),
                    shortRetryLimit(shortRetryLimit), rtsThreshold(rtsThreshold), tx(tx)
{
}

const MACAddress& Ieee80211UpperMacContext::getAddress() const
{
    return address;
}

simtime_t Ieee80211UpperMacContext::getSlotTime() const
{
    return dataFrameMode->getSlotTime();
}

simtime_t Ieee80211UpperMacContext::getAIFS() const
{
    return dataFrameMode->getAifsTime(2); //TODO!!!
}

simtime_t Ieee80211UpperMacContext::getSIFS() const
{
    return dataFrameMode->getSifsTime();
}

simtime_t Ieee80211UpperMacContext::getDIFS() const
{
    return dataFrameMode->getDifsTime();
}

simtime_t Ieee80211UpperMacContext::getEIFS() const
{
    return dataFrameMode->getEifsTime(basicFrameMode, LENGTH_ACK);  //TODO ???
}

simtime_t Ieee80211UpperMacContext::getPIFS() const
{
    return dataFrameMode->getPifsTime();
}

simtime_t Ieee80211UpperMacContext::getRIFS() const
{
    return dataFrameMode->getRifsTime();
}

int Ieee80211UpperMacContext::getMinCW() const
{
    return dataFrameMode->getCwMin(); //TODO naming
}

int Ieee80211UpperMacContext::getMaxCW() const
{
    return dataFrameMode->getCwMax(); //TODO naming
}

int Ieee80211UpperMacContext::getShortRetryLimit() const
{
    return shortRetryLimit;
}

int Ieee80211UpperMacContext::getRtsThreshold() const
{
    return rtsThreshold;
}

simtime_t Ieee80211UpperMacContext::getAckTimeout() const
{
    return 2*MAX_PROPAGATION_DELAY + getSIFS() +  basicFrameMode->getDuration(LENGTH_ACK);
}

simtime_t Ieee80211UpperMacContext::getCtsTimeout() const
{
    return 2*MAX_PROPAGATION_DELAY + getSIFS() +  basicFrameMode->getDuration(LENGTH_CTS);
}

Ieee80211RTSFrame *Ieee80211UpperMacContext::buildRtsFrame(Ieee80211DataOrMgmtFrame *frame) const
{
    Ieee80211RTSFrame *rtsFrame = new Ieee80211RTSFrame("RTS");
    rtsFrame->setTransmitterAddress(address);
    rtsFrame->setReceiverAddress(frame->getReceiverAddress());
    rtsFrame->setDuration(3 * getSIFS() + basicFrameMode->getDuration(LENGTH_CTS) +
            dataFrameMode->getDuration(frame->getBitLength()) +  //TODO maybe not always with dataFrameMode
            basicFrameMode->getDuration(LENGTH_ACK));
    return rtsFrame;
}

Ieee80211CTSFrame *Ieee80211UpperMacContext::buildCtsFrame(Ieee80211RTSFrame *rtsFrame) const
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("CTS");
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - getSIFS() - basicFrameMode->getDuration(LENGTH_CTS));
    return frame;
}

Ieee80211ACKFrame *Ieee80211UpperMacContext::buildAckFrame(Ieee80211DataOrMgmtFrame *frameToACK) const
{
    Ieee80211ACKFrame *frame = new Ieee80211ACKFrame("ACK");
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());

    if (!frameToACK->getMoreFragments())
        frame->setDuration(0);
    else
        frame->setDuration(frameToACK->getDuration() - getSIFS() - basicFrameMode->getDuration(LENGTH_ACK));
    return frame;
}

Ieee80211DataOrMgmtFrame *Ieee80211UpperMacContext::buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend) const //FIXME completely misleading name, random functionality
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();
    frame->setDuration(0);
    return frame;
}

double Ieee80211UpperMacContext::computeFrameDuration(Ieee80211Frame *msg) const
{
    return 0; //TODO computeFrameDuration(msg->getBitLength(), basicFrameMode->getDataMode()->getNetBitrate());
}

double Ieee80211UpperMacContext::computeFrameDuration(int bits, double bitrate) const
{
    const IIeee80211Mode *modType = modeSet->getMode(bps(bitrate));
    double duration = SIMTIME_DBL(modType->getDuration(bits));
    EV_DEBUG << " duration=" << duration * 1e6 << "us(" << bits << "bits " << bitrate / 1e6 << "Mbps)" << endl;
    return duration;
}

Ieee80211Frame *Ieee80211UpperMacContext::setBasicBitrate(Ieee80211Frame *frame) const
{
    ASSERT(frame->getControlInfo() == nullptr);
    TransmissionRequest *ctrl = new TransmissionRequest();
    ctrl->setBitrate(bps(basicFrameMode->getDataMode()->getNetBitrate()));
    frame->setControlInfo(ctrl);
    return frame;
}

void Ieee80211UpperMacContext::setDataFrameDuration(Ieee80211DataOrMgmtFrame *frame) const
{
    if (isBroadcast(frame))
        frame->setDuration(0);
//TODO
//    else if (!frame->getMoreFragments())
//        frame->setDuration(getSIFS() + computeFrameDuration(LENGTH_ACK, basicFrameMode->getDataMode()->getNetBitrate()));
//    else
//        // FIXME: shouldn't we use the next frame to be sent?
//        frame->setDuration(3 * getSIFS() + 2 * computeFrameDuration(LENGTH_ACK, basicFrameMode->getDataMode()->getNetBitrate())
//                + computeFrameDuration(frame));
}

bool Ieee80211UpperMacContext::isForUs(Ieee80211Frame *frame) const
{
    return frame->getReceiverAddress() == address;
}

bool Ieee80211UpperMacContext::isBroadcast(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress().isBroadcast();
}

bool Ieee80211UpperMacContext::isCts(Ieee80211Frame *frame) const
{
    return dynamic_cast<Ieee80211CTSFrame *>(frame);
}

bool Ieee80211UpperMacContext::isAck(Ieee80211Frame *frame) const
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame);
}

void Ieee80211UpperMacContext::transmitContentionFrame(int txIndex, Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, IIeee80211MacTx::ICallback *completionCallback) const
{
    tx->transmitContentionFrame(txIndex, frame, ifs, eifs, cwMin, cwMax, slotTime, retryCount, completionCallback);
}

void Ieee80211UpperMacContext::transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, IIeee80211MacTx::ICallback *completionCallback) const
{
    tx->transmitImmediateFrame(frame, ifs, completionCallback);
}


}
} /* namespace inet */

