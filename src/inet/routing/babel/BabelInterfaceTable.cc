//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelInterfaceTable.h"

#include <sstream>

namespace inet {
namespace babel {

BabelInterface::~BabelInterface()
{
    deleteHTimer();
    deleteUTimer();
}

std::string BabelInterface::str() const
{
    std::stringstream out;
    out << getIfaceName();
    out << ":" << (enabled ? "ena" : "dis");
    out << " Send:" << AF::toStr(afsend);
    out << " Dist:" << AF::toStr(afdist);
    out << " SH:" << (splitHorizon ? "ena" : "dis");
    out << " Wired:" << (wired ? "ena" : "dis");
    out << " HSeqno:" << helloSeqno;
    out << " HInt:" << helloInterval;
    out << " UInt:" << updateInterval;
    return out.str();
}

void BabelInterface::addDirectlyConn(const netPrefix<L3Address>& pre)
{
    for (auto& p : directlyconn)
        if (p == pre)
            return; // already present
    directlyconn.push_back(pre);
}

void BabelInterface::resetHTimer() { resetTimer(helloTimer, CStoS(helloInterval)); }
void BabelInterface::resetHTimer(double delay) { resetTimer(helloTimer, delay); }
void BabelInterface::resetUTimer() { resetTimer(updateTimer, CStoS(updateInterval)); }
void BabelInterface::resetUTimer(double delay) { resetTimer(updateTimer, delay); }
void BabelInterface::deleteHTimer() { deleteTimer(&helloTimer); }
void BabelInterface::deleteUTimer() { deleteTimer(&updateTimer); }

BabelInterfaceTable::~BabelInterfaceTable()
{
    for (auto iface : interfaces)
        delete iface;
    interfaces.clear();
}

BabelInterface *BabelInterfaceTable::findInterfaceById(int ifaceId)
{
    for (auto iface : interfaces)
        if (iface->getInterfaceId() == ifaceId)
            return iface;
    return nullptr;
}

BabelInterface *BabelInterfaceTable::addInterface(BabelInterface *iface)
{
    BabelInterface *intable = findInterfaceById(iface->getInterfaceId());
    if (intable != nullptr)
        return intable;
    interfaces.push_back(iface);
    return iface;
}

void BabelInterfaceTable::removeInterface(BabelInterface *iface)
{
    for (auto it = interfaces.begin(); it != interfaces.end(); ++it) {
        if (*it == iface) {
            delete *it;
            interfaces.erase(it);
            return;
        }
    }
}

void BabelInterfaceTable::removeInterface(int ifaceId)
{
    for (auto it = interfaces.begin(); it != interfaces.end(); ++it) {
        if ((*it)->getInterfaceId() == ifaceId) {
            delete *it;
            interfaces.erase(it);
            return;
        }
    }
}

} // namespace babel
} // namespace inet
