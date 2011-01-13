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

#include "Utils.h"
#include "IntServ.h"

void removeDuplicates(std::vector<int>& vec)
{
    for (unsigned int i = 0; i < vec.size(); i++)
    {
        unsigned int j;
        for (j = 0; j < i; j++)
            if (vec[j] == vec[i])
                break;
        if (j < i)
        {
            vec.erase(vec.begin() + i);
            --i;
        }
    }
}

int find(const EroVector& ERO, IPAddress node)
{
    for (unsigned int i = 0; i < ERO.size(); i++)
        if (ERO[i].node == node)
            return i;
    ASSERT(false);
    return -1; // to prevent warning
}

bool find(std::vector<int>& vec, int value)
{
    for (unsigned int i = 0; i < vec.size(); i++)
        if (vec[i] == value)
            return true;
    return false;
}

bool find(const IPAddressVector& vec, IPAddress addr)
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
    while(msg->getEncapsulatedPacket())
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



