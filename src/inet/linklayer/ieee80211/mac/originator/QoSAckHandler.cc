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

#include "QoSAckHandler.h"

namespace inet {
namespace ieee80211 {

QoSAckHandler::Status& QoSAckHandler::getQoSDataAckStatus(const QoSKey& id)
{
    auto it = ackStatuses.find(id);
    if (it == ackStatuses.end()) {
        ackStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
        return ackStatuses[id];
    }
    return it->second;
}

QoSAckHandler::Status& QoSAckHandler::getMgmtOrNonQoSAckStatus(const Key& id)
{
    auto it = mgmtAckStatuses.find(id);
    if (it == mgmtAckStatuses.end()) {
        mgmtAckStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
        return mgmtAckStatuses[id];
    }
    return it->second;
}

QoSAckHandler::Status QoSAckHandler::getMgmtOrNonQoSAckStatus(Ieee80211DataOrMgmtFrame* frame)
{
    auto id = std::make_pair(frame->getReceiverAddress(), SequenceControlField(frame->getSequenceNumber(), frame->getFragmentNumber()));
    auto it = mgmtAckStatuses.find(id);
    if (it == mgmtAckStatuses.end()) {
        mgmtAckStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
        return Status::FRAME_NOT_YET_TRANSMITTED;
    }
    return it->second;
}


QoSAckHandler::Status QoSAckHandler::getQoSDataAckStatus(Ieee80211DataFrame* frame)
{
    auto id = std::make_pair(frame->getReceiverAddress(), std::make_pair(frame->getTid(), SequenceControlField(frame->getSequenceNumber(), frame->getFragmentNumber())));
    auto it = ackStatuses.find(id);
    if (it == ackStatuses.end()) {
        ackStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
        return Status::FRAME_NOT_YET_TRANSMITTED;
    }
    return it->second;
}

void QoSAckHandler::processReceivedAck(Ieee80211ACKFrame* ack, Ieee80211DataOrMgmtFrame *ackedFrame)
{
    if (ackedFrame->getType() == ST_DATA_WITH_QOS) {
        auto dataFrame = check_and_cast<Ieee80211DataFrame*>(ackedFrame);
        auto id = std::make_pair(dataFrame->getReceiverAddress(), std::make_pair(dataFrame->getTid(), SequenceControlField(dataFrame->getSequenceNumber(), dataFrame->getFragmentNumber())));
        Status &status = getQoSDataAckStatus(id);
        if (status == Status::FRAME_NOT_YET_TRANSMITTED)
            throw cRuntimeError("ackedFrame = %s is not yet transmitted", dataFrame->getName());
        status = Status::NORMAL_ACK_ARRIVED;
    }
    else {
        Status &status = getMgmtOrNonQoSAckStatus(std::make_pair(ackedFrame->getReceiverAddress(), SequenceControlField(ackedFrame->getSequenceNumber(), ackedFrame->getFragmentNumber())));
        if (status == Status::FRAME_NOT_YET_TRANSMITTED)
            throw cRuntimeError("ackedFrame = %s is not yet transmitted", ackedFrame->getName());
        status = Status::NORMAL_ACK_ARRIVED;
    }
}

void QoSAckHandler::processFailedFrame(Ieee80211DataOrMgmtFrame* dataOrMgmtFrame)
{
    if (dataOrMgmtFrame->getType() == ST_DATA_WITH_QOS) {
        auto dataFrame = check_and_cast<Ieee80211DataFrame*>(dataOrMgmtFrame);
        ASSERT(getQoSDataAckStatus(dataFrame) == Status::WAITING_FOR_NORMAL_ACK);
        auto id = std::make_pair(dataFrame->getReceiverAddress(), std::make_pair(dataFrame->getTid(), SequenceControlField(dataFrame->getSequenceNumber(), dataFrame->getFragmentNumber())));
        Status &status = getQoSDataAckStatus(id);
        status = Status::NORMAL_ACK_NOT_ARRIVED;
    }
    else {
        ASSERT(getMgmtOrNonQoSAckStatus(dataOrMgmtFrame) == Status::WAITING_FOR_NORMAL_ACK);
        Status &status = getMgmtOrNonQoSAckStatus(std::make_pair(dataOrMgmtFrame->getReceiverAddress(), SequenceControlField(dataOrMgmtFrame->getSequenceNumber(), dataOrMgmtFrame->getFragmentNumber())));
        status = Status::NORMAL_ACK_NOT_ARRIVED;
    }
}

std::set<std::pair<MACAddress, std::pair<Tid, SequenceControlField>>> QoSAckHandler::processReceivedBlockAck(Ieee80211BlockAck* blockAck)
{
    printAckStatuses();
    MACAddress receiverAddr = blockAck->getTransmitterAddress();
    std::set<QoSKey> ackedFrames;
    // Table 8-16—BlockAckReq frame variant encoding
    if (auto basicBlockAck = dynamic_cast<Ieee80211BasicBlockAck *>(blockAck)) {
        int startingSeqNum = basicBlockAck->getStartingSequenceNumber();
        for (int seqNum = 0; seqNum < 64; seqNum++) {
            BitVector bitmap = basicBlockAck->getBlockAckBitmap(seqNum);
            for (int fragNum = 0; fragNum < 16; fragNum++) { // TODO: declare these const values
                auto id = std::make_pair(receiverAddr, std::make_pair(basicBlockAck->getTidInfo(), SequenceControlField((startingSeqNum + seqNum) % 4096, fragNum)));
                Status& status = getQoSDataAckStatus(id);
                if (status == Status::WAITING_FOR_BLOCK_ACK) {
                    bool acked = bitmap.getBit(fragNum) == 1;
                    if (acked) ackedFrames.insert(id);
                    status = acked ? Status::BLOCK_ACK_ARRIVED_ACKED : Status::BLOCK_ACK_ARRIVED_UNACKED;
                }
                else ; // TODO: erroneous BlockAck
            }
        }
    }
    else if (auto compressedBlockAck = dynamic_cast<Ieee80211CompressedBlockAck *>(blockAck)) {
        int startingSeqNum = compressedBlockAck->getStartingSequenceNumber();
        BitVector bitmap = compressedBlockAck->getBlockAckBitmap();
        for (int seqNum = 0; seqNum < 64; seqNum++) {
            auto id = std::make_pair(receiverAddr, std::make_pair(compressedBlockAck->getTidInfo(), SequenceControlField(startingSeqNum + seqNum, 0)));
            Status& status = getQoSDataAckStatus(id);
            if (status == Status::WAITING_FOR_BLOCK_ACK) {
                bool acked = bitmap.getBit(seqNum) == 1;
                status = acked ? Status::BLOCK_ACK_ARRIVED_ACKED : Status::BLOCK_ACK_ARRIVED_UNACKED;
                if (acked) ackedFrames.insert(id);
            }
            else ; // TODO: erroneous BlockAck
        }
    }
    else {
        throw cRuntimeError("Multi-TID BlockReq is unimplemented");
    }
    printAckStatuses();
    return ackedFrames;
}

void QoSAckHandler::processTransmittedDataOrMgmtFrame(Ieee80211DataOrMgmtFrame* frame)
{
    if (frame->getType() == ST_DATA_WITH_QOS) {
        auto dataFrame = check_and_cast<Ieee80211DataFrame*>(frame);
        auto id = std::make_pair(dataFrame->getReceiverAddress(), std::make_pair(dataFrame->getTid(), SequenceControlField(dataFrame->getSequenceNumber(), dataFrame->getFragmentNumber())));
        switch (dataFrame->getAckPolicy()) {
            case NORMAL_ACK : ackStatuses[id] = Status::WAITING_FOR_NORMAL_ACK; break;
            case BLOCK_ACK : ackStatuses[id] = Status::BLOCK_ACK_NOT_YET_REQUESTED; break;
            case NO_ACK : ackStatuses[id] = Status::NO_ACK_REQUIRED; break;
            case NO_EXPLICIT_ACK : throw cRuntimeError("Unimplemented"); /* TODO: ACKED by default? */ break;
            default: throw cRuntimeError("Unknown Ack Policy = %d", dataFrame->getAckPolicy());
        }
    }
    else
        mgmtAckStatuses[std::make_pair(frame->getReceiverAddress(), SequenceControlField(frame->getSequenceNumber(), frame->getFragmentNumber()))] = Status::WAITING_FOR_NORMAL_ACK;
}

void QoSAckHandler::processTransmittedBlockAckReq(Ieee80211BlockAckReq* blockAckReq)
{
    //printAckStatuses();
    for (auto &ackStatus : ackStatuses) {
        auto tid = ackStatus.first.second.first;
        auto seqCtrlField = ackStatus.first.second.second;
        auto &status = ackStatus.second;
        if (auto basicBlockAckReq = dynamic_cast<Ieee80211BasicBlockAckReq *>(blockAckReq)) {
            if (basicBlockAckReq->getTidInfo() == tid) {
                int startingSeqNum = basicBlockAckReq->getStartingSequenceNumber();
                if (status == Status::BLOCK_ACK_NOT_YET_REQUESTED && seqCtrlField.getSequenceNumber() >= startingSeqNum)
                    status = Status::WAITING_FOR_BLOCK_ACK;
            }
        }
        else if (auto compressedBlockAckReq = dynamic_cast<Ieee80211CompressedBlockAckReq *>(blockAckReq)) {
            if (compressedBlockAckReq->getTidInfo() == tid) {
                int startingSeqNum = compressedBlockAckReq->getStartingSequenceNumber();
                if (status == Status::BLOCK_ACK_NOT_YET_REQUESTED && seqCtrlField.getSequenceNumber() >= startingSeqNum && seqCtrlField.getFragmentNumber() == 0) // TODO: ASSERT(seqCtrlField.second == 0)?
                    status = Status::WAITING_FOR_BLOCK_ACK;
            }
        }
        else
            throw cRuntimeError("Multi-TID BlockReq is unimplemented");
    }
    //printAckStatuses();
}

bool QoSAckHandler::isEligibleToTransmit(Ieee80211DataOrMgmtFrame* frame)
{
    QoSAckHandler::Status status;
    if (frame->getType() == ST_DATA_WITH_QOS) {
        auto dataFrame = check_and_cast<Ieee80211DataFrame*>(frame);
        status = getQoSDataAckStatus(dataFrame);
    }
    else
        status = getMgmtOrNonQoSAckStatus(frame);
    return status == QoSAckHandler::Status::BLOCK_ACK_ARRIVED_UNACKED || status == QoSAckHandler::Status::NORMAL_ACK_NOT_ARRIVED || status == QoSAckHandler::Status::FRAME_NOT_YET_TRANSMITTED;
}

bool QoSAckHandler::isOutstandingFrame(Ieee80211DataOrMgmtFrame* frame)
{
    if (frame->getType() == ST_DATA_WITH_QOS) {
        auto dataFrame = check_and_cast<Ieee80211DataFrame*>(frame);
        auto status = getQoSDataAckStatus(dataFrame);
        return status == QoSAckHandler::Status::BLOCK_ACK_NOT_YET_REQUESTED;
    }
    else
        return false;
}

void QoSAckHandler::frameGotInProgress(Ieee80211DataOrMgmtFrame* dataOrMgmtFrame)
{
    if (dataOrMgmtFrame->getType() == ST_DATA_WITH_QOS) {
        auto dataFrame = check_and_cast<Ieee80211DataFrame*>(dataOrMgmtFrame);
        auto id = std::make_pair(dataFrame->getReceiverAddress(), std::make_pair(dataFrame->getTid(), SequenceControlField(dataFrame->getSequenceNumber(), dataFrame->getFragmentNumber())));
        Status& status = getQoSDataAckStatus(id);
        ASSERT(status != Status::WAITING_FOR_NORMAL_ACK && status != Status::BLOCK_ACK_NOT_YET_REQUESTED && status != Status::WAITING_FOR_BLOCK_ACK);
        ackStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
    }
    else {
        auto id = std::make_pair(dataOrMgmtFrame->getReceiverAddress(), SequenceControlField(dataOrMgmtFrame->getSequenceNumber(), dataOrMgmtFrame->getFragmentNumber()));
        Status& status = getMgmtOrNonQoSAckStatus(id);
        ASSERT(status != Status::WAITING_FOR_NORMAL_ACK);
        mgmtAckStatuses[id] = Status::FRAME_NOT_YET_TRANSMITTED;
    }
}

int QoSAckHandler::getNumberOfFramesWithStatus(Status status)
{
    int count = 0;
    for (auto ackStatus : ackStatuses)
        if (ackStatus.second == status)
            count++;
    return count;
}

std::string QoSAckHandler::getStatusString(Status status)
{
    switch (status) {
        case Status::FRAME_NOT_YET_TRANSMITTED : return "FRAME_NOT_YET_TRANSMITTED";
        case Status::NO_ACK_REQUIRED : return "NO_ACK_REQUIRED";
        case Status::BLOCK_ACK_NOT_YET_REQUESTED : return "BLOCK_ACK_NOT_YET_REQUESTED";
        case Status::WAITING_FOR_NORMAL_ACK : return "WAITING_FOR_NORMAL_ACK";
        case Status::NORMAL_ACK_NOT_ARRIVED : return "NORMAL_ACK_NOT_ARRIVED";
        case Status::BLOCK_ACK_ARRIVED_UNACKED : return "BLOCK_ACK_ARRIVED_UNACKED";
        case Status::BLOCK_ACK_ARRIVED_ACKED  : return "BLOCK_ACK_ARRIVED_ACKED";
        case Status::WAITING_FOR_BLOCK_ACK  : return "WAITING_FOR_BLOCK_ACK";
        case Status::NORMAL_ACK_ARRIVED  : return "NORMAL_ACK_ARRIVED";
        default: throw cRuntimeError("Unknown status");
    }
}

void QoSAckHandler::printAckStatuses()
{
//    for (auto ackStatus : ackStatuses) {
//        std::cout << "Seq Num = " << ackStatus.first.getSequenceNumber() << " " << "Frag Num = " << (int)ackStatus.first.getFragmentNumber() << std::endl;
//        std::cout << "Status = " << getStatusString(ackStatus.second) << std::endl;
//    }
//    std::cout << "=========================================" << std::endl;
}


} /* namespace ieee80211 */
} /* namespace inet */
