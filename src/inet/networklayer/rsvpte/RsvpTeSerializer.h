//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RSVPTESERIALIZER_H
#define __INET_RSVPTESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/networklayer/rsvpte/RsvpHelloMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpPathMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpResvMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpSrefreshMsg_m.h"

namespace inet {

/**
 * Converts between RsvpPacket/RsvpMessage subtypes and binary (network byte
 * order) RSVP-TE PDUs, per RFC 2205 (RSVP) and RFC 3209 (RSVP-TE
 * extensions), with token-bucket parameters per RFC 2210 (Int-Serv).
 *
 * See RsvpTeSerializer.cc for the canonical per-message object layout table
 * (Workstream E, Phase 2 commit 3/4) and the model-enum <-> RFC wire-number
 * translation for the message type field.
 */
class INET_API RsvpTeSerializer : public FieldsChunkSerializer
{
  private:
    static void serializeSession(MemoryOutputStream& stream, const SessionObj& session);
    static SessionObj deserializeSession(MemoryInputStream& stream);

    static void serializeRsvpHop(MemoryOutputStream& stream, const RsvpHopObj& hop);
    static RsvpHopObj deserializeRsvpHop(MemoryInputStream& stream);

    static void serializeTimeValues(MemoryOutputStream& stream, int refreshPeriod);
    static int deserializeTimeValues(MemoryInputStream& stream);

    static void serializeLabelRequest(MemoryOutputStream& stream, const LabelRequestObj& lr);
    static LabelRequestObj deserializeLabelRequest(MemoryInputStream& stream);

    // classNum selects SENDER_TEMPLATE(11) or FILTER_SPEC(10) -- identical
    // C-Type 7 (LSP_TUNNEL_IPV4) body, only the Class-Num on the wire differs.
    static void serializeSenderTemplate(MemoryOutputStream& stream, const SenderTemplateObj& st, uint8_t classNum);
    static SenderTemplateObj deserializeSenderTemplate(MemoryInputStream& stream);

    // classNum selects SENDER_TSPEC(12) or FLOWSPEC(9) -- identical Int-Serv
    // Controlled-Load body, only the Class-Num on the wire differs.
    static void serializeTspec(MemoryOutputStream& stream, double reqBandwidth, uint8_t classNum);
    static double deserializeTspec(MemoryInputStream& stream);

    static void serializeLabel(MemoryOutputStream& stream, int label);
    static int deserializeLabel(MemoryInputStream& stream);

    static void serializeStyle(MemoryOutputStream& stream);
    static void deserializeStyle(MemoryInputStream& stream);

    static void serializeErrorSpec(MemoryOutputStream& stream, Ipv4Address errorNode, int errorCode);
    static void deserializeErrorSpec(MemoryInputStream& stream, Ipv4Address& errorNode, int& errorCode);

    // Omitted entirely from the wire when empty (RsvpTe.cc's
    // computeEroLength()/computeRroLength() likewise contribute 0 bytes).
    static void serializeEro(MemoryOutputStream& stream, const EroVector& ero);
    static EroVector deserializeEro(MemoryInputStream& stream);

    static void serializeRro(MemoryOutputStream& stream, const Ipv4AddressVector& rro);
    static Ipv4AddressVector deserializeRro(MemoryInputStream& stream);

    static void serializeFlowDescriptorList(MemoryOutputStream& stream, const FlowDescriptorVector& flows);
    static FlowDescriptorVector deserializeFlowDescriptorList(MemoryInputStream& stream, B remainingLength);

    // RFC 2961 Section 5.4 (refresh reduction, Workstream C8).
    static void serializeMessageId(MemoryOutputStream& stream, uint32_t epoch, uint32_t id);
    static void deserializeMessageId(MemoryInputStream& stream, uint32_t& epoch, uint32_t& id);

    static void serializeMessageIdAck(MemoryOutputStream& stream, bool nack, uint32_t epoch, uint32_t id);
    static void deserializeMessageIdAck(MemoryInputStream& stream, bool& nack, uint32_t& epoch, uint32_t& id);

    // Appends the optional MESSAGE_ID / MESSAGE_ID_ACK objects shared by all six
    // RsvpPacket subtypes, in that fixed order, when present -- called at the end
    // of each message's own serialize()/deserialize() case. endPos bounds how far
    // into the input stream this message's objects extend (same convention as the
    // ERO/RRO optional-trailing-object checks elsewhere in this file).
    static void serializeOptionalMessageIdObjects(MemoryOutputStream& stream, const RsvpPacket& pkt);
    static void deserializeOptionalMessageIdObjects(MemoryInputStream& stream, B endPos, RsvpPacket& pkt);

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    RsvpTeSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif
