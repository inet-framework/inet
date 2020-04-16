//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyProtocolPrinter.h"

namespace inet {
namespace physicallayer {

Register_Protocol_Printer(&Protocol::ieee80211FhssPhy, Ieee80211PhyProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee80211IrPhy, Ieee80211PhyProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee80211DsssPhy, Ieee80211PhyProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee80211HrDsssPhy, Ieee80211PhyProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee80211OfdmPhy, Ieee80211PhyProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee80211ErpOfdmPhy, Ieee80211PhyProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee80211HtPhy, Ieee80211PhyProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee80211VhtPhy, Ieee80211PhyProtocolPrinter);

void Ieee80211PhyProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    context.infoColumn << "(IEEE 802.11 Phy) " << chunk;
}

} // namespace physicallayer
} // namespace inet

