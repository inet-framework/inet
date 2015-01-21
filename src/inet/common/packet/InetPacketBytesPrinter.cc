//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/common/INETDefs.h"

namespace inet {

#if OMNETPP_VERSION >= 0x0405

class INET_API InetPacketBytesPrinter : public cMessagePrinter
{
  protected:
    mutable bool showEncapsulatedPackets;

  public:
    InetPacketBytesPrinter() { showEncapsulatedPackets = true; }
    virtual ~InetPacketBytesPrinter() {}
    virtual int getScoreFor(cMessage *msg) const override;
    virtual void printMessage(std::ostream& os, cMessage *msg) const override;
};

Register_MessagePrinter(InetPacketBytesPrinter);

static const char INFO_SEPAR[] = "  \t";
//static const char INFO_SEPAR[] = "   ";

int InetPacketBytesPrinter::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 18 : 0;
}

void InetPacketBytesPrinter::printMessage(std::ostream& os, cMessage *msg) const
{
    std::string outs;

    //reset mutable variables
    showEncapsulatedPackets = true;

    for (cPacket *pk = dynamic_cast<cPacket *>(msg); showEncapsulatedPackets && pk; pk = pk->getEncapsulatedPacket()) {
        std::ostringstream out;
        out << pk->getClassName() << ":" << pk->getByteLength() << " bytes";
        if (outs.length())
            out << INFO_SEPAR << outs;
        outs = out.str();
    }
    os << outs;
}

#endif    // Register_MessagePrinter

} // namespace inet

