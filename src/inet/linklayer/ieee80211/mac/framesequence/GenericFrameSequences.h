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

#ifndef __INET_GENERICFRAMESEQUENCES_H
#define __INET_GENERICFRAMESEQUENCES_H

#include <algorithm>
#include <functional>

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"

namespace inet {
namespace ieee80211 {

#define OPTIONALFS_PREDICATE(predicate) [this](OptionalFs *frameSequence, FrameSequenceContext *context){ return predicate(frameSequence, context); }
#define REPEATINGFS_PREDICATE(predicate) [this](RepeatingFs *frameSequence, FrameSequenceContext *context){ return predicate(frameSequence, context); }
#define ALTERNATIVESFS_SELECTOR(selector) [this](AlternativesFs *frameSequence, FrameSequenceContext *context){ return selector(frameSequence, context); }

class INET_API SequentialFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;
        int elementIndex = -1;
        std::vector<IFrameSequence *> elements;

    public:
        virtual ~SequentialFs();
        SequentialFs(std::vector<IFrameSequence*> elements);

        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override;
};

class INET_API OptionalFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;
        bool apply = false;
        IFrameSequence *element;
        std::function<bool (OptionalFs *, FrameSequenceContext *)> predicate;

    public:
        virtual ~OptionalFs() { delete element; }
        OptionalFs(IFrameSequence *element, std::function<bool(OptionalFs*, FrameSequenceContext*)> predicate = nullptr);

        virtual int getStep() { return firstStep + step; }
        virtual bool isSequenceApply(FrameSequenceContext *context) { return predicate(this, context); }

        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override;
};

class INET_API RepeatingFs : public IFrameSequence {
    protected:
        int count = 0;
        int firstStep = -1;
        int step = -1;
        bool apply = false;
        IFrameSequence *element;
        std::function<bool (RepeatingFs *, FrameSequenceContext *)> predicate;
        std::vector<std::string> histories;

    public:
        virtual ~RepeatingFs() { delete element; }
        RepeatingFs(IFrameSequence *element, std::function<bool(RepeatingFs*, FrameSequenceContext*)> predicate = nullptr);

        virtual int getCount() { return count; }
        virtual int getStep() { return firstStep + step; }
        virtual bool isSequenceApply(FrameSequenceContext *context) { return predicate(this, context); }
        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual void repeatSequence(FrameSequenceContext *context);
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override;
};

class INET_API AlternativesFs : public IFrameSequence {
    protected:
        int firstStep = -1;
        int step = -1;
        int elementIndex = -1;
        std::vector<IFrameSequence *> elements;
        std::function<int (AlternativesFs *, FrameSequenceContext *)> selector;

    public:
        virtual ~AlternativesFs();
        AlternativesFs(std::vector<IFrameSequence*> elements, std::function<int(AlternativesFs*, FrameSequenceContext*)> selector);

        virtual int getStep() { return firstStep + step; }
        virtual int selectSequence(FrameSequenceContext *context) { return selector(this, context); }

        virtual void startSequence(FrameSequenceContext *context, int firstStep) override;
        virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) override;
        virtual bool completeStep(FrameSequenceContext *context) override;

        virtual std::string getHistory() const override;
};

} // namespace ieee80211
} // namespace inet

#endif

