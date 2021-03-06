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

#include "inet/networklayer/ipv4/Ipv4NetworkLayer.h"

#include "inet/common/StringFormat.h"

namespace inet {

Define_Module(Ipv4NetworkLayer);

void Ipv4NetworkLayer::refreshDisplay() const
{
    updateDisplayString();
}

void Ipv4NetworkLayer::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text);
    }
}

const char *Ipv4NetworkLayer::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'i':
            result = getSubmodule("ip")->getDisplayString().getTagArg("t", 0);
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

} // namespace inet

