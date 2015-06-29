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

#include "Ieee80211MacPrepareMpdu.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacPrepareMpdu);

void Ieee80211MacPrepareMpdu::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        throw cRuntimeError("This module doesn't handle self messages.")
    }
    else
    {
        if (dynamic_cast<Ieee80211MacSignalMsduRequest *>(msg->getControlInfo()))
            handleMsduRequest(dynamic_cast<Ieee80211MacSignalMsduRequest *>(msg->getControlInfo()), dynamic_cast<Ieee80211NewFrame *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalMmRequest *>(msg->getControlInfo()))
            handleMmRequest(dynamic_cast<Ieee80211MacSignalMmRequest *>(msg->getControlInfo()), dynamic_cast<Ieee80211NewFrame *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalFragConfirm *>(msg->getControlInfo()))
            handleFragConfirm(dynamic_cast<Ieee80211MacSignalFragConfirm *>(msg->getControlInfo()));
    }
}

void Ieee80211MacPrepareMpdu::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);
        macmib = getModuleFromPar<Ieee80211MacMacmibPackage>(par("macmibPackage"), this);

//        msdu = gate("msdu");
//        fragMsdu = gate("fragMsdu");
    }
}

void Ieee80211MacPrepareMpdu::handleMsduRequest(Ieee80211MacSignalMsduRequest *msduRequest, Ieee80211NewFrame *frame)
{
    pri = msduRequest->getPriority();
    sdu = frame;
    if (state == PREPARE_MPDU_STATE_NO_BSS)
    {
        emitMsduConfirm(sdu, pri, TxStatus::TxStatus_noBss);
        state = PREPARE_MPDU_STATE_NO_BSS;
    }
    else if (state == PREPARE_MPDU_STATE_PREPARE_BSS)
    {
        if (!macsorts->getIntraMacRemoteVariables()->isAssoc())
            state = PREPARE_MPDU_STATE_NO_BSS;
        else
        {
            sdu->setAddr3(sdu->getAddr1());
            sdu->setAddr1(macsorts->getIntraMacRemoteVariables()->getBssId());
            sdu->setToDs(true);
            useWep = macmib->getStationConfigTable()->isDot11PrivacyOptionImplemented(); // TODO: dot11PrivacyInvoked
            fragment();
        }
    }
    else if (state == PREPARE_MDPU_STATE_PREPARE_IBSS)
    {
        if (!macsorts->getIntraMacRemoteVariables()->isIbss())
            state = PREPARE_MPDU_STATE_NO_BSS;
        useWep = false; // macmib->getStationConfigTable()->isDot11PrivacyOptionImplemented(); // TODO: dot11PrivacyInvoked
        fragment();
    }
    else if (state == PREPARE_MPDU_STATE_PREPARE_AP)
    {
        if (!macsorts->getIntraMacRemoteVariables()->isActingAsAp())
            state = PREPARE_MPDU_STATE_NO_BSS;
        else
        {
            useWep = macmib->getStationConfigTable()->isDot11PrivacyOptionImplemented(); // TODO: dot11PrivacyInvoked
            fragment();
        }
    }
}

void Ieee80211MacPrepareMpdu::fragment()
{
    fsdu->fTot = 1;
    fsdu->fCur = 0;
    fsdu->fAnc = 0;
    fsdu->eol = 0;
    fsdu->sqf = 0;
    fsdu->src = 0;
    fsdu->lrc = 0;
    fsdu->psm = false;
    fsdu->rate = bps(0);
    fsdu->grpa = sdu->getAddr1().isMulticast();
    fsdu->cf = pri;
//    fsdu.cnfTo = sender;
    fsdu->resume = false;
    mpduOvhd = Ieee80211MacNamedStaticIntDataValues::sMacHdrLng + Ieee80211MacNamedStaticIntDataValues::sCrcLng;
    pduSize = macmib->getOperationTable()->getDot11FragmentationThreshold();
    //TODO
    int sduLength = sdu->getByteLength();
    if (!fsdu->grpa && sduLength > pduSize)
    {
//        This is the typical case, with the length of all but the last fragment
//        equal to dot11FragmentationThreshold (plus sWepAddLng if useWep=true). The
//        value selected for pduSize must be >=256, even, and <=aMpduMaxLength.
        pduSize -= mpduOvhd;
        fsdu->fTot = (sduLength - Ieee80211MacNamedStaticIntDataValues::sMacHdrLng) / pduSize + ((sduLength - Ieee80211MacNamedStaticIntDataValues::sMacHdrLng % pduSize) ? 1 : 0);
    }
    else
        pduSize = sduLength - Ieee80211MacNamedStaticIntDataValues::sMacHdrLng;
    if (fsdu->fTot == 0)
        fsdu->fTot = 1;
    makePdus();
}

void Ieee80211MacPrepareMpdu::makePdus()
{
    int sduLength;
    f = 0;
    int p = Ieee80211MacNamedStaticIntDataValues::sMacHdrLng;
    fsdu->pdus[f] = nullptr;
    keyOk = false;

    while (f != fsdu->fTot)
    {
    //    fsdu!pdus(f):=
    //    fsdu!pdus(f) //
    //    substr(sdu,0,sMacHdrLng) //
    //    substr(sdu,p,
    //    pduSize)

    //    fsdu!pdus(f):=
    //    setFrag(
    //    fsdu!pdus(f),f)
        if (f+1 < fsdu->fTot)
        {
//            fsdu!pdus(f):=
//            setMoreFrag(
//            fsdu!pdus(f),1)
        }
        if (useWep)
        {
            // TODO
    //        Encrypt
    //        (fsdu!pdus(f),
    //        keyOk,import(dot11WepKey_
    //                Mappings),
    //                import(dot11WepKey_
    //                MappingLength),
    //                import(dot11Wep_
    //                DefaultKeys),
    //                import(dot11WepDefault_
    //                KeyId), import(mCap))
        }
        if (keyOk || !useWep)
        {
            f++;
            p += pduSize;
            if (p + pduSize > sduLength)
                pduSize = sduLength - p + 1;
            if (f == fsdu->fTot)
                emitFragRequest(fsdu);
        }
        else
        {
            emitMsduConfirm(sdu, pri, TxStatus::TxStatus_unavailableKeyMapping);
            break;
        }
    }
}

void Ieee80211MacPrepareMpdu::handleResetMac()
{
    state = PREPARE_MPDU_STATE_NO_BSS;
}

void Ieee80211MacPrepareMpdu::handleMmRequest(Ieee80211MacSignalMmRequest *mmRequest, Ieee80211NewFrame *frame)
{
    sdu = frame;
    pri = mmRequest->getPriority();
    // Mmpdus sent even when not in Bss/Ibss.
    bcmc = sdu->getAddr1().isMulticast();
// TODO    useWep:=dot11PrivacyOptionImplemented and if wepBit(sdu)=1 then true else false fi
    useWep = false;
    fragment();
}

void Ieee80211MacPrepareMpdu::handleFragConfirm(Ieee80211MacSignalFragConfirm *fragConfirm)
{
//    fsdu = fragConfirm->getFsdu();
    pri = fragConfirm->getPriority();
    rrsl = fragConfirm->getTxResult();
//    rsdu:= substr(fsdu!pdus(0), 0,sMacHdrLng),
//    pri = fsdu->cf;
//    rrsl = txResult;
//    if (basetype(fsdu->pdus[0]))
//    {
////        emitMmConfirm()
//    }
//    else
//        emitMsduConfirm(rsdu, pri, rrsl);
}

void Ieee80211MacPrepareMpdu::receiveSignal(cComponent* source, int signalID, cObject* obj)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);
    if (signalID == Ieee80211MacMacsorts::intraMacRemoteVariablesChanged)
    {
        switch (state)
        {
            case PREPARE_MPDU_STATE_NO_BSS:
                if (macsorts->getIntraMacRemoteVariables()->isAssoc() && !macsorts->getIntraMacRemoteVariables()->isActingAsAp())
                    state = PREPARE_MPDU_STATE_PREPARE_BSS;
                break;
                if (macsorts->getIntraMacRemoteVariables()->isIbss())
                    state = PREPARE_MDPU_STATE_PREPARE_IBSS;
                break;
                if (macsorts->getIntraMacRemoteVariables()->isActingAsAp())
                    state = PREPARE_MPDU_STATE_PREPARE_AP;
                break;
        }
    }
}

void Ieee80211MacPrepareMpdu::emitMmConfirm(cPacket *rsdu, TxStatus txStatus)
{
    Ieee80211MacSignalMmConfirm *mmConfirm = new Ieee80211MacSignalMmConfirm();
    mmConfirm->setStatus(txStatus);
    rsdu->setControlInfo(mmConfirm);
}

void Ieee80211MacPrepareMpdu::emitMsduConfirm(cPacket *sdu, CfPriority priority, TxStatus txStatus)
{
    Ieee80211MacSignalMsduConfirm *msduConfirm = new Ieee80211MacSignalMsduConfirm();
    msduConfirm->setPriority(priority);
    msduConfirm->setTxStatus(txStatus);
    sdu->setControlInfo(msduConfirm);
    send(sdu, msdu);
}

void Ieee80211MacPrepareMpdu::emitFragRequest(FragSdu *sdu)
{
//    Ieee80211MacSignalFragRequest *fragRequest = new Ieee80211MacSignalFragRequest();
//    fragRequest->setFsdu(sdu);
//    send(fragRequest, fragMsdu);
}

} /* namespace inet */
} /* namespace ieee80211 */

