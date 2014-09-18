//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include "inet/networklayer/rsvp_te/Utils.h"
#include "inet/networklayer/rsvp_te/IntServ.h"

namespace inet {

std::string vectorToString(const IPAddressVector& vec)
{
    return vectorToString(vec, ", ");
}

std::string vectorToString(const IPAddressVector& vec, const char *delim)
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

EroVector routeToEro(const IPAddressVector& rro)
{
    EroVector ero;

    for (unsigned int i = 0; i < rro.size(); i++) {
        EroObj_t hop;
        hop.L = false;
        hop.node = rro[i];
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

int find(const EroVector& ERO, IPv4Address node)
{
    for (unsigned int i = 0; i < ERO.size(); i++)
        if (ERO[i].node == node)
            return i;

    ASSERT(false);
    return -1;    // to prevent warning
}

bool find(std::vector<int>& vec, int value)
{
    for (unsigned int i = 0; i < vec.size(); i++)
        if (vec[i] == value)
            return true;

    return false;
}

bool find(const IPAddressVector& vec, IPv4Address addr)
{
    for (unsigned int i = 0; i < vec.size(); i++)
        if (vec[i] == addr)
            return true;

    return false;
}

void append(std::vector<int>& dest, const std::vector<int>& src)
{
    for (unsigned int i = 0; i < src.size(); i++)
        dest.push_back(src[i]);
}

cModule *getPayloadOwner(cPacket *msg)
{
    while (msg->getEncapsulatedPacket())
        msg = msg->getEncapsulatedPacket();

    if (msg->hasPar("owner"))
        return simulation.getModule(msg->par("owner"));
    else
        return NULL;
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

