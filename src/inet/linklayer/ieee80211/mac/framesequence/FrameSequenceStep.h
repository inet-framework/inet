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
        Packet *frameToTransmit = nullptr;
        simtime_t ifs = -1;

    public:
        TransmitStep(Packet *frame, simtime_t ifs) :
            frameToTransmit(frame),
            ifs(ifs)
        { }
        virtual ~TransmitStep()
        {
#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
            if (!dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(frameToTransmit->peekAtFront<Ieee80211MacHeader>()))
                delete frameToTransmit;
#endif
        }

        virtual Completion getCompletion() override { return completion; }
        virtual void setCompletion(Completion completion) override { this->completion = completion; }
        virtual Packet *getFrameToTransmit() override { return frameToTransmit; }
        virtual simtime_t getIfs() override { return ifs; }
};

class INET_API RtsTransmitStep : public TransmitStep
{
    protected:
        const Packet *protectedFrame = nullptr;

    public:
        RtsTransmitStep(Packet *protectedFrame, Packet *frame, simtime_t ifs) :
            TransmitStep(frame, ifs),
            protectedFrame(protectedFrame)
        { }

        virtual const Packet *getProtectedFrame() { return protectedFrame; }
};

class INET_API ReceiveStep : public IReceiveStep
{
    protected:
        Completion completion = Completion::UNDEFINED;
        simtime_t timeout = -1;
        Packet *receivedFrame = nullptr;

    public:
        ReceiveStep(simtime_t timeout = -1) :
            timeout(timeout)
        { }
        virtual ~ReceiveStep() { delete receivedFrame; }

        virtual Completion getCompletion() override { return completion; }
        virtual void setCompletion(Completion completion) override { this->completion = completion; }
        virtual simtime_t getTimeout() override { return timeout; }
        virtual Packet *getReceivedFrame() override { return receivedFrame; }
        virtual void setFrameToReceive(Packet *frame) override { this->receivedFrame = frame; }
};

} // namespace ieee80211
} // namespace inet

#endif

