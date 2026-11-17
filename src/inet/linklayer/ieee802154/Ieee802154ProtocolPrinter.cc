// SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee802154/Ieee802154Frame_m.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"

namespace inet {

class Ieee802154ProtocolPrinter : public ProtocolPrinter
{
  public:
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *, const cMessagePrinter::Options *, Context& context) const override
    {
        context.typeColumn << "IEEE 802.15.4";
        if (auto frame = dynamicPtrCast<const Ieee802154Frame>(chunk)) {
            context.sourceColumn << frame->getSourceAddress();
            context.destinationColumn << frame->getDestinationAddress();
            context.infoColumn << (frame->getFrameType() == 0 ? "BEACON" : frame->getFrameType() == 2 ? "ACK" : frame->getFrameType() == 3 ? "COMMAND" : "DATA")
                    << " version=" << frame->getFrameVersion() << " DSN=" << int(frame->getDsn())
                    << " parse=" << frame->getDecodeStatus() << " FCS="
                    << (!frame->getFcsPresent() ? "absent" : frame->getComputeFcs() ? "computed" : frame->getFcsValid() ? "valid" : "invalid");
        }
        else if (auto legacy = dynamicPtrCast<const Ieee802154MacHeader>(chunk)) {
            context.sourceColumn << legacy->getSrcAddr();
            context.destinationColumn << legacy->getDestAddr();
            context.infoColumn << "legacy model DSN=" << legacy->getSequenceId();
        }
        else
            context.infoColumn << chunk;
    }
};

Register_Protocol_Printer(&Protocol::ieee802154, Ieee802154ProtocolPrinter);
} // namespace inet
