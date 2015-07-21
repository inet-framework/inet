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

#include "Ieee80211MacMsduToLlc.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacMsduToLlc);

void Ieee80211MacMsduToLlc::processSignal(cMessage *msg)
{
    if (dynamic_cast<Ieee80211MacSignalMsduIndicate *>(msg->getControlInfo()))
        handleMsduIndicate(dynamic_cast<Ieee80211MacSignalMsduIndicate *>(msg->getControlInfo()), dynamic_cast<Ieee80211NewFrame *>(msg));
}

void Ieee80211MacMsduToLlc::handleMsduIndicate(Ieee80211MacSignalMsduIndicate* msduIndicate, Ieee80211NewFrame *frame)
{
    // From source of the RSDU channel.
    // STA source is Protocol Control,
    // AP source is Distribution Service.
    if (state == MSDU_TO_LLC_STATE_TO_LLC)
    {
        da = sdu->getAddr1();
        sa = sdu->getFrDs() ? sdu->getAddr3() : sdu->getAddr2();
        srv = sdu->getOrderBit() ? ServiceClass_strictlyOrdered : ServiceClass_reordable;
        // Remove MAC header from beginning of
        // MSDU to obtain the LLC data octet string.
//      TODO:  LLCdata:= substr(sdu, sMacHdrLng,length(sdu) - sMacHdrLng)
        // Reception status always successful
        // because any error would prevent the
        // MsduIndicate from reaching this process.
        emitMaUnitDataIndication(sa, da, Routing_null_rt, llcData, RxStatus_rx_success, cf, srv);

    }
}

void Ieee80211MacMsduToLlc::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        auto processSignalFunction = [=] (cMessage *m) { processSignal(m); };
        sdlProcess = new SdlProcess(processSignalFunction,
                                    {{MSDU_TO_LLC_STATE_START,
                                      {},
                                     {}},
                                      {MSDU_TO_LLC_STATE_TO_LLC,
                                      {MSDU_INDICATE},
                                      {}}
                                    });
        state = MSDU_TO_LLC_STATE_TO_LLC;
        sdlProcess->setCurrentState(state);
    }
}

void Ieee80211MacMsduToLlc::emitMaUnitDataIndication(MACAddress sa, MACAddress da, Routing routing, cPacket* llcData, RxStatus rxStat, CfPriority cf, ServiceClass srv)
{
    Ieee80211MacSignalMaUnitDataIndication *signal = new Ieee80211MacSignalMaUnitDataIndication();
    signal->setSa(sa);
    signal->setDa(da);
    signal->setRouting(routing);
    signal->setRxStat(rxStat);
    signal->setCf(cf);
    signal->setSrv(srv);
    createSignal(llcData, signal);
    send(llcData, "toLlc$o");
}

} /* namespace inet */
} /* namespace ieee80211 */

