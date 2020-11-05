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

#include "inet/networklayer/ipv4/ipsec/SecurityAssociationDatabase.h"

namespace inet {
namespace ipsec {

Define_Module(SecurityAssociationDatabase);

SecurityAssociationDatabase::~SecurityAssociationDatabase()
{
    for (SecurityAssociation *entry : entries)
        delete entry;
}

void SecurityAssociationDatabase::initialize()
{
    WATCH_PTRVECTOR(entries);
}

void SecurityAssociationDatabase::addEntry(SecurityAssociation *sadEntry)
{
    entries.push_back(sadEntry);
}

SecurityAssociation *SecurityAssociationDatabase::findEntry(IPsecRule::Direction direction, unsigned int spi)
{
    for (SecurityAssociation *entry : entries)
        if (entry->getDirection() == direction && entry->getSpi() == spi)
            return entry;
    return nullptr;
}

void SecurityAssociationDatabase::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "entries: %ld", entries.size());
    getDisplayString().setTagArg("t", 0, buf);
}

}    //ipsec namespace
}    //namespace

