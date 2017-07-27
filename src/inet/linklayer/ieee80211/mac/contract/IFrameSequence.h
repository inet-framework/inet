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

#ifndef __INET_IFRAMESEQUENCE_H
#define __INET_IFRAMESEQUENCE_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

class FrameSequenceContext;

class INET_API IFrameSequenceStep {
    public:
        enum class Type {
            TRANSMIT,
            RECEIVE,
        };

        enum class Completion {
            UNDEFINED,
            ACCEPTED,
            REJECTED,
            EXPIRED,
        };

    public:
        virtual ~IFrameSequenceStep() { }

        virtual Type getType() = 0;
        virtual Completion getCompletion() = 0;
        virtual void setCompletion(Completion completion) = 0;
};

class INET_API ITransmitStep : public IFrameSequenceStep
{
    public:
        virtual Type getType() override { return Type::TRANSMIT; }

        virtual Packet *getFrameToTransmit() = 0;
        virtual simtime_t getIfs() = 0;
};

class INET_API IReceiveStep : public IFrameSequenceStep
{
    public:
        virtual Type getType() override { return Type::RECEIVE; }

        virtual simtime_t getTimeout() = 0;
        virtual Packet *getReceivedFrame() = 0;
        virtual void setFrameToReceive(Packet *frame) = 0;
};

class INET_API IFrameSequence
{
    public:
        virtual ~IFrameSequence() { }

        virtual void startSequence(FrameSequenceContext *context, int step) = 0;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) = 0;
        virtual bool completeStep(FrameSequenceContext *context) = 0;

        virtual std::string getHistory() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif // __INET_IFRAMESEQUENCE_H
