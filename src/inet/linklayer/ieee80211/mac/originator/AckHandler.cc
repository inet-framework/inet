//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/AckHandler.h"

namespace inet {
namespace ieee80211 {

Define_Module(AckHandler);

std::ostream& operator<<(std::ostream& os, const AckHandler::Status& status) { return os << AckHandler::getStatusString(status); }

void AckHandler::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        WATCH_MAP(ackStatuses);
    }
}

AckHandler::Status AckHandler::getAckStatus(SequenceControlField id)
{
    auto it = ackStatuses.find(id);
    return (it != ackStatuses.end()) ? it->second : Status::FRAME_NOT_YET_TRANSMITTED;
}

AckHandler::Status AckHandler::getAckStatus(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    auto id = SequenceControlField(header->getSequenceNumber().get(), header->getFragmentNumber());
    return getAckStatus(id);
}

void AckHandler::processReceivedAck(const Ptr<const Ieee80211AckFrame>& ack, const Ptr<const Ieee80211DataOrMgmtHeader>& ackedHeader)
{
    auto id = SequenceControlField(ackedHeader->getSequenceNumber().get(), ackedHeader->getFragmentNumber());
    auto status = getAckStatus(id);
    if (status == Status::FRAME_NOT_YET_TRANSMITTED)
        throw cRuntimeError("ackedFrame = %s is not yet transmitted", ackedHeader->getName());
    ackStatuses[id] = Status::ACK_ARRIVED;
}

void AckHandler::processTransmittedDataOrMgmtFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    auto id = SequenceControlField(header->getSequenceNumber().get(), header->getFragmentNumber());
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
        if (dataHeader->getAckPolicy() == NORMAL_ACK)
            ackStatuses[id] = Status::WAITING_FOR_ACK;
        else
            ackStatuses[id] = Status::NO_ACK_REQUIRED;
    }
    else
        ackStatuses[id] = Status::NO_ACK_REQUIRED;
}

void AckHandler::frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    auto id = SequenceControlField(dataOrMgmtHeader->getSequenceNumber().get(), dataOrMgmtHeader->getFragmentNumber());
    auto status = getAckStatus(id);
    ASSERT(status != Status::WAITING_FOR_ACK);
    ackStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
}

bool AckHandler::isEligibleToTransmit(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    auto status = getAckStatus(header);
    return status == AckHandler::Status::NO_ACK_REQUIRED ||
           status == AckHandler::Status::ACK_NOT_ARRIVED ||
           status == AckHandler::Status::FRAME_NOT_YET_TRANSMITTED;
}

bool AckHandler::isOutstandingFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    return false;
}

void AckHandler::processFailedFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader)) {
        if (dataHeader->getAckPolicy() == NORMAL_ACK) {
            ASSERT(getAckStatus(dataOrMgmtHeader) == Status::WAITING_FOR_ACK);
            auto id = SequenceControlField(dataOrMgmtHeader->getSequenceNumber().get(), dataOrMgmtHeader->getFragmentNumber());
            ackStatuses[id] = Status::ACK_NOT_ARRIVED;
        }
    }
}

void AckHandler::dropFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    auto id = SequenceControlField(dataOrMgmtHeader->getSequenceNumber().get(), dataOrMgmtHeader->getFragmentNumber());
    ackStatuses.erase(id);
}

std::string AckHandler::getStatusString(Status status)
{
    switch (status) {
        case Status::FRAME_NOT_YET_TRANSMITTED: return "FRAME_NOT_YET_TRANSMITTED";
        case Status::NO_ACK_REQUIRED: return "NO_ACK_REQUIRED";
        case Status::WAITING_FOR_ACK: return "WAITING_FOR_NORMAL_ACK";
        case Status::ACK_NOT_ARRIVED: return "NORMAL_ACK_NOT_ARRIVED";
        case Status::ACK_ARRIVED: return "NORMAL_ACK_ARRIVED";
        default: throw cRuntimeError("Unknown status");
    }
}

void AckHandler::printAckStatuses()
{
    for (auto ackStatus : ackStatuses) {
        std::cout << "Seq Num = " << ackStatus.first.getSequenceNumber() << " " << "Frag Num = " << (int)ackStatus.first.getFragmentNumber() << std::endl;
        std::cout << "Status = " << getStatusString(ackStatus.second) << std::endl;
    }
    std::cout << "=========================================" << std::endl;
}

} /* namespace ieee80211 */
} /* namespace inet */

