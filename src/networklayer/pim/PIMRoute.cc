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

#include "PIMRoute.h"

Register_Class(PIMMulticastRoute);

PIMMulticastRoute::PIMMulticastRoute()
{
    grt = NULL;
    sat = NULL;
    srt = NULL;
    rst = NULL;
    kat = NULL;
    et = NULL;
    jt = NULL;
    ppt = NULL;

    RP = IPv4Address::UNSPECIFIED_ADDRESS;
    sequencenumber = 0;

    this->setRoutingTable(NULL);
    this->setInInterface(NULL);
    this->setSourceType(MANUAL);
    this->setMetric(0);
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
    for (unsigned int j = 0; j < flags.size(); j++)
    {
        switch(flags[j])
        {
            case PIMMulticastRoute::D: out << "D"; break;
            case PIMMulticastRoute::S: out << "S"; break;
            case PIMMulticastRoute::C: out << "C"; break;
            case PIMMulticastRoute::P: out << "P"; break;
            case PIMMulticastRoute::A: out << "A"; break;
            case PIMMulticastRoute::F: out << "F"; break;
            case PIMMulticastRoute::T: out << "T"; break;
            case PIMMulticastRoute::NO_FLAG: break;
        }
    }
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

bool PIMMulticastRoute::isFlagSet(flag fl)
{
    for(unsigned int i = 0; i < flags.size(); i++)
    {
        if (flags[i] == fl)
            return true;
    }
    return false;
}

void PIMMulticastRoute::addFlag(flag fl)
{
    if (!isFlagSet(fl))
        flags.push_back(fl);
}

void PIMMulticastRoute::removeFlag(flag fl)
{
    for(unsigned int i = 0; i < flags.size(); i++)
    {
        if (flags[i] == fl)
        {
            flags.erase(flags.begin() + i);
            return;
        }
    }
}

void PIMMulticastRoute::setRegStatus(int intId, RegisterState regState)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->intId == intId)
        {
            outInterface->regState = regState;
            break;
        }

    }
}

PIMMulticastRoute::RegisterState PIMMulticastRoute::getRegStatus(int intId)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->intId == intId)
            break;
    }
    return i < getNumOutInterfaces() ? getAnsaOutInterface(i)->regState : NoInfoRS;
}

int PIMMulticastRoute::getOutIdByIntId(int intId)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->intId == intId)
            break;
    }
    return i; // FIXME return -1 if not found
}

bool PIMMulticastRoute::outIntExist(int intId)
{
    unsigned int i;
    for (i = 0; i < getNumOutInterfaces(); i++)
    {
        AnsaOutInterface *outInterface = getAnsaOutInterface(i);
        if (outInterface->intId == intId)
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

void PIMMulticastRoute::addOutIntFull(InterfaceEntry *intPtr, int intId, intState forwading, intState mode, PIMpt *pruneTimer,
                                                PIMet *expiryTimer, AssertState assert, RegisterState regState, bool show)
{
    AnsaOutInterface *outIntf = new AnsaOutInterface(intPtr);

    outIntf->intId = intId;
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

void PIMMulticastRoute::addFlags(flag fl1, flag fl2, flag fl3,flag fl4)
{
    if (fl1 != NO_FLAG && !isFlagSet(fl1))
        flags.push_back(fl1);
    if (fl2 != NO_FLAG && !isFlagSet(fl2))
        flags.push_back(fl2);
    if (fl3 != NO_FLAG && !isFlagSet(fl3))
        flags.push_back(fl3);
    if (fl4 != NO_FLAG && !isFlagSet(fl4))
        flags.push_back(fl4);
}

void PIMMulticastRoute::setAddresses(IPv4Address multOrigin, IPv4Address multGroup, IPv4Address RP)
{
    this->RP = RP;
    this->setMulticastGroup(multGroup);
    this->setOrigin(multOrigin);
    this->setOriginNetmask(multOrigin.isUnspecified() ? IPv4Address::UNSPECIFIED_ADDRESS : IPv4Address::ALLONES_ADDRESS);
}
