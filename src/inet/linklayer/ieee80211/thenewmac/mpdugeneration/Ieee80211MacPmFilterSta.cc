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

void Ieee80211MacPmFilterSta::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {

    }
    else
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
}

void Ieee80211MacPmFilterSta::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
//        fragMsdu = gate("fragMsdu");
//        mpdu = gate("mpdu");
//        pwrMgt = gate("pwrMgt");
        subscribe(Ieee80211MacMacsorts::intraMacRemoteVariablesChanged, this);
    }
}

void Ieee80211MacPmFilterSta::handleResetMac()
{
    // clear queue
    state = PM_FILTER_STA_STATE_PM_IDLE;
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
    }
    else if (state == PM_FILTER_STA_IBSS_ATIM_W)
    {
        if (sentBcn)
        {
            emitPsInquiry(fsdu->dst);
            state = PM_FILTER_STA_WAIT_PS_RESPONSE;
        }
        else
        {
            if (fsdu->pdus[1]) // TODO: ftype
            {
                // beacon
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

void Ieee80211MacPmFilterSta::continousSignalPmIdle1()
{
    state = PM_FILTER_STA_STATE_PM_IBSS_DATA;
}

void Ieee80211MacPmFilterSta::continousSignalPmIdle2()
{
    state = PM_FILTER_STA_STATE_PM_BSS;
}

void Ieee80211MacPmFilterSta::continuousSignalPmBss2()
{
    state = PM_FILTER_STA_STATE_BSS_CFP;
}

void Ieee80211MacPmFilterSta::continuousSignalBssCfp()
{
    state = PM_FILTER_STA_STATE_PM_BSS;
}

void Ieee80211MacPmFilterSta::handleCfPolled(Ieee80211MacSignalCfPolled* cfPolled)
{
    if (cfQ.size() > 0)
    {
        fsdu = cfQ.front();
        cfQ.pop_front();
        if (cfQ.size() + txQ.size() > 0)
        {
//            'set moreData
//            bit in each
//            fsdu fragment'
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
//                set moreData
//                bit in each
//                fsdu fragment'
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
//    Ieee80211MacSignalFragConfirm *fragConfirm = new Ieee80211MacSignalFragConfirm();
//    fragConfirm->setFsdu(*fsdu);
//    fragConfirm->setTxResult(txResult);
//    send(fragConfirm, fragMsdu);
}

void Ieee80211MacPmFilterSta::receiveSignal(cComponent *source, int signalID, cObject *obj)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);
    if (signalID == Ieee80211MacMacsorts::intraMacRemoteVariablesChanged)
    {
        switch (state)
        {
            case PM_FILTER_STA_STATE_PM_IDLE:
                if (macsorts->getIntraMacRemoteVariables()->isAssoc())
                    continousSignalPmIdle1();
                else if (macsorts->getIntraMacRemoteVariables()->isIbss())
                    continousSignalPmIdle2();
                break;
            case PM_FILTER_STA_STATE_PM_BSS:
                if (macsorts->getIntraMacRemoteVariables()->isCfp())
                    continuousSignalPmBss2();
                // TODO: continuousSignal1
                break;
            case PM_FILTER_STA_STATE_BSS_CFP:
                if (macsorts->getIntraMacRemoteVariables()->isCfp())
                    continuousSignalBssCfp();
                break;
        }

    }
}

void Ieee80211MacPmFilterSta::handleAtimW(Ieee80211MacSignalAtimW* awtimW)
{
    if (state == PM_FILTER_STA_STATE_PM_IBSS_DATA)
    {
        // TODO: save signal
    }
    else if (state == PM_FILTER_STA_PRE_ATIM)
    {
        n = anQ.size();
        while (n > 0)
        {
            psQ.push_back(anQ.front());
//            anQ:=tail(anQ);
            n--;
        }
        state = PM_FILTER_STA_IBSS_ATIM_W;
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
    }
}

void Ieee80211MacPmFilterSta::emitPsInquiry(MACAddress dst)
{
    Ieee80211MacSignalPsInquiry *signal = new Ieee80211MacSignalPsInquiry();
    signal->setMacAddress(dst);
    cMessage *psInquiry = createSignal("psInquiry", signal);
    send(psInquiry, pwrMgt);
}

} /* namespace inet */
} /* namespace ieee80211 */

