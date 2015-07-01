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

#include "Ieee80211MacMib.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacMib);

void Ieee80211MacMib::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
            throw cRuntimeError("This module doesn't handle self messages");
    else
    {
        if (dynamic_cast<Ieee80211MacSignalMlmeResetRequest *>(msg->getControlInfo()))
            handleMlmeResetRequest(dynamic_cast<Ieee80211MacSignalMlmeResetRequest *>(msg->getControlInfo()));
        if (dynamic_cast<Ieee80211MacSignalMlmeGetRequest *>(msg->getControlInfo()))
            handleMlmeGetRequest(dynamic_cast<Ieee80211MacSignalMlmeGetRequest *>(msg->getControlInfo()));
        if (dynamic_cast<Ieee80211MacSignalMlmeSetRequest *>(msg->getControlInfo()))
            handleMlmeSetRequest(dynamic_cast<Ieee80211MacSignalMlmeSetRequest *>(msg->getControlInfo()));
        else
            throw cRuntimeError("Unknown incoming signal");
    }
}

void Ieee80211MacMib::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        macmib = getModuleFromPar<Ieee80211MacMacmibPackage>(par("macmibPackage"), this);
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);

        dot11ExcludeUnencrypted = par("excludeUnencrypted");
        dot11FragmentationThreshold = par("fragmentationThreshold");
//        std::vector<MACAddress> dot11GroupAddresses;
        dot11LongRetryLimit = par("longRetryLimit");
        dot11MaxReceiveLifetime = par("maxReceiveLifetime");
        dot11MaxTransmitMsduLifetime = par("maxTransmitMsduLifetime");
        dot11MediumOccupancyLimit = par("mediumOccupancyLimit");
        dot11PrivacyInvoked = par("privacyInvoked");
        mReceiveDTIMs = par("receiveDTIMs");
        dot11CfpPeriod = par("cfpPeriod");
        dot11CfpMaxDuration = par("cfpMaxDuration");
        dot11AuthenticationResponseTimeout = par("authenticationResponseTimeout");
        dot11RtsThreshold = par("rtsThreshold");
        dot11ShortRetryLimit = par("shortRetryLimit");
        dot11WepDefaultKeyId = par("wepDefaultKeyId");
        dot11CurrentChannelNumber = par("currentChannelNumber");
        dot11CurrentSet = par("currentSet");
        dot11CurrentPattern = par("currentPattern");
        dot11CurrentIndex = par("currentIndex");
        exportValuesOfAttributesDeclaredHere();
    }
}

void Ieee80211MacMib::handleMlmeResetRequest(Ieee80211MacSignalMlmeResetRequest* resetRequest)
{
    if (state == MAC_MIB_STATE_MIB_IDLE)
    {
        adr = resetRequest->getAddr();
        dflt = resetRequest->getDflt();
        if (dflt)
        {
// TODO:            Reset read-write attributes in the MAC
//            MIB. The write-only attributes in the
//            privacy group may also be reset.
//            If there is a (non-Mlme) means to alter
//            any of the read-only attribute values,
//            they must be restored to default values.
        }
// TODO:        A locally-administered MAC address
//        may be used in lieu of the unique,
//        globally-administered MAC address
//        assigned to the station. However, the
//        value of dot11MacAddress may not change
//        during MAC operation.

    }
    emitMlmeResetConfirm(MlmeStatus_succes);
    exportValuesOfAttributesDeclaredHere();
}

void Ieee80211MacMib::handleMlmeGetRequest(Ieee80211MacSignalMlmeGetRequest* getRequest)
{
    if (state == MAC_MIB_STATE_MIB_IDLE)
    {
        x = getRequest->getMibAtrib();
        MibAttribType mibAttribType = getMibAttribType(x); // TODO!!!!
        if (mibAttribType == MIB_ATTRIB_TYPE_VALID)
        {
            if (isDeclaredHere(x))
                v = getMibValue(x);
            else
                v = importMibValue(x);
            emitMlmeGetConfirm(MibStatus_succes, x, v); // TODO:
        }
        else if (mibAttribType == MIB_ATTRIB_TYPE_INVALID)
        {
            emitMlmeGetConfirm(MibStatus_invalid, x, v);
        }
    }

}

void Ieee80211MacMib::handleMlmeSetRequest(Ieee80211MacSignalMlmeSetRequest* setRequest)
{
    if (state == MAC_MIB_STATE_MIB_IDLE)
    {
        x = setRequest->getMibAtrib();
        v = setRequest->getMibValue();
        MibAttribType mibAttribType = getMibAttribType(x); // TODO!!!!
        if (mibAttribType == MIB_ATTRIB_TYPE_INVALID)
        {
            emitMlmeSetConfirm(MibStatus_invalid, x);
        }
        else if (mibAttribType == MIB_ATTRIB_TYPE_VALID)
        {
            setMibValue(x, v);
            exportMibValue(x);
            emitMlmeSetConfirm(MibStatus_read_only, x);
        }
    }
}

void Ieee80211MacMib::emitResetMac()
{
//    ResetMAC is sent to all processes
//    in all blocks. However, to reduce
//    clutter and enhance readability,
//    ResetMAC is omitted from signallists
//    and signal routes needed solely for
//    the ResetMAC signal are not shown.
    // TODO:
}

void Ieee80211MacMib::emitMlmeGetConfirm(MibStatus status, std::string mibAttrib, int mibValue)
{
    cMessage *mlmeGetConfirm = new cMessage("MlmeGetConfirm");
    Ieee80211MacSignalMlmeGetConfirm *signal = new Ieee80211MacSignalMlmeGetConfirm();
    signal->setMibStatus(status);
    signal->setMibAtrib(mibAttrib.c_str());
    signal->setMibValue(mibValue);
    mlmeGetConfirm->setControlInfo(signal);
    send(mlmeGetConfirm, "getSet$o");
}

void Ieee80211MacMib::emitMlmeSetConfirm(MibStatus status, std::string mibAttrib)
{
    cMessage *mlmeSetConfirm = new cMessage("MlmeSetConfirm");
    Ieee80211MacSignalMlmeSetConfirm *signal = new Ieee80211MacSignalMlmeSetConfirm();
    signal->setMibStatus(status);
    signal->setMibAtrib(mibAttrib.c_str());
    mlmeSetConfirm->setControlInfo(signal);
    send(mlmeSetConfirm, "getSet$o");
}

void Ieee80211MacMib::emitMlmeResetConfirm(MlmeStatus status)
{
    cMessage *mlmeResetConfirm = new cMessage("MlmeResetConfirm");
    Ieee80211MacSignalMlmeResetConfirm *signal = new Ieee80211MacSignalMlmeResetConfirm();
    signal->setMlmeStatus(status);
    mlmeResetConfirm->setControlInfo(signal);
    send(mlmeResetConfirm, "getSet$o");
}


Ieee80211MacMib::MibAttribType Ieee80211MacMib::getMibAttribType(std::string mibAttrib) const
{
    // TODO:
    return MIB_ATTRIB_TYPE_VALID;
}

bool Ieee80211MacMib::isDeclaredHere(std::string mibAttrib) const
{
    // TODO:
    return true;
}

int Ieee80211MacMib::importMibValue(std::string mibAttrib) const
{
    // TODO:
    return 0;
}

int Ieee80211MacMib::getMibValue(std::string mibAttrib) const
{
    // TODO
    return 0;
}

void Ieee80211MacMib::exportMibValue(std::string mibAttrib) const
{
    // TODO
}

void Ieee80211MacMib::setMibValue(std::string mibAttrib, int mibValue)
{
    // TODO
}

void Ieee80211MacMib::exportValuesOfAttributesDeclaredHere()
{
    // TODO: incomplete
    macmib->getOperationTable()->setDot11RtsThreshold(dot11RtsThreshold);
    macmib->getOperationTable()->setDot11ShortRetryLimit(dot11ShortRetryLimit);
    macmib->getOperationTable()->setDot11FragmentationThreshold(dot11FragmentationThreshold);
    macmib->getOperationTable()->setDot11LongRetryLimit(dot11LongRetryLimit);
    macmib->getOperationTable()->setDot11MaxReceiveLifetime(dot11MaxReceiveLifetime);
    macmib->getOperationTable()->setDot11MaxTransmitMsduLifetime(dot11MaxTransmitMsduLifetime);

    macmib->getStationConfigTable()->setDot11CfpPeriod(dot11CfpPeriod);
    macmib->getStationConfigTable()->setDot11CfpMaxDuration(dot11CfpMaxDuration);
    macmib->getStationConfigTable()->setDot11AuthenticationResponseTimeout(dot11AuthenticationResponseTimeout);

    macsorts->getIntraMacRemoteVariables()->setReceiveDtiMs(mReceiveDTIMs);
}

} /* namespace inet */
} /* namespace ieee80211 */

