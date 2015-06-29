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

#include "Ieee80211MacDistributeMmpdusService.h"

namespace inet {
namespace ieee80211 {

void Ieee80211MacDistributeMmpdusService::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {

    }
}


void Ieee80211MacDistributeMmpdusService::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {

    }
    else
    {
        if (dynamic_cast<Ieee80211MacSignalXport *>(msg->getControlInfo()))
            handleXport();
        else if (dynamic_cast<Ieee80211MacSignalSst *>(msg->getControlInfo()))
            handleSst(dynamic_cast<Ieee80211MacSignalSst *>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalMmConfirm *>(msg->getControlInfo()))
            handleMmConfirm(dynamic_cast<Ieee80211MacSignalMmConfirm *>(msg->getControlInfo()), dynamic_cast<cPacket *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalMmIndicate *>(msg->getControlInfo()))
            handleMmIndicate(dynamic_cast<Ieee80211MacSignalMmIndicate *>(msg->getControlInfo()));
    }
}

void Ieee80211MacDistributeMmpdusService::handleXport()
{

}

void Ieee80211MacDistributeMmpdusService::handleSst(Ieee80211MacSignalSst* sst)
{
//    mAdr = sst->getAddr();
//    mSst = sst->getStationState();
//    emitStaState();
}

void Ieee80211MacDistributeMmpdusService::handleSend(Ieee80211MacSignalSend* send, cPacket *frame)
{
    mSpdu = frame;
    mIm = send->getPriority();
//    The selection criteria for Mmpdu Tx data rate are
//    not specified. Frames to group addresses must
//    use one of the basic rates. Requests should use one of
//    the basic rates unless the operational rates of the
//    recipient station are known. Responses must use a basic
//    rate or the rate at which the request was received.
    //emitMmRequest();
}

void Ieee80211MacDistributeMmpdusService::handleMmConfirm(Ieee80211MacSignalMmConfirm* mmConfirm, cPacket *frame)
{
    mSpdu = frame;
//    if (ftype(mSpdu) == beacon || ftype(mSpdu) == probe_rsp)
//    {
//        emitSent();
//    }
}

void Ieee80211MacDistributeMmpdusService::handleMmIndicate(Ieee80211MacSignalMmIndicate* mmIndicate)
{
}

void Ieee80211MacDistributeMmpdusService::reExport()
{
    //TODO: export intra-mac remote variables
}

void Ieee80211MacDistributeMmpdusService::emitStaState(MACAddress addr, StationState stationState)
{
}

void Ieee80211MacDistributeMmpdusService::emitMmRequest(cPacket* mspdu, CfPriority mim, bps mRate)
{
}

void Ieee80211MacDistributeMmpdusService::emitSent(cPacket* mspdu, TxStatus txStatus)
{
}

void Ieee80211MacDistributeMmpdusService::emitCls2Err(MACAddress addr)
{
}

void Ieee80211MacDistributeMmpdusService::emitCls3Err(MACAddress addr)
{
}

void Ieee80211MacDistributeMmpdusService::chkSigtype()
{
    if (mSerr == StateErr::StateErr_class2)
    {
//        emitCls2Err();
    }
    else if (mSerr == StateErr::StateErr_class3)
    {
//        emitCls3Err();
    }
    else
    {
//        if (ftype(mRpdu) == cfend, cfendack)
//        {
//        }
    }
}

} /* namespace inet */
} /* namespace ieee80211 */

