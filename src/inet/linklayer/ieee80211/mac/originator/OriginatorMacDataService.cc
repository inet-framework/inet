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

#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"
#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/NonQoSSequenceNumberAssignment.h"
#include "OriginatorMacDataService.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorMacDataService);

void OriginatorMacDataService::initialize()
{
    sequenceNumberAssigment = new NonQoSSequenceNumberAssignment();
    fragmentationPolicy = check_and_cast<IFragmentationPolicy*>(getSubmodule("fragmentationPolicy"));
    fragmentation = new Fragmentation();
}

Ieee80211DataOrMgmtFrame* OriginatorMacDataService::assignSequenceNumber(Ieee80211DataOrMgmtFrame* frame)
{
    sequenceNumberAssigment->assignSequenceNumber(frame);
    return frame;
}

OriginatorMacDataService::Fragments *OriginatorMacDataService::fragmentIfNeeded(Ieee80211DataOrMgmtFrame *frame)
{
    auto fragmentSizes = fragmentationPolicy->computeFragmentSizes(frame);
    if (fragmentSizes.size() != 0) {
        auto fragmentFrames = fragmentation->fragmentFrame(frame, fragmentSizes);
        return fragmentFrames;
    }
    return nullptr;
}

OriginatorMacDataService::Fragments* OriginatorMacDataService::extractFramesToTransmit(PendingQueue *pendingQueue)
{
    if (pendingQueue->isEmpty())
        return nullptr;
    else {
        // if (msduRateLimiting)
        //    txRateLimitingIfNeeded();
        Ieee80211DataOrMgmtFrame *frame = pendingQueue->pop();
        if (sequenceNumberAssigment)
            frame = assignSequenceNumber(frame);
        // if (msduIntegrityAndProtection)
        //    frame = protectMsduIfNeeded(frame);
        Fragments *fragments = nullptr;
        if (fragmentationPolicy)
            fragments = fragmentIfNeeded(frame);
        if (!fragments)
            fragments = new Fragments({frame});
        // if (mpduEncryptionAndIntegrity)
        //    fragments = encryptMpduIfNeeded(fragments);
        // if (mpduHeaderPlusCrc)
        //    fragments = mpduCrcFooBarIfNeeded(fragments);
        return fragments;
    }
}

OriginatorMacDataService::~OriginatorMacDataService()
{
    delete sequenceNumberAssigment;
    delete fragmentation;
}


} /* namespace ieee80211 */
} /* namespace inet */
