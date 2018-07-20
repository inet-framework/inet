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

#include "AckHandler.h"

namespace inet {
namespace ieee80211 {

AckHandler::Status& AckHandler::getAckStatus(SequenceControlField id)
{
    auto it = ackStatuses.find(id);
    if (it == ackStatuses.end()) {
        ackStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
        return ackStatuses[id];
    }
    return it->second;
}

AckHandler::Status AckHandler::getAckStatus(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    auto id = SequenceControlField(header->getSequenceNumber(), header->getFragmentNumber());
    auto it = ackStatuses.find(id);
    if (it == ackStatuses.end()) {
        ackStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
        return Status::FRAME_NOT_YET_TRANSMITTED;
    }
    return it->second;
}

void AckHandler::processReceivedAck(const Ptr<const Ieee80211AckFrame>& ack, const Ptr<const Ieee80211DataOrMgmtHeader>& ackedHeader)
{
    auto id = SequenceControlField(ackedHeader->getSequenceNumber(), ackedHeader->getFragmentNumber());
    Status &status = getAckStatus(id);
    if (status == Status::FRAME_NOT_YET_TRANSMITTED)
        throw cRuntimeError("ackedFrame = %s is not yet transmitted", ackedHeader->getName());
    status = Status::ACK_ARRIVED;
}

void AckHandler::processTransmittedDataOrMgmtFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    // TODO: mgmt with NoAck subfield?
    auto id = SequenceControlField(header->getSequenceNumber(), header->getFragmentNumber());
    ackStatuses[id] = Status::WAITING_FOR_ACK;
}

void AckHandler::frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    auto id = SequenceControlField(dataOrMgmtHeader->getSequenceNumber(), dataOrMgmtHeader->getFragmentNumber());
    Status& status = getAckStatus(id);
    ASSERT(status != Status::WAITING_FOR_ACK);
    ackStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
}

bool AckHandler::isEligibleToTransmit(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    auto status = getAckStatus(header);
    return status == AckHandler::Status::ACK_NOT_ARRIVED || status == AckHandler::Status::FRAME_NOT_YET_TRANSMITTED;
}

bool AckHandler::isOutstandingFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    return false;
}

int AckHandler::getNumberOfFramesWithStatus(Status status)
{
    int count = 0;
    for (auto ackStatus : ackStatuses)
        if (ackStatus.second == status)
            count++;
    return count;
}


void AckHandler::processFailedFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    ASSERT(getAckStatus(dataOrMgmtHeader) == Status::WAITING_FOR_ACK);
    auto id = SequenceControlField(dataOrMgmtHeader->getSequenceNumber(), dataOrMgmtHeader->getFragmentNumber());
    ackStatuses[id] = Status::ACK_NOT_ARRIVED;
}

std::string AckHandler::getStatusString(Status status)
{
    switch (status) {
        case Status::FRAME_NOT_YET_TRANSMITTED : return "FRAME_NOT_YET_TRANSMITTED";
        case Status::NO_ACK_REQUIRED : return "NO_ACK_REQUIRED";
        case Status::WAITING_FOR_ACK : return "WAITING_FOR_NORMAL_ACK";
        case Status::ACK_NOT_ARRIVED : return "NORMAL_ACK_NOT_ARRIVED";
        case Status::ACK_ARRIVED  : return "NORMAL_ACK_ARRIVED";
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
