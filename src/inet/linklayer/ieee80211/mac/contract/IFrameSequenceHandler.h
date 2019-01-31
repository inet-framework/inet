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

#ifndef __INET_IFRAMESEQUENCEHANDLER_H
#define __INET_IFRAMESEQUENCEHANDLER_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"

namespace inet {
namespace ieee80211 {

class FrameSequenceContext;

class INET_API IFrameSequenceHandler
{
    public:
        static simsignal_t frameSequenceStartedSignal;
        static simsignal_t frameSequenceFinishedSignal;

    public:
        class INET_API ICallback
        {
            public:
                virtual ~ICallback() { }

                virtual void transmitFrame(Packet *packet, simtime_t ifs) = 0;

                virtual void originatorProcessRtsProtectionFailed(Packet *packet) = 0;
                virtual void originatorProcessTransmittedFrame(Packet *packet) = 0;
                virtual void originatorProcessReceivedFrame(Packet *packet, Packet *lastTransmittedFrame) = 0;
                virtual void originatorProcessFailedFrame(Packet *packet) = 0;
                virtual void frameSequenceFinished() = 0;
                virtual void scheduleStartRxTimer(simtime_t timeout) = 0;
        };

    public:
        virtual ~IFrameSequenceHandler() { }

        virtual const FrameSequenceContext *getContext() const = 0;
        virtual const IFrameSequence *getFrameSequence() const = 0;
        virtual void startFrameSequence(IFrameSequence *frameSequence, FrameSequenceContext *context, ICallback *callback) = 0;
        virtual void processResponse(Packet *frame) = 0;
        virtual void transmissionComplete() = 0;
        virtual bool isSequenceRunning() = 0;
        virtual void handleStartRxTimeout() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif // ifndef __INET_IFRAMESEQUENCEHANDLER_H
