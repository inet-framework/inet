//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/linklayer/DataLinkOsgVisualizer.h"

#include "inet/linklayer/base/MacProtocolBase.h"

#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#endif // INET_WITH_IEEE80211

namespace inet {

namespace visualizer {

Define_Module(DataLinkOsgVisualizer);

bool DataLinkOsgVisualizer::isLinkStart(cModule *module) const
{
    return dynamic_cast<MacProtocolBase *>(module) != nullptr
#ifdef INET_WITH_IEEE80211
           || dynamic_cast<ieee80211::ICoordinationFunction *>(module) != nullptr
#endif // INET_WITH_IEEE80211
        ;
}

bool DataLinkOsgVisualizer::isLinkEnd(cModule *module) const
{
    return dynamic_cast<MacProtocolBase *>(module) != nullptr
#ifdef INET_WITH_IEEE80211
           || dynamic_cast<ieee80211::ICoordinationFunction *>(module) != nullptr
#endif // INET_WITH_IEEE80211
        ;
}

} // namespace visualizer

} // namespace inet

