/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdexcept>
#include "inet/routing/extras/dymo_fau/DYMO_OutstandingRREQList.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {

namespace inetmanet {

std::ostream& operator<<(std::ostream& os, const DYMO_OutstandingRREQ& o)
{
    os << "[ ";
    os << "destination: " << o.destAddr << ", ";
    os << "tries: " << o.tries << ", ";
    os << "wait time: " << *o.wait_time << ", ";
    os << "creationTime: " << o.creationTime;
    os << " ]";

    return os;
}

DYMO_OutstandingRREQList::DYMO_OutstandingRREQList()
  : host(nullptr)
{
}

DYMO_OutstandingRREQList::~DYMO_OutstandingRREQList()
{
    delAll();
}

const char* DYMO_OutstandingRREQList::getFullName() const
{
    return "DYMO_OutstandingRREQList";
}

std::string DYMO_OutstandingRREQList::info() const
{
    std::ostringstream ss;

    int total = outstandingRREQs.size();
    ss << total << " outstanding RREQs: ";

    ss << "{" << std::endl;
    for (std::vector<DYMO_OutstandingRREQ*>::const_iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++)
    {
        DYMO_OutstandingRREQ* e = *iter;
        ss << "  " << *e << std::endl;
    }
    ss << "}";

    return ss.str();
}

std::string DYMO_OutstandingRREQList::detailedInfo() const
{
    return info();
}

DYMO_OutstandingRREQ* DYMO_OutstandingRREQList::getByDestAddr(unsigned int destAddr, int prefix)
{
    for (auto iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++)
    {
        if (IPv4Address(destAddr).prefixMatches(IPv4Address((*iter)->destAddr), prefix)) return *iter;
    }
    return nullptr;
}

DYMO_OutstandingRREQ* DYMO_OutstandingRREQList::getExpired()
{
    for (auto iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++)
    {
        if ((*iter)->wait_time->stopWhenExpired()) return *iter;
    }
    return nullptr;
}

bool DYMO_OutstandingRREQList::hasActive() const
{
    for (std::vector<DYMO_OutstandingRREQ*>::const_iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++)
    {
        if ((*iter)->wait_time->isActive())
            return true;
    }
    return false;
}

void DYMO_OutstandingRREQList::add(DYMO_OutstandingRREQ* outstandingRREQ)
{
    outstandingRREQs.push_back(outstandingRREQ);
}

void DYMO_OutstandingRREQList::del(DYMO_OutstandingRREQ* outstandingRREQ)
{
    auto iter = outstandingRREQs.begin();
    while (iter != outstandingRREQs.end())
    {
        if ((*iter) == outstandingRREQ)
        {
            (*iter)->wait_time->cancel();
            delete (*iter)->wait_time;
            delete (*iter);
            iter = outstandingRREQs.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void DYMO_OutstandingRREQList::delAll()
{
    auto iter = outstandingRREQs.begin();
    while (iter != outstandingRREQs.end())
    {
        (*iter)->wait_time->cancel();
        delete (*iter)->wait_time;
        delete (*iter);
        iter = outstandingRREQs.erase(iter);
    }
}

std::ostream& operator<<(std::ostream& os, const DYMO_OutstandingRREQList& o)
{
    os << o.info();
    return os;
}

} // namespace inetmanet

} // namespace inet

