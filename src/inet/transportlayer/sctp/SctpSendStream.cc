//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpSendStream.h"

#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"

namespace inet {
namespace sctp {

SctpSendStream::SctpSendStream(SctpAssociation *assoc_, const uint16_t id)
{
    assoc = assoc_;
    streamId = id;
    nextStreamSeqNum = 0;
    bytesInFlight = 0;
    resetRequested = false;
    fragInProgress = false;

    char queueName[64];
    snprintf(queueName, sizeof(queueName), "OrderedSendQueue ID %d", id);
    streamQ = new cPacketQueue(queueName);
    snprintf(queueName, sizeof(queueName), "UnorderedSendQueue ID %d", id);
    uStreamQ = new cPacketQueue(queueName);
}

SctpSendStream::~SctpSendStream()
{
    deleteQueue();
}

void SctpSendStream::deleteQueue()
{
    SctpDataMsg *datMsg;
    SctpSimpleMessage *smsg;
    while (!streamQ->isEmpty()) {
        datMsg = check_and_cast<SctpDataMsg *>(streamQ->pop());
        smsg = check_and_cast<SctpSimpleMessage *>(datMsg->decapsulate());
        delete smsg;
        delete datMsg;
    }
    while (!uStreamQ->isEmpty()) {
        datMsg = check_and_cast<SctpDataMsg *>(uStreamQ->pop());
        smsg = check_and_cast<SctpSimpleMessage *>(datMsg->decapsulate());
        delete smsg;
        delete datMsg;
    }
    delete streamQ;
    delete uStreamQ;
}

uint32_t SctpSendStream::getNextStreamSeqNum() {
    return nextStreamSeqNum;
};

void SctpSendStream::setNextStreamSeqNum(const uint16_t num) {
    nextStreamSeqNum = num;
};

} // namespace sctp
} // namespace inet

