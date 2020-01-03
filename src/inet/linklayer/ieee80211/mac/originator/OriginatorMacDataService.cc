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
#include "inet/linklayer/ieee80211/mac/originator/OriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/NonQoSSequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorMacDataService);

void OriginatorMacDataService::initialize()
{
    sequenceNumberAssigment = new NonQoSSequenceNumberAssignment();
    fragmentationPolicy = check_and_cast<IFragmentationPolicy*>(getSubmodule("fragmentationPolicy"));
    fragmentation = new Fragmentation();
}

void OriginatorMacDataService::assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header)
{
    sequenceNumberAssigment->assignSequenceNumber(header);
}

std::vector<Packet *> *OriginatorMacDataService::fragmentIfNeeded(Packet *frame)
{
    auto fragmentSizes = fragmentationPolicy->computeFragmentSizes(frame);
    if (fragmentSizes.size() != 0) {
        emit(packetFragmentedSignal, frame);
        auto fragmentFrames = fragmentation->fragmentFrame(frame, fragmentSizes);
        return fragmentFrames;
    }
    return nullptr;
}

std::vector<Packet *> *OriginatorMacDataService::extractFramesToTransmit(queueing::IPacketQueue *pendingQueue)
{
    if (pendingQueue->isEmpty())
        return nullptr;
    else {
        // if (msduRateLimiting)
        //    txRateLimitingIfNeeded();
        Packet *packet = pendingQueue->popPacket();
        take(packet);
        if (sequenceNumberAssigment) {
            auto frame = packet->removeAtFront<Ieee80211DataOrMgmtHeader>();
            assignSequenceNumber(frame);
            packet->insertAtFront(frame);
        }
        // if (msduIntegrityAndProtection)
        //    frame = protectMsduIfNeeded(frame);
        std::vector<Packet *> *fragments = nullptr;
        if (fragmentationPolicy)
            fragments = fragmentIfNeeded(packet);
        if (!fragments)
            fragments = new std::vector<Packet *>({packet});
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
