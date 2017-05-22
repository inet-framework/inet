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

#ifndef __INET_FRAMESEQUENCESTEP_H
#define __INET_FRAMESEQUENCESTEP_H

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"

namespace inet {
namespace ieee80211 {

class INET_API TransmitStep : public ITransmitStep
{
    protected:
        Completion completion = Completion::UNDEFINED;
        Ieee80211Frame *frameToTransmit = nullptr;
        simtime_t ifs = -1;

    public:
        TransmitStep(Ieee80211Frame *frame, simtime_t ifs) :
            frameToTransmit(frame),
            ifs(ifs)
        { }
        virtual ~TransmitStep() { if (!dynamic_cast<Ieee80211DataOrMgmtFrame *>(frameToTransmit)) delete frameToTransmit; }

        virtual Completion getCompletion() override { return completion; }
        virtual void setCompletion(Completion completion) override { this->completion = completion; }
        virtual Ieee80211Frame *getFrameToTransmit() override { return frameToTransmit; }
        virtual simtime_t getIfs() override { return ifs; }
};

class INET_API RtsTransmitStep : public TransmitStep
{
    protected:
        Ieee80211DataOrMgmtFrame *protectedFrame = nullptr;

    public:
        RtsTransmitStep(Ieee80211DataOrMgmtFrame *protectedFrame, Ieee80211Frame *frame, simtime_t ifs) :
            TransmitStep(frame, ifs),
            protectedFrame(protectedFrame)
        { }

        virtual Ieee80211DataOrMgmtFrame *getProtectedFrame() { return protectedFrame; }
};

class INET_API ReceiveStep : public IReceiveStep
{
    protected:
        Completion completion = Completion::UNDEFINED;
        simtime_t timeout = -1;
        Ieee80211Frame *receivedFrame = nullptr;

    public:
        ReceiveStep(simtime_t timeout = -1) :
            timeout(timeout)
        { }
        virtual ~ReceiveStep() { delete receivedFrame; }

        virtual Completion getCompletion() override { return completion; }
        virtual void setCompletion(Completion completion) override { this->completion = completion; }
        virtual simtime_t getTimeout() override { return timeout; }
        virtual Ieee80211Frame *getReceivedFrame() override { return receivedFrame; }
        virtual void setFrameToReceive(Ieee80211Frame *frame) override { this->receivedFrame = frame; }
};

} // namespace ieee80211
} // namespace inet

#endif

