//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/nexthop/NextHopInterfaceData.h"

#include <algorithm>
#include <sstream>

namespace inet {

std::string NextHopInterfaceData::str() const
{
    std::stringstream out;
    out << "generic addr:" << getAddress();
    return out.str();
}

std::string NextHopInterfaceData::detailedInfo() const
{
    std::stringstream out;
    out << "generic addr:" << getAddress() << "\n"
        << "Metric: " << getMetric() << "\n";
    return out.str();
}

} // namespace inet

