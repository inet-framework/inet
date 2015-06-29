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

#include "Ieee80211MacMsduFromLlc.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacMsduFromLlc);

void Ieee80211MacMsduFromLlc::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);
        macmib = getModuleFromPar<Ieee80211MacMacmibPackage>(par("macmibPackage"), this);
        subscribe(Ieee80211MacMacsorts::intraMacRemoteVariablesChanged, this);
    }
}

void Ieee80211MacMsduFromLlc::handleMessage(cMessage* msg)
{
    if (dynamic_cast<Ieee80211MacSignalMaUnitDataRequest *>(msg->getControlInfo()))
        handleMaUnitDataRequest(dynamic_cast<Ieee80211MacSignalMaUnitDataRequest *>(msg->getControlInfo()), dynamic_cast<cPacket *>(msg));
}

void Ieee80211MacMsduFromLlc::handleMaUnitDataRequest(Ieee80211MacSignalMaUnitDataRequest *signal, cPacket *frame)
{
    if (state == MSDU_FROM_LCC_STATE_FROM_LLC)
    {
        da = signal->getDestinationAddress();
        sa = signal->getSenderAddres();
        rt = signal->getRouting();
        llcData = frame;
        srv = signal->getServiceClass();
        cf = signal->getPriority();

        // TODO: validate parameters??
        if (rt != Routing_null_rt)
            stat = TxStatus_nonNullSourceRouting;
        else if (llcData->getByteLength() > Ieee80211MacNamedStaticIntDataValues::sMaxMsduLng || llcData->getByteLength() < 0)
            stat = TxStatus_excessiveDataLength;
        else
            stat = TxStatus_successful;

        if (stat == TxStatus_successful)
        {
            if (srv == ServiceClass_strictlyOrdered)
            {
                if (macmib->getStationConfigTable()->getDot11PowerMangementMode() == PsMode_sta_active)
                {
                    if (macsorts->getIntraMacRemoteVariables()->isDisable())
                    {
                        stat = TxStatus_noBss;
                        emitMaUnitDataStatusIndication(sa, da, stat, cf, srv);
                    }
                    else
                    {
                        if (cf == CfPriority_contention)
                            makeMsdu();
                        else if (cf == CfPriority_contentionFree)
                        {
                            if (macsorts->getIntraMacRemoteVariables()->isPcAvail())
                                makeMsdu();
                            else
                            {
                                emitMaUnitDataStatusIndication(sa, da, TxStatus_unavailablePriority, cf, srv);
                                cf = CfPriority_contention;
                                makeMsdu();
                            }

                        }
                        else
                        {
                            stat = TxStatus_unsupportedPriority;
                            emitMaUnitDataStatusIndication(sa, da, stat, cf, srv);
                        }
                    }
                }
                else
                {
                    stat = TxStatus_unavailableServiceClass;
                    emitMaUnitDataStatusIndication(sa, da, stat, cf, srv);
                }
            }
            else
            {
                stat = TxStatus_unsupportedServiceClass;
                emitMaUnitDataStatusIndication(sa, da, stat, cf, srv);
            }
        }
        else
            emitMaUnitDataStatusIndication(sa, da, stat, cf, srv);
    }
}

void Ieee80211MacMsduFromLlc::handleMsduConfirm(Ieee80211MacSignalMsduConfirm *signal)
{
//    srv:= if
//    orderBit
//    (sdu) = 1 then
//    strictlyOrdered
//    else reorderable fi
//    da:= if
//    toDs(sdu) = 1
//    then addr3(sdu)
//    else addr1(sdu)
//    fi
//    emitMaUnitDataStatusIndication(addr2(sdu),da,stat,cf,srv);
}

void Ieee80211MacMsduFromLlc::emitMaUnitDataStatusIndication(MACAddress sa, MACAddress da, TxStatus stat, CfPriority cf,
        ServiceClass srv)
{
}

void Ieee80211MacMsduFromLlc::makeMsdu()
{
//    Build frame with 24-octet
//    MAC header and LLCdata:
//    ftype:= data
//    toDS := 0
//    addr1:= da
//    addr2:= dot11MacAddress
//    (sa parameter not used)
//    addr3:= mBssId
//    <other header fields> := 0
    Ieee80211NewFrame *sdu = new Ieee80211NewFrame();
    sdu->setFtype(TypeSubtype_data);
    sdu->setAddr1(da);
    sdu->setAddr2(macmib->getOperationTable()->getDot11MacAddress());
    sdu->setAddr3(macsorts->getIntraMacRemoteVariables()->getBssId());
    sdu->setByteLength(24);
    if (srv == ServiceClass_strictlyOrdered)
        sdu->setOrderBit(true);
//    Send Msdu to Mpdu preparation
//    (to distribution service at AP)
//    with basic header. Other fields are
//    filled in prior to transmission
    emitMsduRequest(sdu, cf);
}

void Ieee80211MacMsduFromLlc::emitMsduRequest(Ieee80211NewFrame* sdu, CfPriority priority)
{
    Ieee80211MacSignalMsduRequest *signal = new Ieee80211MacSignalMsduRequest();
    signal->setPriority(priority);
    sdu->setControlInfo(signal);
    send(sdu, "txMsdu$o");
}

} /* namespace inet */
} /* namespace ieee80211 */
