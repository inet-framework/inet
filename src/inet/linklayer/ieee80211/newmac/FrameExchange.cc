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
#include "IContentionTx.h"
#include "IImmediateTx.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

FrameExchange::FrameExchange(FrameExchangeContext *context, IFinishedCallback *callback) :
    MacPlugin(context->ownerModule),
    params(context->params),
    utils(context->utils),
    immediateTx(context->immediateTx),
    contentionTx(context->contentionTx),
    finishedCallback(callback)
{
}

FrameExchange::~FrameExchange()
{
}

void FrameExchange::transmitContentionFrame(Ieee80211Frame *frame, int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount)
{
    contentionTx[txIndex]->transmitContentionFrame(frame, ifs, eifs, cwMin, cwMax, slotTime, retryCount, this);
}

void FrameExchange::transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs)
{
    immediateTx->transmitImmediateFrame(frame, ifs, this);
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
    handleWithFSM(EVENT_START, nullptr);
}

bool FsmBasedFrameExchange::lowerFrameReceived(Ieee80211Frame *frame)
{
    return handleWithFSM(EVENT_FRAMEARRIVED, frame);
}

void FsmBasedFrameExchange::transmissionComplete(int txIndex)
{
    handleWithFSM(EVENT_TXFINISHED, nullptr);
}

void FsmBasedFrameExchange::internalCollision(int txIndex)
{
    handleWithFSM(EVENT_INTERNALCOLLISION, nullptr);
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
        CASE(TRANSMIT_CONTENTION_FRAME);
        CASE(TRANSMIT_IMMEDIATE_FRAME);
        CASE(EXPECT_REPLY);
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
        case TRANSMIT_CONTENTION_FRAME: return "transmitContentionFrame()";
        case TRANSMIT_IMMEDIATE_FRAME: return "transmitImmediateFrame()";
        case EXPECT_REPLY: return "expectReply()";
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
                throw cRuntimeError(this, "doStep(step=%d) should have executed an operation like transmitContentionFrame(), transmitImmediateFrame(), expectReply(), gotoStep(), fail(), or succeed()", step);
            if (operation == GOTO_STEP)
                proceed();
        }
        else {
            cleanupAndReportResult(); // should be the last call in the lifetime of the object
        }
    }
}

void StepBasedFrameExchange::transmissionComplete(int txIndex)
{
    ASSERT(status == INPROGRESS);
    ASSERT(operation == TRANSMIT_CONTENTION_FRAME || operation == TRANSMIT_IMMEDIATE_FRAME);
    EV_DETAIL << "Transmission complete\n";
    proceed();
}

bool StepBasedFrameExchange::lowerFrameReceived(Ieee80211Frame *frame)
{
    EV_DETAIL << "Lower frame received in step " << step << "\n";
    ASSERT(status == INPROGRESS);

    if (operation != EXPECT_REPLY)
        return false;    // not ready to process frames
    else {
        operation = NONE;
        bool accepted = processReply(step, frame);
        if (status == INPROGRESS) {
            logStatus(accepted ? "processReply() returned ACCEPT": "processReply() returned REJECT");
            checkOperation(operation, "processReply()"); //FIXME gotoStep+return false will result in strange behavior
            if (accepted)
                proceed();
        }
        else {
            cleanupAndReportResult(); // should be the last call in the lifetime of the object
        }
        return accepted;
    }
}

void StepBasedFrameExchange::handleSelfMessage(cMessage *msg)
{
    EV_DETAIL << "Timeout in step " << step << "\n";
    ASSERT(status == INPROGRESS);
    ASSERT(operation == EXPECT_REPLY);
    ASSERT(msg == timeoutMsg);

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

void StepBasedFrameExchange::internalCollision(int txIndex)
{
    EV_DETAIL << "Internal collision in step " << step << "\n";
    ASSERT(status == INPROGRESS);
    ASSERT(operation == TRANSMIT_CONTENTION_FRAME);

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
        case NONE: case GOTO_STEP: break; // compensate for step++ in proceed()
        case FAIL: case SUCCEED: ASSERT(false); break;  // it is not safe to do anything after fail() or succeed(), as the callback may delete this object
        default: throw cRuntimeError(this, "operation %s is not permitted inside %s, only gotoStep(), fail() and succeed())", where, operationFunctionName(operation));
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

void StepBasedFrameExchange::transmitContentionFrame(Ieee80211Frame *frame, int retryCount)
{
    transmitContentionFrame(frame, retryCount, defaultTxIndex, defaultAccessCategory);
}

void StepBasedFrameExchange::transmitContentionFrame(Ieee80211Frame *frame, int retryCount, int txIndex, AccessCategory ac)
{
    setOperation(TRANSMIT_CONTENTION_FRAME);
    contentionTx[txIndex]->transmitContentionFrame(frame, params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMin(ac), params->getCwMax(ac), params->getSlotTime(), retryCount, this);
}

void StepBasedFrameExchange::transmitMulticastContentionFrame(Ieee80211Frame *frame)
{
    transmitMulticastContentionFrame(frame, defaultTxIndex, defaultAccessCategory);
}

void StepBasedFrameExchange::transmitMulticastContentionFrame(Ieee80211Frame *frame, int txIndex, AccessCategory ac)
{
    setOperation(TRANSMIT_CONTENTION_FRAME);
    contentionTx[txIndex]->transmitContentionFrame(frame, params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMulticast(ac), params->getCwMulticast(ac), params->getSlotTime(), 0, this);
}

void StepBasedFrameExchange::transmitContentionFrame(Ieee80211Frame *frame, int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount)
{
    setOperation(TRANSMIT_CONTENTION_FRAME);
    contentionTx[txIndex]->transmitContentionFrame(frame, ifs, eifs, cwMin, cwMax, slotTime, retryCount, this);
}

void StepBasedFrameExchange::transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs)
{
    setOperation(TRANSMIT_IMMEDIATE_FRAME);
    immediateTx->transmitImmediateFrame(frame, ifs, this);
}

void StepBasedFrameExchange::expectReply(simtime_t timeout)
{
    setOperation(EXPECT_REPLY);
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

