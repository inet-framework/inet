//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/rsvpte/Utils.h"

#include "inet/networklayer/rsvpte/IntServ_m.h"

namespace inet {

std::string vectorToString(const Ipv4AddressVector& vec)
{
    return vectorToString(vec, ", ");
}

std::string vectorToString(const Ipv4AddressVector& vec, const char *delim)
{
    std::ostringstream stream;
    for (unsigned int i = 0; i < vec.size(); i++) {
        stream << vec[i];
        if (i < vec.size() - 1)
            stream << delim;
    }
    stream << std::flush;
    std::string str(stream.str());
    return str;
}

std::string vectorToString(const EroVector& vec)
{
    return vectorToString(vec, ", ");
}

std::string vectorToString(const EroVector& vec, const char *delim)
{
    std::ostringstream stream;
    for (unsigned int i = 0; i < vec.size(); i++) {
        stream << vec[i].node;

        if (i < vec.size() - 1)
            stream << delim;
    }
    stream << std::flush;
    std::string str(stream.str());
    return str;
}

EroVector routeToEro(const Ipv4AddressVector& rro)
{
    EroVector ero;

    for (auto& elem : rro) {
        EroObj hop;
        hop.L = false;
        hop.node = elem;
        ero.push_back(hop);
    }

    return ero;
}

void removeDuplicates(std::vector<int>& vec)
{
    for (unsigned int i = 0; i < vec.size(); i++) {
        unsigned int j;
        for (j = 0; j < i; j++)
            if (vec[j] == vec[i])
                break;

        if (j < i) {
            vec.erase(vec.begin() + i);
            --i;
        }
    }
}

void append(std::vector<int>& dest, const std::vector<int>& src)
{
    for (auto& elem : src)
        dest.push_back(elem);
}

cModule *getPayloadOwner(cPacket *msg)
{
    while (msg->getEncapsulatedPacket())
        msg = msg->getEncapsulatedPacket();

    if (msg->hasPar("owner"))
        return cSimulation::getActiveSimulation()->getModule(msg->par("owner"));
    else
        return nullptr;
}

/*
   void prepend(EroVector& dest, const EroVector& src, bool reverse)
   {
    ASSERT(dest.size() > 0);
    ASSERT(src.size() > 0);

    int size = src.size();
    for (unsigned int i = 0; i < size; i++)
    {
        int n = reverse? i: size - 1 - i;

        if (dest[0] == src[n])
            continue;

        dest.insert(dest.begin(), src[n]);
    }
   }
 */

} // namespace inet

