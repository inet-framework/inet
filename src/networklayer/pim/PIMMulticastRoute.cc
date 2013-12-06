//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#include "IIPv4RoutingTable.h"
#include "PIMMulticastRoute.h"

Register_Class(PIMMulticastRoute);

PIMMulticastRoute::PIMMulticastRoute()
    : IPv4MulticastRoute(), RP(IPv4Address::UNSPECIFIED_ADDRESS), flags(0)
{
    grt = NULL;
    sat = NULL;
    srt = NULL;
    rst = NULL;
    kat = NULL;
    et = NULL;
    jt = NULL;
    ppt = NULL;

    sequencenumber = 0;
}

// Format is same as format on Cisco routers.
std::string PIMMulticastRoute::info() const
{
    std::stringstream out;
    out << "(" << (getOrigin().isUnspecified() ? "*" : getOrigin().str()) << ", " << getMulticastGroup()
        << "), ";
    if (getOrigin().isUnspecified() && !getRP().isUnspecified())
        out << "RP is " << getRP() << ", ";
    out << "flags: ";
    if (isFlagSet(D)) out << "D";
    if (isFlagSet(S)) out << "S";
    if (isFlagSet(C)) out << "C";
    if (isFlagSet(P)) out << "P";
    if (isFlagSet(A)) out << "A";
    if (isFlagSet(F)) out << "F";
    if (isFlagSet(T)) out << "T";

    out << endl;

    out << "Incoming interface: " << (getInIntPtr() ? getInIntPtr()->getName() : "Null") << ", "
        << "RPF neighbor " << (getInIntNextHop().isUnspecified() ? "0.0.0.0" : getInIntNextHop().str()) << endl;

    out << "Outgoing interface list:" << endl;
    for (unsigned int k = 0; k < getNumOutInterfaces(); k++)
    {
        PIMMulticastRoute::AnsaOutInterface *outInterface = getAnsaOutInterface(k);
        if ((outInterface->mode == PIMMulticastRoute::Sparsemode && outInterface->shRegTun)
                || outInterface->mode ==PIMMulticastRoute:: Densemode)
        {
            out << outInterface->getInterface()->getName() << ", "
                << (outInterface->forwarding == Forward ? "Forward/" : "Pruned/")
                << (outInterface->mode == Densemode ? "Dense" : "Sparse")
                << endl;
        }
        else
            out << "Null" << endl;
    }

    if (getNumOutInterfaces() == 0)
        out << "Null" << endl;

    return out.str();
}

void PIMMulticastRoute::setRegStatus(int interfaceId, RegisterState regState)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->interfaceId == interfaceId)
        {
            outInterface->regState = regState;
            break;
        }

    }
}

PIMMulticastRoute::RegisterState PIMMulticastRoute::getRegStatus(int interfaceId)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->interfaceId == interfaceId)
            break;
    }
    return i < getNumOutInterfaces() ? getAnsaOutInterface(i)->regState : NoInfoRS;
}

int PIMMulticastRoute::getOutIdByIntId(int interfaceId)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->interfaceId == interfaceId)
            break;
    }
    return i; // FIXME return -1 if not found
}

bool PIMMulticastRoute::outIntExist(int interfaceId)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->interfaceId == interfaceId)
            return true;
    }
    return false;
}

bool PIMMulticastRoute::isOilistNull()
{
    bool olistNull = true;
    for (unsigned int i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->forwarding == Forward)
        {
            olistNull = false;
            break;
        }
    }
    return olistNull;
}

void PIMMulticastRoute::addOutIntFull(InterfaceEntry *ie, int interfaceId, InterfaceState forwading, InterfaceState mode, PIMpt *pruneTimer,
                                                PIMet *expiryTimer, AssertState assert, RegisterState regState, bool show)
{
    AnsaOutInterface *outIntf = new AnsaOutInterface(ie);

    outIntf->interfaceId = interfaceId;
    outIntf->forwarding = forwading;
    outIntf->mode = mode;
    outIntf->pruneTimer = NULL;
    outIntf->pruneTimer = pruneTimer;
    outIntf->expiryTimer = expiryTimer;
    outIntf->regState = regState;
    outIntf->assert = assert;
    outIntf->shRegTun = show;

    addOutInterface(outIntf);
}

void PIMMulticastRoute::setAddresses(IPv4Address multOrigin, IPv4Address multGroup, IPv4Address RP)
{
    this->RP = RP;
    this->setMulticastGroup(multGroup);
    this->setOrigin(multOrigin);
    this->setOriginNetmask(multOrigin.isUnspecified() ? IPv4Address::UNSPECIFIED_ADDRESS : IPv4Address::ALLONES_ADDRESS);
}
