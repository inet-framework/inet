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
// Author: Benjamin Seregi
//

#include "Ieee80211MacAdvancedFrameExchange.h"
#include "Ieee80211MacTransmission.h"
#include "Ieee80211UpperMac.h"

namespace inet {
namespace ieee80211 {

Ieee80211AdvancedFrameExchange::Ieee80211AdvancedFrameExchange(Ieee80211NewMac* mac, IFinishedCallback* callback) :
        Ieee80211FrameExchange(mac, callback)
{
    timeout = new cMessage("Timeout");
    waitTxComplete = new WaitTxCompleteStep("Waiting for transmission to complete");
    exchangeFinished = new ExchangeFinishedStep("Exchange finished");
}

void Ieee80211AdvancedFrameExchange::transmit(FrameExchangeStep* step)
{
    if (step->isImmediate())
        mac->transmitImmediateFrame(step->getFrame(), step->getDeferDuration());
    else
        getTransmission()->transmitContentionFrame(step->getFrame(), step->getDeferDuration(), step->getCw());
    FrameExchangeStepWithResponse *stepWithResponse = dynamic_cast<FrameExchangeStepWithResponse *>(step);
    if (stepWithResponse)
    {
        if(!timeout->isScheduled())
            scheduleAt(simTime() + stepWithResponse->getTimeout(), timeout);
        else
            reportFailure();
    }
}

void Ieee80211AdvancedFrameExchange::doStep()
{
    if (dynamic_cast<FrameExchangeStep *>(currentStep))
    {
        transmit(dynamic_cast<FrameExchangeStep *>(currentStep));
        setCurrentStep(++stepId);
    }
    else if (dynamic_cast<ExchangeFinishedStep *>(currentStep))
        reportSuccess();
    else
        reportFailure();
}

void Ieee80211AdvancedFrameExchange::lowerFrameReceived(Ieee80211Frame* frame)
{
    FrameExchangeStepWithResponse *step = dynamic_cast<FrameExchangeStepWithResponse *>(currentStep);
    if (step && step->isResponseOk(frame))
    {
        cancelEvent(timeout);
        setCurrentStep(++stepId);
        doStep();
    }
    else
        reportFailure();
}

void Ieee80211AdvancedFrameExchange::transmissionFinished()
{
    if (dynamic_cast<WaitTxCompleteStep *>(currentStep))
        setCurrentStep(++stepId);
    else
        reportFailure();
}

void Ieee80211AdvancedFrameExchange::handleMessage(cMessage* timer)
{
    if (timer == timeout)
    {
        FrameExchangeStepWithResponse *stepWithResponse = dynamic_cast<FrameExchangeStepWithResponse *>(currentStep);
        // Retransmission
        if (stepWithResponse && stepWithResponse->getRetryCount() < stepWithResponse->getMaxRetryCount())
        {
            EV_INFO << "No response has arrived within t=" << stepWithResponse->getTimeout() << ". Starting retransmission!" << endl;
            stepWithResponse->increaseRetryCount();
            stepWithResponse->increaseContentionWindowIfPossible();
            stepWithResponse->getFrame()->setRetry(true);
            transmit(stepWithResponse);
        }
        else
            reportFailure();
    }
    else
        throw cRuntimeError("Unknown msg");
}

Ieee80211SendRtsCtsFrameExchange::Ieee80211SendRtsCtsFrameExchange(Ieee80211NewMac* mac, IFinishedCallback* callback, Ieee80211DataOrMgmtFrame* frameToSend) :
        Ieee80211AdvancedFrameExchange(mac, callback)
{
    rtsCtsExchange = new FrameExchangeStepWithResponse("RTS/CTS Exchange", buildRtsFrame(frameToSend), getUpperMac()->getDIFS(), 3, 511, false, computeCtsTimeout(), [=] (Ieee80211Frame *frame) { return isCtsFrame(frame); }, 10);
}

void Ieee80211SendRtsCtsFrameExchange::setCurrentStep(int step)
{
    switch(step)
    {
        case 0:
            currentStep = rtsCtsExchange; // Transmit RTS
            break;
        case 1:
            currentStep = waitTxComplete; // Wait for RTS tx complete
            break;
        case 2:
            currentStep =  rtsCtsExchange; // Wait for CTS
            break;
        case 3:
            currentStep = exchangeFinished; // RTS-CTS-DATA-ACK exchange finished
    }
}

simtime_t Ieee80211SendRtsCtsFrameExchange::computeCtsTimeout()
{
    return getUpperMac()->computeFrameDuration(LENGTH_RTS, mac->basicBitrate) + getUpperMac()->getSIFS() + getUpperMac()->computeFrameDuration(LENGTH_CTS, mac->basicBitrate) + MAX_PROPAGATION_DELAY * 2;
}

Ieee80211RTSFrame* Ieee80211SendRtsCtsFrameExchange::buildRtsFrame(Ieee80211DataOrMgmtFrame* frameToSend)
{
    Ieee80211RTSFrame *rtsFrame = new Ieee80211RTSFrame("RTS Frame");
    rtsFrame->setTransmitterAddress(mac->address);
    rtsFrame->setReceiverAddress(frameToSend->getReceiverAddress());
    rtsFrame->setDuration(3 * getUpperMac()->getSIFS() + getUpperMac()->computeFrameDuration(LENGTH_CTS, mac->basicBitrate) +
            getUpperMac()->computeFrameDuration(frameToSend) +
            getUpperMac()->computeFrameDuration(LENGTH_ACK, mac->basicBitrate));
    return rtsFrame;
}

Ieee80211SendDataAckFrameExchange::Ieee80211SendDataAckFrameExchange(Ieee80211NewMac* mac, IFinishedCallback* callback, Ieee80211DataOrMgmtFrame* frameToSend) :
        Ieee80211AdvancedFrameExchange(mac, callback)
{
    dataAckExchange = new FrameExchangeStepWithResponse("DATA/ACK Exchange", frameToSend, getUpperMac()->getSIFS(), 3, 511, true, computeAckTimeout(frameToSend), [=] (Ieee80211Frame *frame) { return isAckFrame(frame); }, 10);
}

void Ieee80211SendDataAckFrameExchange::setCurrentStep(int step)
{
    switch(step)
    {
        case 0:
            currentStep = dataAckExchange; // Transmit DATA
            break;
        case 1:
            currentStep = waitTxComplete; // Wait for DATA tx complete
            break;
        case 2:
            currentStep = dataAckExchange; // Wait for ACK
            break;
        case 3:
            currentStep = exchangeFinished; // DATA-ACK exchange finished
    }
}


simtime_t Ieee80211SendDataAckFrameExchange::computeAckTimeout(Ieee80211DataOrMgmtFrame *frameToSend)
{
    return getUpperMac()->computeFrameDuration(frameToSend) + getUpperMac()->getSIFS() + getUpperMac()->computeFrameDuration(LENGTH_ACK, mac->basicBitrate) + MAX_PROPAGATION_DELAY * 2;
}


Ieee80211SendRtsCtsDataAckFrameExchange::Ieee80211SendRtsCtsDataAckFrameExchange(Ieee80211NewMac* mac, IFinishedCallback* callback, Ieee80211DataOrMgmtFrame* frameToSend) :
        Ieee80211AdvancedFrameExchange(mac, callback),
        Ieee80211SendRtsCtsFrameExchange(mac, callback, frameToSend),
        Ieee80211SendDataAckFrameExchange(mac, callback, frameToSend)
{
}

void Ieee80211SendRtsCtsDataAckFrameExchange::setCurrentStep(int step)
{
    switch(step)
    {
        case 0:
            EV_INFO << "Starting RTS/CTS exchange" << endl;
            currentStep = rtsCtsExchange;
            break;
        case 1:
            EV_INFO << "Waiting for RTS tx to complete" << endl;
            currentStep = waitTxComplete;
            break;
        case 2:
            EV_INFO << "Waiting for CTS response" << endl;
            currentStep =  rtsCtsExchange;
            break;
        case 3:
            EV_INFO << "Starting DATA/ACK exchange" << endl;
            currentStep = dataAckExchange;
            break;
        case 4:
            EV_INFO << "Waiting for DATA tx to complete" << endl;
            currentStep = waitTxComplete;
            break;
        case 5:
            EV_INFO << "Waiting for ACK response" << endl;
            currentStep = dataAckExchange;
            break;
        case 6:
            EV_INFO << "RTS-CTS-DATA-ACK exchange finished" << endl;
            currentStep = exchangeFinished;
    }
}

}
} /* namespace inet */

