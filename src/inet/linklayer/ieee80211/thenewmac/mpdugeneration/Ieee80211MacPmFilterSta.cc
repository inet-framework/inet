//
// Copyright (C) 2015 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "Ieee80211MacPmFilterSta.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacPmFilterSta);

void Ieee80211MacPmFilterSta::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        auto processSignalFunction = [=] (cMessage *m) { processSignal(m); };
        sdlProcess = new SdlProcess(processSignalFunction,
                                  {{PM_FILTER_STA_STATE_START,
                                    {},
                                    {}},
                                  {PM_FILTER_STA_STATE_PM_IDLE,
                                    {{-1, [=] (cMessage *m) { processPmIdleContinuousSignal1(); }, false, nullptr, [=] () { return isPmIdleContinuoisSignal1Enabled(); }},
                                    {-1, [=] (cMessage *m) { processPmIdleContinuousSignal2(); }, false, nullptr, [=] () { return isPmIdleContinuoisSignal2Enabled(); }},
                                    {-1, [=] (cMessage *m) { processAllStatesContinuousSignal1(); }, false, nullptr, [=] () { return isAllStatesContinuousSignal1Enabled(); }},
                                    {FRAG_REQUEST},
                                    {PDU_CONFIRM}},
                                    {}},
                                  {PM_FILTER_STA_STATE_PM_BSS,
                                    {{-1, [=] (cMessage *m) { processPmBssContinuousSignal1(); }, false, nullptr, [=] () { return isPmBssContinuousSignal1Enabled(); }},
                                    {-1, [=] (cMessage *m) { processPmBssContinuousSignal2(); }, false, nullptr, [=] () { return isPmBssContinuousSignal2Enabled(); }},
                                    {-1, [=] (cMessage *m) { processAllStatesContinuousSignal1(); }, false, nullptr, [=] () { return isAllStatesContinuousSignal1Enabled(); }},
                                    {FRAG_REQUEST},
                                    {PDU_CONFIRM}},
                                    {}},
                                  {PM_FILTER_STA_STATE_PM_IBSS_DATA,
                                    {{-1, [=] (cMessage *m) { processPmIbssDataContinuousSignal1(); }, false, nullptr, [=] () { return isPmIbssDataContinuousSignal1Enabled(); }},
                                    {-1, [=] (cMessage *m) { processPmIbssDataContinuousSignal2(); }, false, nullptr, [=] () { return isPmIbssDataContinuousSignal2Enabled(); }},
                                    {-1, [=] (cMessage *m) { processPmIbssDataContinuousSignal3(); }, false, nullptr, [=] () { return isPmIbssDataContinuousSignal3Enabled(); }},
                                    {-1, [=] (cMessage *m) { processAllStatesContinuousSignal1(); }, false, nullptr, [=] () { return isAllStatesContinuousSignal1Enabled(); }},
                                    {PDU_CONFIRM},
                                    {PS_CHANGE},
                                    {FRAG_REQUEST}},
                                    {ATIM_W}},
                                  {PM_FILTER_STA_STATE_BSS_CFP,
                                    {{-1, [=] (cMessage *m) { processBssCfpContinuousSignal1(); }, false, nullptr, [=] () { return isBssCfpContinuousSignal1Enabled(); }},
                                    {-1, [=] (cMessage *m) { processAllStatesContinuousSignal1(); }, false, nullptr, [=] () { return isAllStatesContinuousSignal1Enabled(); }},
                                    {FRAG_REQUEST},
                                    {PDU_CONFIRM},
                                    {CF_POLLED}},
                                    {}},
                                  {PM_FILTER_STA_WAIT_PS_RESPONSE,
                                    {{PS_RESPONSE},
                                    {-1, [=] (cMessage *m) { processAllStatesContinuousSignal1(); }, false, nullptr, [=] () { return isAllStatesContinuousSignal1Enabled(); }}},
                                    {-1}},
                                  {PM_FILTER_STA_PRE_ATIM,
                                    {{-1, [=] (cMessage *m) { processAllStatesContinuousSignal1(); }, false, nullptr, [=] () { return isAllStatesContinuousSignal1Enabled(); }},
                                    {ATIM_W}},
                                    {-1}},
                                  {PM_FILTER_STA_IBSS_ATIM_W,
                                    {{-1, [=] (cMessage *m) { processPmIbssAtimWContinuousSignal1(); }, false, nullptr, [=] () { return isPmIbssAtimWContinuousSignal1Enabled(); }},
                                    {-1, [=] (cMessage *m) { processPmIbssAtimWContinuousSignal2(); }, false, nullptr, [=] () { return isPmIbssAtimWContinuousSignal2Enabled(); }},
                                    {-1, [=] (cMessage *m) { processAllStatesContinuousSignal1(); }, false, nullptr, [=] () { return isAllStatesContinuousSignal1Enabled(); }},
                                    {FRAG_REQUEST}},
                                    {PS_CHANGE}}
                                 });
        state = PM_FILTER_STA_STATE_PM_IDLE;
        sdlProcess->setCurrentState(state);
        subscribe(Ieee80211MacMacsorts::intraMacRemoteVariablesChanged, this);
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);
        macmib = getModuleFromPar<Ieee80211MacMacmibPackage>(par("macmibPackage"), this);
    }
}

void Ieee80211MacPmFilterSta::processSignal(cMessage* msg)
{
    if (dynamic_cast<Ieee80211MacSignalFragRequest *>(msg->getControlInfo()))
        handleFragRequest(dynamic_cast<Ieee80211MacSignalFragRequest *>(msg->getControlInfo()), dynamic_cast<FragSdu *>(msg));
    else if (dynamic_cast<Ieee80211MacSignalPduConfirm *>(msg->getControlInfo()))
        handlePduConfirm(dynamic_cast<Ieee80211MacSignalPduConfirm *>(msg->getControlInfo()));
    else if (dynamic_cast<Ieee80211MacSignalCfPolled *>(msg->getControlInfo()))
        handleCfPolled(dynamic_cast<Ieee80211MacSignalCfPolled *>(msg->getControlInfo()));
    else if (dynamic_cast<Ieee80211MacSignalAtimW *>(msg->getControlInfo()))
        handleAtimW(dynamic_cast<Ieee80211MacSignalAtimW *>(msg->getControlInfo()));
    else if (dynamic_cast<Ieee80211MacSignalPsChange *>(msg->getControlInfo()))
        handlePsChange(dynamic_cast<Ieee80211MacSignalPsChange *>(msg->getControlInfo()));
    else if (dynamic_cast<Ieee80211MacSignalPsResponse *>(msg->getControlInfo()))
        handlePsResponse(dynamic_cast<Ieee80211MacSignalPsResponse *>(msg->getControlInfo()));
}

void Ieee80211MacPmFilterSta::handleResetMac()
{
    // TODO: in all states: mDisable continuous signal
    anQ.clear();
    cfQ.clear();
    psQ.clear();
    txQ.clear();
    state = PM_FILTER_STA_STATE_PM_IDLE;
    sdlProcess->setCurrentState(state);
}

void Ieee80211MacPmFilterSta::handleFragRequest(Ieee80211MacSignalFragRequest *fragRequest, FragSdu *fsdu)
{
    this->fsdu = fsdu;
    if (state == PM_FILTER_STA_STATE_PM_IDLE)
        emitPduRequest(fsdu);
    else if (state == PM_FILTER_STA_STATE_PM_BSS || state == PM_FILTER_STA_STATE_BSS_CFP)
    {
        if (fsdu->cf == CfPriority::CfPriority_contention)
            txQ.push_back(fsdu);
        else if (fsdu->cf == CfPriority::CfPriority_contentionFree)
            cfQ.push_back(fsdu);
        else
            throw cRuntimeError("Unknown CfPriority");
    }
    else if (state == PM_FILTER_STA_STATE_PM_IBSS_DATA)
    {
        emitPsInquiry(fsdu->dst);
        state = PM_FILTER_STA_WAIT_PS_RESPONSE;
        sdlProcess->setCurrentState(state);
    }
    else if (state == PM_FILTER_STA_IBSS_ATIM_W)
    {
        if (sentBcn)
        {
            emitPsInquiry(fsdu->dst);
            state = PM_FILTER_STA_WAIT_PS_RESPONSE;
            sdlProcess->setCurrentState(state);
        }
        else
        {
            if (fsdu->pdus[1]->getFtype() == TypeSubtype_beacon)
            {
                emitPduRequest(fsdu);
                sentBcn = true;
            }
            else
            {
                // todo: fragrequest to self
            }
        }
    }
}

void Ieee80211MacPmFilterSta::handlePduConfirm(Ieee80211MacSignalPduConfirm* pduConfirm)
{
    fsdu = new FragSdu(pduConfirm->getFsdu());
    resl = pduConfirm->getTxResult();
    if (state == PM_FILTER_STA_STATE_PM_IDLE)
        emitFragConfirm(fsdu, resl);
    else if (state == PM_FILTER_STA_STATE_PM_BSS)
    {
        fsPend = false;
        if (resl == TxResult::TxResult_partial)
        {
            fsdu->resume = true;
            txQ.push_front(fsdu);
        }
        else
            emitFragConfirm(fsdu, resl);
    }
    else if (state == PM_FILTER_STA_STATE_BSS_CFP)
    {
        fsPend = false;
        if (resl == TxResult::TxResult_partial)
        {
            if (fsdu->cf == CfPriority::CfPriority_contention)
            {
                fsdu->resume = true;
                txQ.push_front(fsdu);
            }
            else if (fsdu->cf == CfPriority::CfPriority_contention)
            {
                cfQ.push_front(fsdu);
                fsPend = false; // ??
            }
            else
                throw cRuntimeError("error");
        }
        else
            emitFragConfirm(fsdu, resl);
    }
    else if (state == PM_FILTER_STA_STATE_PM_IBSS_DATA)
    {
        fsPend = false;
        if (resl == TxResult::TxResult_partial)
        {
            fsdu->resume = true;
            if (fsdu->psm)
                anQ.push_front(fsdu);
            else
                txQ.push_front(fsdu);
        }
        else
            emitFragConfirm(fsdu, resl);
    }
    else if (state == PM_FILTER_STA_IBSS_ATIM_W)
    {
        atPend = false;
        if (resl == TxResult::TxResult_atimAck)
            anQ.push_back(fsdu);
        else
            psQ.push_back(fsdu);
    }
}

void Ieee80211MacPmFilterSta::emitPduRequest(FragSdu *fsdu)
{
    Ieee80211MacSignalPduRequest *signal = new Ieee80211MacSignalPduRequest();
    delete fsdu->removeControlInfo(); // TODO
    createSignal(fsdu, signal);
    send(fsdu, "mpdu$o");
}

void Ieee80211MacPmFilterSta::handleCfPolled(Ieee80211MacSignalCfPolled* cfPolled)
{
    if (cfQ.size() > 0)
    {
        fsdu = cfQ.front();
        cfQ.pop_front();
        if (cfQ.size() + txQ.size() > 0)
        {
            // set moreData
            // bit in each
            // fsdu fragment

            // TODO: Is it OK to set it true in the last fragment too?
            for (int i = 0; i < fsdu->fTot; i++)
                fsdu->pdus[i]->setMoreData(true);
        }
        emitPduRequest(fsdu);
    }
    else // == 0
    {
        if (txQ.size() > 0)
        {
            fsdu = txQ.front();
            txQ.pop_front();
            if (txQ.size() > 0)
            {
                // set moreData
                // bit in each
                // fsdu fragment
                for (int i = 0; i < fsdu->fTot; i++)
                    fsdu->pdus[i]->setMoreData(true);
                }
            else
            {
                emitPduRequest(fsdu);
            }
        }
        else //
        {
//            Send null SDU if CFqueue empty. TxCtl
//            then responds with CfAck or Null rather
//            than Data or DataAck.
//            emitPduRequest(nullSdu); TODO: nullSdu
        }

    }
}

void Ieee80211MacPmFilterSta::emitFragConfirm(FragSdu *fsdu, TxResult txResult)
{
    Ieee80211MacSignalFragConfirm *signal = new Ieee80211MacSignalFragConfirm();
    signal->setTxResult(txResult);
    createSignal(fsdu, signal);
    send(fsdu, "fragMsdu$o");
}

void Ieee80211MacPmFilterSta::receiveSignal(cComponent *source, int signalID, cObject *obj)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);
    if (signalID == Ieee80211MacMacsorts::intraMacRemoteVariablesChanged)
        emitDataChanged();
}

void Ieee80211MacPmFilterSta::handleAtimW(Ieee80211MacSignalAtimW* awtimW)
{
    if (state == PM_FILTER_STA_PRE_ATIM)
    {
        n = anQ.size();
        while (n > 0)
        {
            psQ.push_back(anQ.front());
            anQ.pop_front();
            n--;
        }
        state = PM_FILTER_STA_IBSS_ATIM_W;
        sdlProcess->setCurrentState(state);
    }
}

void Ieee80211MacPmFilterSta::handlePsChange(Ieee80211MacSignalPsChange* psChange)
{
    if (state == PM_FILTER_STA_STATE_PM_IBSS_DATA)
    {
//        n:=qsearch(psQ, sta)
        while (n >= 0)
        {
            txQ.push_back(psQ.at(n));
            // n:=qsearch(psQ, sta)
            //psQ:=subQ(psQ,0,n)//subQ(psQ,n+1,length(psQ)-n-1)
        }
    }
}

void Ieee80211MacPmFilterSta::handlePsResponse(Ieee80211MacSignalPsResponse *psResponse)
{
    if (state == PM_FILTER_STA_WAIT_PS_RESPONSE)
    {
        dpsm = psResponse->getPsMode();
        if (dpsm == PsMode::PsMode_sta_active)
            txQ.push_back(fsdu);
        else
        {
            fsdu->psm = true;
            psQ.push_back(fsdu);
        }
        state = PM_FILTER_STA_IBSS_ATIM_W;
        sdlProcess->setCurrentState(state);
    }
}

void Ieee80211MacPmFilterSta::emitPsInquiry(MACAddress dst)
{
    Ieee80211MacSignalPsInquiry *signal = new Ieee80211MacSignalPsInquiry();
    signal->setMacAddress(dst);
    cMessage *psInquiry = createSignal("psInquiry", signal);
    send(psInquiry, "pwrMgt$o");
}

void Ieee80211MacPmFilterSta::processPmIdleContinuousSignal1()
{
    state = PM_FILTER_STA_STATE_PM_BSS;
    sdlProcess->setCurrentState(state);
}

bool Ieee80211MacPmFilterSta::isPmIdleContinuoisSignal1Enabled()
{
    return macsorts->getIntraMacRemoteVariables()->isAssoc();
}

void Ieee80211MacPmFilterSta::processPmIdleContinuousSignal2()
{
    state = PM_FILTER_STA_STATE_PM_IBSS_DATA;
    sdlProcess->setCurrentState(state);
}

bool Ieee80211MacPmFilterSta::isPmIdleContinuoisSignal2Enabled()
{
    return macsorts->getIntraMacRemoteVariables()->isIbss();
}

void Ieee80211MacPmFilterSta::processPmBssContinuousSignal1()
{
    fsdu = txQ.front();
    txQ.pop_front();
    emitPduRequest(fsdu);
    fsPend = true;
}

bool Ieee80211MacPmFilterSta::isPmBssContinuousSignal1Enabled()
{
    return !fsPend && (txQ.size() != 0);
}

void Ieee80211MacPmFilterSta::processPmBssContinuousSignal2()
{
    state = PM_FILTER_STA_STATE_BSS_CFP;
    sdlProcess->setCurrentState(state);
}

bool Ieee80211MacPmFilterSta::isPmBssContinuousSignal2Enabled()
{
    return macsorts->getIntraMacRemoteVariables()->isCfp();
}

void Ieee80211MacPmFilterSta::processBssCfpContinuousSignal1()
{
    state = PM_FILTER_STA_STATE_PM_BSS;
    sdlProcess->setCurrentState(state);
}

bool Ieee80211MacPmFilterSta::isBssCfpContinuousSignal1Enabled()
{
    return !macsorts->getIntraMacRemoteVariables()->isCfp();
}

void Ieee80211MacPmFilterSta::processPmIbssDataContinuousSignal1()
{
    state = PM_FILTER_STA_PRE_ATIM;
    sdlProcess->setCurrentState(state);
}

bool Ieee80211MacPmFilterSta::isPmIbssDataContinuousSignal1Enabled()
{
    return !fsPend && macsorts->getIntraMacRemoteVariables()->isAtimW();
}

void Ieee80211MacPmFilterSta::processPmIbssDataContinuousSignal2()
{
    fsdu = anQ.front();
    anQ.pop_front();
    emitPduRequest(fsdu);
    fsPend = true;
}

bool Ieee80211MacPmFilterSta::isPmIbssDataContinuousSignal2Enabled()
{
    return !fsPend && (anQ.size() != 0);
}

void Ieee80211MacPmFilterSta::processPmIbssDataContinuousSignal3()
{
    fsdu = txQ.front();
    txQ.pop_front();
    emitPduRequest(fsdu);
    fsPend = true;
}

bool Ieee80211MacPmFilterSta::isPmIbssDataContinuousSignal3Enabled()
{
    return !fsPend && (anQ.size() == 0) && (txQ.size() != 0);
}

void Ieee80211MacPmFilterSta::processPmIbssAtimWContinuousSignal1()
{
    sentBcn = false;
    state = PM_FILTER_STA_STATE_PM_IBSS_DATA;
    sdlProcess->setCurrentState(state);
}

bool Ieee80211MacPmFilterSta::isPmIbssAtimWContinuousSignal1Enabled()
{
    return !atPend && !macsorts->getIntraMacRemoteVariables()->isAtimW();
}

void Ieee80211MacPmFilterSta::processPmIbssAtimWContinuousSignal2()
{
    fsdu = psQ.front();
    psQ.pop_front();
    emitPduRequest(fsdu);
    atPend = true;
}

bool Ieee80211MacPmFilterSta::isPmIbssAtimWContinuousSignal2Enabled()
{
    return !atPend && (psQ.size() != 0);
}

void Ieee80211MacPmFilterSta::processAllStatesContinuousSignal1()
{
    handleResetMac();
}

bool Ieee80211MacPmFilterSta::isAllStatesContinuousSignal1Enabled()
{
    // TODO: hack
    return false;
    return macsorts->getIntraMacRemoteVariables()->isDisable();
}

} /* namespace inet */
} /* namespace ieee80211 */

