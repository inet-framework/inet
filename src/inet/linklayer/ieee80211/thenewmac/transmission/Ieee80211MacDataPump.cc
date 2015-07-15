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

#include "inet/linklayer/ieee80211/thenewmac/transmission/Ieee80211MacDataPump.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacDataPump);

void Ieee80211MacDataPump::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        if (strcmp("ResetMAC", msg->getName()) == 0)
            handleResetMac();
        else
            throw cRuntimeError("Unknown self message", msg->getName());
    }
    else
    {
        EV_DEBUG << msg->getName() << " signal arrived" << endl;
        if (dynamic_cast<Ieee80211MacSignalTxRequest *>(msg->getControlInfo()))
            handleTxRequest(dynamic_cast<Ieee80211NewFrame *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalBusy *>(msg->getControlInfo()))
            handleBusy();
        else if (dynamic_cast<Ieee80211MacSignalIdle *>(msg->getControlInfo()))
            handleIdle();
        else if (dynamic_cast<Ieee80211MacSignalSlot *>(msg->getControlInfo()))
            handleSlot();
        else if (dynamic_cast<Ieee80211MacSignalPhyTxStartConfirm *>(msg->getControlInfo()))
            handlePhyTxStartConfirm();
        else if (dynamic_cast<Ieee80211MacSignalPhyTxEndConfirm *>(msg->getControlInfo()))
            handlePhyTxEndConfirm();
        else if (dynamic_cast<Ieee80211MacSignalPhyDataConfirm *>(msg->getControlInfo()))
            handlePhyDataConfirm();
        else
            throw cRuntimeError("Unknown signal", msg->getControlInfo()->getName());
    }
}

void Ieee80211MacDataPump::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsModule"), this);
    }
    if (stage == INITSTAGE_LINK_LAYER)
    {
        state = DATA_PUMP_STATE_TX_IDLE;
        source = nullptr;
    }
}

void Ieee80211MacDataPump::handleBusy()
{
    emitBusy();
}

void Ieee80211MacDataPump::handleIdle()
{
    emitIdle();
}

void Ieee80211MacDataPump::handleSlot()
{
    emitSlot();
}

void Ieee80211MacDataPump::handleTxRequest(Ieee80211NewFrame *frame)
{
//    handlePdu(check_and_cast<cPacket *>(txRequest->getEncapsulatedPacket()));
//    source = txRequest->getArrivalGate();
    pdu = frame;
    k = 0;
    // TODO: fcs = initCrc
//    Plcp length is Mpdu length + Fcs length
    txLength = pdu->getByteLength(); // TODO: + fcs
//    Indicate medium busy to freeze backoff count during transmit.
    emitBusy();
//  TODO: emit PhyTxStart._request signal
    state = DATA_PUMP_STATE_WAIT_TX_START;
    //sendOneByte(); TODO: serializer
    // TODO: hack
    take(pdu);
    send(pdu, "toPhy$o");
}

void Ieee80211MacDataPump::emitBusy()
{
    Ieee80211MacSignalBusy *signal = new Ieee80211MacSignalBusy();
    cMessage *busy = createSignal("Busy", signal);
    send(busy, "fwdCs");
}

void Ieee80211MacDataPump::emitIdle()
{
    Ieee80211MacSignalIdle *signal = new Ieee80211MacSignalIdle();
    cMessage *idle = createSignal("Idle", signal);
    send(idle, "fwdCs");
}

void Ieee80211MacDataPump::handleResetMac()
{
    // TODO: emit PhyTxEnd._request signal
    state = DATA_PUMP_STATE_TX_IDLE;
}

void Ieee80211MacDataPump::handlePhyTxStartConfirm()
{
    sendOneByte();
}

void Ieee80211MacDataPump::sendOneByte()
{
    // TODO: emit PhyData._request(pdu(k))
    // TODO: fcs = crc32(fcs, pdu(k))
    k++;
    if (k == txLength)
    {
        k = 0;
        // Send the 1's complement of calculated FCS value, MSb to LSb.
        // TODO: fcs = mirror(not(fcs));
        state = DATA_PUMP_STATE_SEND_CRC;
    }
    else
    {
        if ((getFrameType() == ST_PROBERESPONSE || getFrameType() == ST_BEACON) &&
            k == Ieee80211MacNamedStaticIntDataValues::sTsOctet)
        {
            // Start of time stamp in beacon and probe_rsp.
            state = DATA_PUMP_STATE_INSERT_TIMESTAMP;
        }
        else
        {
            state = DATA_PUMP_STATE_SEND_FRAME;
            // TODO:
        }
    }
}

void Ieee80211MacDataPump::handlePhyDataConfirm()
{
    if (state == DATA_PUMP_STATE_SEND_CRC)
        sendOneCrcByte(); // TODO: wrong name
    else if (state == DATA_PUMP_STATE_SEND_FRAME)
        sendOneByte();
    else if (state == DATA_PUMP_STATE_INSERT_TIMESTAMP)
        insertTimestamp(); // TODO: wrong name
    // else: No save symbol => ignore
}

void Ieee80211MacDataPump::sendOneCrcByte()
{
    if (k == Ieee80211MacNamedStaticIntDataValues::sCrcLng)
    {
        // TODO: emit PhyTxEnd.request
        state = DATA_PUMP_STATE_WAIT_TX_END;
    }
    else
    {
        // TODO: emit PhyData.request(fcs(k))
        k++;
        state = DATA_PUMP_STATE_SEND_CRC;
    }

}

void Ieee80211MacDataPump::insertTimestamp()
{
    // At confirm of octet 23, insert TSF + Phy Tx delay into [24:31]
    // of beacon or probe rsp.
    // TODO: pdu:=setTs(pdu,call Tsf(0,false)+dTx)
    sendOneByte();
}

int Ieee80211MacDataPump::getFrameType() const
{
    return pdu->getFtype();
}

//void Ieee80211MacDataPump::handlePdu(const cPacket* pdu)
//{
//    if (dynamic_cast<const RawPacket *>(pdu))
//    {
//        rawPdu = static_cast<const RawPacket *>(pdu);
//        // TODO: this->pdu = deserialize(pdu)
//    }
//    else
//    {
//        this->pdu = check_and_cast<const Ieee80211Frame *>(pdu);
//        // TODO: rawPdu = serialize(pdu)
//    }
//}

void Ieee80211MacDataPump::handlePhyTxEndConfirm()
{
    // TxConfirm goes to process that sent TxRequest.
    // TODO: emit TxConfirm to source
    state = DATA_PUMP_STATE_TX_IDLE;
}

void Ieee80211MacDataPump::emitSlot()
{
    Ieee80211MacSignalSlot *signal = new Ieee80211MacSignalSlot();
    cMessage *slot = createSignal("Slot", signal);
    send(slot, "fwdCs");
}

Ieee80211MacDataPump::~Ieee80211MacDataPump()
{
    // TODO: pdu
}

void Ieee80211MacDataPump::emitPhyTxStartRequest(int length, bps rate)
{
}

} /* namespace ieee80211 */
} /* namespace inet */

