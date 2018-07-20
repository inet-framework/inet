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

#ifndef __INET_PRIMITIVEFRAMESEQUENCES_H
#define __INET_PRIMITIVEFRAMESEQUENCES_H

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"

namespace inet {
namespace ieee80211 {

class INET_API SelfCtsFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return "CTS + self"; }
};

class INET_API DataFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;
        int ackPolicy = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return "DATA"; }
};

class INET_API ManagementAckFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return "MANAGEMENT"; }
};

class INET_API ManagementFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return "MANAGEMENT"; }
};

class INET_API AckFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return "ACK"; }
};

class INET_API RtsCtsFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return std::string("RTS") + (step == 2 ? " CTS" : ""); } // TODO: completeStep = true?
};

class INET_API RtsFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return "RTS"; } // TODO: completeStep = true?
};


class INET_API CtsFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return "CTS"; } // TODO: completeStep = true?
};


class INET_API FragFrameAckFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return std::string("FRAG-FRAME") + (step == 2 ? " ACK" : ""); } // TODO: completeStep = true?

};

class INET_API LastFrameAckFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return std::string("LAST-FRAME") + (step == 2 ? " ACK" : ""); } // TODO: completeStep = true?
};

class INET_API BlockAckReqBlockAckFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;

    public:
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override { return std::string("BLOCKACKREQ") + (step == 2 ? " BLOCKACK" : ""); } // TODO: completeStep = true?
};

} // namespace ieee80211
} // namespace inet

#endif
