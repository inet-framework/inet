//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

