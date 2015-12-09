//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#include "FrameExchange.h"
#include "IMacParameters.h"
#include "IContention.h"
#include "ITx.h"
#include "IRx.h"
#include "Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

FrameExchange::FrameExchange(FrameExchangeContext *context, IFinishedCallback *callback) :
    MacPlugin(context->ownerModule),
    params(context->params),
    utils(context->utils),
    tx(context->tx),
    contention(context->contention),
    rx(context->rx),
    statistics(context->statistics),
    finishedCallback(callback)
{
}

FrameExchange::~FrameExchange()
{
}

void FrameExchange::startContention(int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount)
{
    contention[txIndex]->startContention(ifs, eifs, cwMin, cwMax, slotTime, retryCount, this);
}

void FrameExchange::transmitFrame(Ieee80211Frame *frame, simtime_t ifs)
{
    tx->transmitFrame(frame, ifs, this);
}

void FrameExchange::releaseChannel(int txIndex)
{
    contention[txIndex]->channelReleased();
}

IFrameExchange::FrameProcessingResult FrameExchange::lowerFrameReceived(Ieee80211Frame *frame)
{
    return IGNORED;  // not ours
}

void FrameExchange::corruptedOrNotForUsFrameReceived()
{
    // we don't care
}

void FrameExchange::channelAccessGranted(int txIndex)
{
}

void FrameExchange::reportSuccess()
{
    EV_DETAIL << "Frame exchange successful\n";
    finishedCallback->frameExchangeFinished(this, true);    // may delete *this* FrameExchange object!
}

void FrameExchange::reportFailure()
{
    EV_DETAIL << "Frame exchange failed\n";
    finishedCallback->frameExchangeFinished(this, false);    // may delete *this* FrameExchange object!
}

//--------

void FsmBasedFrameExchange::start()
{
    EV_DETAIL << "Starting frame exchange " << getClassName() << std::endl;
    handleWithFSM(EVENT_START);
}

IFrameExchange::FrameProcessingResult FsmBasedFrameExchange::lowerFrameReceived(Ieee80211Frame *frame)
{
    return handleWithFSM(EVENT_FRAMEARRIVED, frame);
}

void FsmBasedFrameExchange::corruptedOrNotForUsFrameReceived()
{
    handleWithFSM(EVENT_CORRUPTEDFRAMEARRIVED);
}

void FsmBasedFrameExchange::transmissionComplete()
{
    handleWithFSM(EVENT_TXFINISHED);
}

void FsmBasedFrameExchange::internalCollision(int txIndex)
{
    handleWithFSM(EVENT_INTERNALCOLLISION);
}

void FsmBasedFrameExchange::handleSelfMessage(cMessage *msg)
{
    handleWithFSM(EVENT_TIMER, msg);
}

//--------

StepBasedFrameExchange::StepBasedFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, int txIndex, AccessCategory accessCategory) :
    FrameExchange(context, callback), defaultTxIndex(txIndex), defaultAccessCategory(accessCategory)
{
}

StepBasedFrameExchange::~StepBasedFrameExchange()
{
    if (timeoutMsg)
        cancelAndDelete(timeoutMsg);
}

std::string StepBasedFrameExchange::info() const
{
    std::stringstream out;
    switch (status) {
        case SUCCEEDED: out << "SUCCEEDED in step " << step; break;
        case FAILED: out << "FAILED in step " << step; break;
        case INPROGRESS: out << "step " << step << ", operation=" << operationName(operation); break;
    }
    return out.str();
}

const char *StepBasedFrameExchange::statusName(Status status)
{
#define CASE(x) case x: return #x
    switch (status) {
        CASE(SUCCEEDED);
        CASE(FAILED);
        CASE(INPROGRESS);
        default: ASSERT(false); return "???";
    }
#undef CASE
}

const char *StepBasedFrameExchange::operationName(Operation operation)
{
#define CASE(x) case x: return #x
    switch (operation) {
        CASE(NONE);
        CASE(START_CONTENTION);
        CASE(TRANSMIT_FRAME);
        CASE(EXPECT_FULL_REPLY);
        CASE(EXPECT_REPLY_RXSTART);
        CASE(GOTO_STEP);
        CASE(FAIL);
        CASE(SUCCEED);
        default: ASSERT(false); return "???";
    }
#undef CASE
}

const char *StepBasedFrameExchange::operationFunctionName(Operation operation)
{
    switch (operation) {
        case NONE: return "no-op";
        case START_CONTENTION: return "startContention()";
        case TRANSMIT_FRAME: return "transmitFrame()";
        case EXPECT_FULL_REPLY: return "expectFullReplyWithin()";
        case EXPECT_REPLY_RXSTART: return "expectReplyStartTxWithin()";
        case GOTO_STEP: return "gotoStep()";
        case FAIL: return "fail()";
        case SUCCEED: return "succeed()";
        default: ASSERT(false); return "???";
    }
}

void StepBasedFrameExchange::start()
{
    EV_DETAIL << "Starting frame exchange " << getClassName() << std::endl;
    ASSERT(step == 0);
    operation = GOTO_STEP;
    gotoTarget = 0;
    proceed();
}

void StepBasedFrameExchange::proceed()
{
    if (status == INPROGRESS) {
        if (operation == GOTO_STEP)
            step = gotoTarget;
        else
            step++;
        EV_DETAIL << "Doing step " << step << "\n";
        operation = NONE;
        doStep(step);
        if (status == INPROGRESS) {
            logStatus("doStep()");
            if (operation == NONE)
                throw cRuntimeError(this, "doStep(step=%d) should have executed an operation like startContention(), transmitFrame(), expectFullReplyWithin(), expectReplyRxStartWithin(), gotoStep(), fail(), or succeed()", step);
            if (operation == GOTO_STEP)
                proceed();
        }
        else {
            cleanupAndReportResult(); // should be the last call in the lifetime of the object
        }
    }
}

void StepBasedFrameExchange::channelAccessGranted(int txIndex)
{
    ASSERT(status == INPROGRESS);
    ASSERT(operation == START_CONTENTION);
    EV_DETAIL << "Channel access granted\n";
    proceed();
}

void StepBasedFrameExchange::transmissionComplete()
{
    ASSERT(status == INPROGRESS);
    ASSERT(operation == TRANSMIT_FRAME);
    EV_DETAIL << "Transmission complete\n";
    proceed();
}

IFrameExchange::FrameProcessingResult StepBasedFrameExchange::lowerFrameReceived(Ieee80211Frame *frame)
{
    EV_DETAIL << "Lower frame received in step " << step << "\n";
    ASSERT(status == INPROGRESS);

    if (operation == EXPECT_FULL_REPLY) {
        operation = NONE;
        FrameProcessingResult result = processReply(step, frame);
        if (status == INPROGRESS) {
            logStatus(result == IGNORED ? "processReply(): frame IGNORED": "processReply(): frame PROCESSED");
            checkOperation(operation, "processReply()");
            if (result == PROCESSED_KEEP || result == PROCESSED_DISCARD || operation != NONE)
                proceed();
            else
                operation = EXPECT_FULL_REPLY; // restore
        }
        else {
            cleanupAndReportResult(); // should be the last call in the lifetime of the object
        }
        return result;
    }
    else if (operation == EXPECT_REPLY_RXSTART) {
        operation = NONE;
        FrameProcessingResult result = processReply(step, frame);
        if (status == INPROGRESS) {
            logStatus(result == IGNORED ? "processReply(): frame IGNORED": "processReply(): frame PROCESSED");
            checkOperation(operation, "processReply()");
            if (result == PROCESSED_KEEP || result == PROCESSED_DISCARD || operation != NONE) {
                proceed();
            }
            else {
                if (timeoutMsg->isScheduled())
                    operation = EXPECT_REPLY_RXSTART; // restore operation and continue waiting
                else
                    handleTimeout();  // frame being received when timeout expired was not accepted as response: declare timeout
            }
        }
        else {
            cleanupAndReportResult(); // should be the last call in the lifetime of the object
        }
        return result;
    }
    else {
        return IGNORED; // momentarily not interested in received frames
    }
}

void StepBasedFrameExchange::corruptedOrNotForUsFrameReceived()
{
    if (operation == EXPECT_REPLY_RXSTART && !timeoutMsg->isScheduled())
        handleTimeout();  // the frame we were receiving when the timeout expired was received incorrectly
}

void StepBasedFrameExchange::handleSelfMessage(cMessage *msg)
{
    EV_DETAIL << "Timeout in step " << step << "\n";
    ASSERT(status == INPROGRESS);
    ASSERT(msg == timeoutMsg);

    if (operation == EXPECT_FULL_REPLY) {
        handleTimeout();
    }
    else if (operation == EXPECT_REPLY_RXSTART) {
        // If there's no sign of the reply (e.g ACK) being received, declare timeout.
        // Otherwise we'll wait for the frame to be fully received and be reported
        // to us via lowerFrameReceived() or corruptedFrameReceived(), and decide then.
        if (!rx->isReceptionInProgress())
            handleTimeout();
    }
    else {
        ASSERT(false);
    }
}

void StepBasedFrameExchange::internalCollision(int txIndex)
{
    EV_DETAIL << "Internal collision in step " << step << "\n";
    ASSERT(status == INPROGRESS);
    ASSERT(operation == START_CONTENTION);

    operation = NONE;
    processInternalCollision(step);
    if (status == INPROGRESS) {
        logStatus("processInternalCollision()");
        checkOperation(operation, "processInternalCollision()");
        proceed();
    }
    else {
        cleanupAndReportResult(); // should be the last call in the lifetime of the object
    }
}

void StepBasedFrameExchange::checkOperation(Operation operation, const char *where)
{
    switch (operation) {
        case NONE: case GOTO_STEP: break;
        case FAIL: case SUCCEED: ASSERT(false); break;  // it is not safe to do anything after fail() or succeed(), as the callback may delete this object
        default: throw cRuntimeError(this, "operation %s is not permitted inside %s, only gotoStep(), fail() and succeed())", where, operationFunctionName(operation));
    }
}

void StepBasedFrameExchange::handleTimeout()
{
    operation = NONE;
    processTimeout(step);
    if (status == INPROGRESS) {
        logStatus("processTimeout()");
        checkOperation(operation, "processTimeout()");
        proceed();
    }
    else {
        cleanupAndReportResult(); // should be the last call in the lifetime of the object
    }
}

void StepBasedFrameExchange::cleanupAndReportResult()
{
    cleanup();
    switch (status) {
        case SUCCEEDED: reportSuccess(); break;
        case FAILED: reportFailure(); break;
        default: ASSERT(false);
    }
    // NOTE: no members or methods may be accessed after this point, because the
    // success/failure callback might has probably deleted the frame exchange object!
}

void StepBasedFrameExchange::startContentionIfNeeded(int retryCount)
{
    if (!contention[defaultTxIndex]->isOwning())
        startContention(retryCount);
}

void StepBasedFrameExchange::startContention(int retryCount)
{
    startContention(retryCount, defaultTxIndex, defaultAccessCategory);
}

void StepBasedFrameExchange::startContention(int retryCount, int txIndex, AccessCategory ac)
{
    setOperation(START_CONTENTION);
    contention[txIndex]->startContention(params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMin(ac), params->getCwMax(ac), params->getSlotTime(), retryCount, this);
}

void StepBasedFrameExchange::startContentionForMulticast()
{
    startContentionForMulticast(defaultTxIndex, defaultAccessCategory);
}

void StepBasedFrameExchange::startContentionForMulticast(int txIndex, AccessCategory ac)
{
    setOperation(START_CONTENTION);
    contention[txIndex]->startContention(params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMulticast(ac), params->getCwMulticast(ac), params->getSlotTime(), 0, this);
}

void StepBasedFrameExchange::startContention(int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount)
{
    setOperation(START_CONTENTION);
    contention[txIndex]->startContention(ifs, eifs, cwMin, cwMax, slotTime, retryCount, this);
}

void StepBasedFrameExchange::transmitFrame(Ieee80211Frame *frame)
{
    setOperation(TRANSMIT_FRAME);
    tx->transmitFrame(frame, this);
}

void StepBasedFrameExchange::transmitFrame(Ieee80211Frame *frame, simtime_t ifs)
{
    setOperation(TRANSMIT_FRAME);
    tx->transmitFrame(frame, ifs, this);
}

void StepBasedFrameExchange::expectFullReplyWithin(simtime_t timeout)
{
    setOperation(EXPECT_FULL_REPLY);
    if (!timeoutMsg)
        timeoutMsg = new cMessage("timeout");
    scheduleAt(simTime() + timeout, timeoutMsg);
}

void StepBasedFrameExchange::expectReplyRxStartWithin(simtime_t timeout)
{
    setOperation(EXPECT_REPLY_RXSTART);
    if (!timeoutMsg)
        timeoutMsg = new cMessage("timeout");
    scheduleAt(simTime() + timeout, timeoutMsg);
}

void StepBasedFrameExchange::gotoStep(int step)
{
    setOperation(GOTO_STEP);
    gotoTarget = step;
}

void StepBasedFrameExchange::fail()
{
    setOperation(FAIL);
    status = FAILED;
    // note: we cannot call reportFailure() right here as it might delete the frame exchange object
}

void StepBasedFrameExchange::succeed()
{
    setOperation(SUCCEED);
    status = SUCCEEDED;
    // note: we cannot call reportSuccess() right here as it might delete the frame exchange object
}

void StepBasedFrameExchange::releaseChannel()
{
    releaseChannel(defaultTxIndex);
}

void StepBasedFrameExchange::setOperation(Operation newOperation)
{
    if (status != INPROGRESS)
        throw cRuntimeError(this, "cannot do operation %s: frame exchange already terminated (%s)", operationFunctionName(newOperation), statusName(status));
    if (operation != NONE)
        throw cRuntimeError(this, "only one operation is permitted per step: cannot do %s after %s, in doStep(step=%d)", operationFunctionName(newOperation), operationFunctionName(operation), step);
    operation = newOperation;
}

void StepBasedFrameExchange::logStatus(const char *what)
{
    if (status != INPROGRESS)
        EV_DETAIL << what << " in step=" << step << " terminated the frame exchange: " << statusName(status) << endl;
    else
        EV_DETAIL << what << " in step=" << step << " performed " << operationFunctionName(operation) << endl;
}

void StepBasedFrameExchange::cleanup()
{
    if (timeoutMsg)
        cancelEvent(timeoutMsg);
}


} // namespace ieee80211
} // namespace inet
