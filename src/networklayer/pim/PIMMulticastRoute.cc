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

Register_Abstract_Class(PIMMulticastRoute);

PIMMulticastRoute::PIMMulticastRoute(IPv4Address origin, IPv4Address group)
    : IPv4MulticastRoute(), RP(IPv4Address::UNSPECIFIED_ADDRESS), flags(0),
      grt(NULL), sat(NULL), srt(NULL), kat(NULL), rst(NULL), et(NULL), jt(NULL), ppt(NULL),
      sequencenumber(0)
{
    setMulticastGroup(group);
    setOrigin(origin);
    setOriginNetmask(origin.isUnspecified() ? IPv4Address::UNSPECIFIED_ADDRESS : IPv4Address::ALLONES_ADDRESS);
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

    PIMInInterface *inInterface = getPIMInInterface();
    out << "Incoming interface: " << (inInterface ? inInterface->getInterface()->getName() : "Null") << ", "
        << "RPF neighbor " << (inInterface->nextHop.isUnspecified() ? "0.0.0.0" : inInterface->nextHop.str()) << endl;

    out << "Outgoing interface list:" << endl;
    for (unsigned int k = 0; k < getNumOutInterfaces(); k++)
    {
        PIMMulticastRoute::PIMOutInterface *outInterface = getPIMOutInterface(k);
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

PIMMulticastRoute::PIMOutInterface *PIMMulticastRoute::findOutInterfaceByInterfaceId(int interfaceId)
{
    for (unsigned int i = 0; i < getNumOutInterfaces(); i++)
    {
        PIMOutInterface *outInterface = dynamic_cast<PIMOutInterface*>(getOutInterface(i));
        if (outInterface && outInterface->getInterfaceId() == interfaceId)
            return outInterface;
    }
    return NULL;
}

bool PIMMulticastRoute::isOilistNull()
{
    for (unsigned int i = 0; i < getNumOutInterfaces(); i++)
    {
        PIMOutInterface *outInterface = getPIMOutInterface(i);
        if (outInterface->forwarding == Forward)
            return false;
    }
    return true;
}
