//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/networklayer/ipv4/ipsec/SecurityPolicyDatabase.h"

namespace inet {
namespace ipsec {

Define_Module(SecurityPolicyDatabase);

std::ostream& operator<<(std::ostream& os, const SecurityPolicy& e)
{
    os << e.str();
    return os;
}

SecurityPolicyDatabase::~SecurityPolicyDatabase()
{
    for (SecurityPolicy *entry : entries)
        delete entry;
}

void SecurityPolicyDatabase::initialize()
{
    WATCH_PTRVECTOR(entries);
}

SecurityPolicy *SecurityPolicyDatabase::findEntry(IPsecRule::Direction direction, PacketInfo *packet)
{
    for (SecurityPolicy *entry : entries)
        if (entry->getDirection() == direction && entry->getSelector().matches(packet))
            return entry;

    return nullptr;
}

void SecurityPolicyDatabase::addEntry(SecurityPolicy *entry)
{
    entries.push_back(entry);
}

void SecurityPolicyDatabase::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "entries: %ld", entries.size());
    getDisplayString().setTagArg("t", 0, buf);
}

}    // namespace ipsec
}    // namespace inet

