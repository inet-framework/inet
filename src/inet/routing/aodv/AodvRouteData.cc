//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/aodv/AodvRouteData.h"

namespace inet {
namespace aodv {

std::string AodvRouteData::str() const
{
    std::ostringstream out;
    out << "isActive = " << isActive()
        << ", hasValidDestNum = " << hasValidDestNum()
        << ", destNum = " << getDestSeqNum()
        << ", lifetime = " << getLifeTime();

    const auto& preList = getPrecursorList();

    if (!preList.empty()) {
        out << ", precursor list: ";
        auto iter = preList.begin();
        out << iter->first << "@" << iter->second;
        for (++iter; iter != preList.end(); ++iter)
            out << "; " << iter->first << "@" << iter->second;
    }
    return out.str();
};

} // namespace aodv
} // namespace inet

