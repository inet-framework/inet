//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapProtocolPrinter.h"

#include "inet/applications/coap/CoapHeaderSerializer.h"
#include "inet/applications/coap/CoapHeader_m.h"
#include "inet/applications/coap/CoapProtocol.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"

namespace inet {
namespace coap {

Register_Protocol_Printer(&getCoapProtocol(), CoapProtocolPrinter);

namespace {

const char *messageTypeName(CoapMessageType type)
{
    switch (type) {
        case COAP_TYPE_CON: return "CON";
        case COAP_TYPE_NON: return "NON";
        case COAP_TYPE_ACK: return "ACK";
        case COAP_TYPE_RST: return "RST";
        default: return "?";
    }
}

} // namespace

void CoapProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const CoapHeader>(chunk)) {
        uint8_t cls = getCoapCodeClass(header->getCode());
        uint8_t det = getCoapCodeDetail(header->getCode());
        context.typeColumn << "CoAP";
        context.infoColumn << messageTypeName(header->getMessageType())
                           << " " << static_cast<int>(cls) << "."
                           << (det < 10 ? "0" : "") << static_cast<int>(det)
                           << " MID=" << header->getMessageId()
                           << " TKL=" << header->getTokenArraySize();
        if (header->getOptionsArraySize() > 0)
            context.infoColumn << " opts=" << header->getOptionsArraySize();
        if (header->isIncorrect())
            context.infoColumn << " [MALFORMED]";
    }
    else {
        context.infoColumn << "(CoAP) " << chunk;
    }
}

} // namespace coap
} // namespace inet
