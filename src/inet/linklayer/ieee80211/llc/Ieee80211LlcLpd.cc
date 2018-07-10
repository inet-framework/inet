//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ProtocolGroup.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211LlcLpd.h"
#include "inet/linklayer/ieee80211/llc/LlcProtocolTag_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211LlcLpd);

void Ieee80211LlcLpd::encapsulate(Packet *frame)
{
    Ieee8022Llc::encapsulate(frame);
    frame->addTagIfAbsent<LlcProtocolTag>()->setProtocol(getProtocol());
}

const Protocol *Ieee80211LlcLpd::getProtocol() const
{
    return &Protocol::ieee8022;
}

} // namespace ieee80211
} // namespace inet

