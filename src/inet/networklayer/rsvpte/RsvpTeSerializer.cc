//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/rsvpte/RsvpTeSerializer.h"

#include <cstring>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/rsvpte/RsvpPathMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpResvMsg_m.h"

namespace inet {

// Registering the base RsvpPacket (in addition to each concrete subtype
// below) so that generic code paths popping the base type off a byte stream
// can dispatch too -- mirrors the LdpPacket + concrete-subtype registration
// pattern used by LdpPacketSerializer (see that file for the rationale).
Register_Serializer(RsvpPacket, RsvpTeSerializer);
Register_Serializer(RsvpPathMsg, RsvpTeSerializer);
Register_Serializer(RsvpPathTear, RsvpTeSerializer);
Register_Serializer(RsvpPathError, RsvpTeSerializer);
Register_Serializer(RsvpResvMsg, RsvpTeSerializer);
Register_Serializer(RsvpResvTear, RsvpTeSerializer);
Register_Serializer(RsvpResvError, RsvpTeSerializer);
Register_Serializer(RsvpHelloMsg, RsvpTeSerializer);

// ===========================================================================
// Canonical per-message RSVP object layout (Workstream E, Phase 2 commits 3
// and 4). This is the ONE place the on-wire object sequence is defined; the
// per-object byte budgets below must equal the constants in RsvpTe.cc's
// compute*MessageLength() helpers (B6) -- this serializer's test asserts
// that equality, and any future change to either side must keep them in
// sync.
//
//   Path        = CommonHdr, SESSION(7), RSVP_HOP(1), TIME_VALUES(1),
//                 LABEL_REQUEST(1), [ERO(1) if non-empty], SENDER_TEMPLATE(7),
//                 SENDER_TSPEC(2)
//   PathTear    = CommonHdr, SESSION(7), RSVP_HOP(1), SENDER_TEMPLATE(7)
//   PathErr     = CommonHdr, SESSION(7), ERROR_SPEC(1), SENDER_TEMPLATE(7),
//                 SENDER_TSPEC(2)                                  [no RSVP_HOP]
//   Resv        = CommonHdr, SESSION(7), RSVP_HOP(1), TIME_VALUES(1), STYLE(1),
//                 { FLOWSPEC(2), FILTER_SPEC(7), LABEL(1), [RRO(1) if non-empty] }*
//   ResvTear    = CommonHdr, SESSION(7), RSVP_HOP(1), STYLE(1),
//                 { FLOWSPEC(2), FILTER_SPEC(7), LABEL(1), [RRO(1) if non-empty] }*
//                                                                   [no TIME_VALUES]
//   ResvErr     = CommonHdr, SESSION(7), RSVP_HOP(1), ERROR_SPEC(1), STYLE(1),
//                 { FLOWSPEC(2), FILTER_SPEC(7), LABEL(1), [RRO(1) if non-empty] }*
//   Hello       = CommonHdr, HELLO(1 request / 2 ack)                [no SESSION]
//
// (C-Type in parentheses.) Class-Nums (RFC 2205 / RFC 3209 Appendix A):
// SESSION=1, RSVP_HOP=3, TIME_VALUES=5, ERROR_SPEC=6, STYLE=8, FLOWSPEC=9,
// FILTER_SPEC=10, SENDER_TEMPLATE=11, SENDER_TSPEC=12, LABEL=16,
// LABEL_REQUEST=19, ERO=20, RRO=21, HELLO=22.
//
// Message-type wire numbers (RFC 2205 Section 3.1.1; Hello per RFC 3209
// Appendix): Path=1, Resv=2, PathErr=3, ResvErr=4, PathTear=5, ResvTear=6,
// ResvConf=7 (not modeled), Hello=20. The model's internal RsvpConstants
// enum (RsvpPacket.msg) does NOT use these numbers -- it was never written
// to match RFC wire numbering (PATH_MESSAGE=1, RESV_MESSAGE=2,
// PTEAR_MESSAGE=3, RTEAR_MESSAGE=4, PERROR_MESSAGE=5, RERROR_MESSAGE=6,
// HELLO_MESSAGE=7). This serializer SERIALIZES THE RFC NUMBER and keeps a
// translation table (rfcMsgTypeFor()/rsvpKindFor() below); the model enum is
// left untouched (renumbering it is an unrelated, riskier change with its
// own blast radius across every switch in RsvpTe.cc -- out of scope here).
//
//   model rsvpKind      | RFC wire msg type
//   --------------------+-------------------
//   PATH_MESSAGE   (1)  | Path      (1)
//   RESV_MESSAGE   (2)  | Resv      (2)
//   PTEAR_MESSAGE  (3)  | PathTear  (5)
//   RTEAR_MESSAGE  (4)  | ResvTear  (6)
//   PERROR_MESSAGE (5)  | PathErr   (3)
//   RERROR_MESSAGE (6)  | ResvErr   (4)
//   HELLO_MESSAGE  (7)  | Hello     (20)
//
// Known non-features (documented, not silently dropped):
// - SessionObj.setupPri/holdingPri are model-only fields; a real RSVP-TE
//   implementation would carry them in a SESSION_ATTRIBUTE object (RFC 3209
//   Section 4.7), which this model does not emit (B6 never budgeted bytes
//   for one). They do not survive a serialize/deserialize round trip.
// - RsvpPathTear.force is model-internal signalling state with no RFC wire
//   representation; not serialized (B6 does not budget space for it either).
// - SENDER_TSPEC/FLOWSPEC only carry a single bandwidth value
//   (SenderTspecObj::req_bandwidth); it is written into the RFC 2210 token
//   bucket's rate [r], size [b] and peak rate [p] fields alike (a real
//   sender would generally set these independently). The minimum policed
//   unit [m] and maximum packet size [M] parameters are not modeled state;
//   fixed placeholder values are written and discarded on deserialize.
// ===========================================================================

namespace {

enum RsvpClassNum {
    CLASSNUM_SESSION = 1,
    CLASSNUM_RSVP_HOP = 3,
    CLASSNUM_TIME_VALUES = 5,
    CLASSNUM_ERROR_SPEC = 6,
    CLASSNUM_STYLE = 8,
    CLASSNUM_FLOWSPEC = 9,
    CLASSNUM_FILTER_SPEC = 10,
    CLASSNUM_SENDER_TEMPLATE = 11,
    CLASSNUM_SENDER_TSPEC = 12,
    CLASSNUM_LABEL = 16,
    CLASSNUM_LABEL_REQUEST = 19,
    CLASSNUM_ERO = 20,
    CLASSNUM_RRO = 21,
    CLASSNUM_HELLO = 22,
};

constexpr uint8_t CTYPE_IPV4 = 1;
constexpr uint8_t CTYPE_LSP_TUNNEL_IPV4 = 7;
constexpr uint8_t CTYPE_INTSERV = 2;
constexpr uint8_t CTYPE_HELLO_REQUEST = 1;
constexpr uint8_t CTYPE_HELLO_ACK = 2;

// RFC 2205 Section 3.1.1 message-type numbers (Hello per RFC 3209 Appendix).
enum RsvpWireMsgType {
    WIRE_PATH = 1,
    WIRE_RESV = 2,
    WIRE_PATH_ERR = 3,
    WIRE_RESV_ERR = 4,
    WIRE_PATH_TEAR = 5,
    WIRE_RESV_TEAR = 6,
    WIRE_HELLO = 20,
};

int rfcMsgTypeForRsvpKind(int rsvpKind)
{
    switch (rsvpKind) {
        case PATH_MESSAGE: return WIRE_PATH;
        case RESV_MESSAGE: return WIRE_RESV;
        case PTEAR_MESSAGE: return WIRE_PATH_TEAR;
        case RTEAR_MESSAGE: return WIRE_RESV_TEAR;
        case PERROR_MESSAGE: return WIRE_PATH_ERR;
        case RERROR_MESSAGE: return WIRE_RESV_ERR;
        case HELLO_MESSAGE: return WIRE_HELLO;
        default: throw cRuntimeError("RsvpTeSerializer: unknown rsvpKind %d", rsvpKind);
    }
}

// Returns -1 for an unrecognized wire message type -- a corrupted/foreign
// byte stream is data, not a model bug, so (mirroring LdpPacketSerializer's
// default: markIncorrect() branch) this must not throw; the caller returns
// an incorrect-marked chunk instead.
int rsvpKindForRfcMsgType(int wireType)
{
    switch (wireType) {
        case WIRE_PATH: return PATH_MESSAGE;
        case WIRE_RESV: return RESV_MESSAGE;
        case WIRE_PATH_TEAR: return PTEAR_MESSAGE;
        case WIRE_RESV_TEAR: return RTEAR_MESSAGE;
        case WIRE_PATH_ERR: return PERROR_MESSAGE;
        case WIRE_RESV_ERR: return RERROR_MESSAGE;
        case WIRE_HELLO: return HELLO_MESSAGE;
        default: return -1;
    }
}

struct ObjectHeader {
    uint16_t length; // total object length in bytes, including this 4-byte header
    uint8_t classNum;
    uint8_t cType;
};

void writeObjectHeader(MemoryOutputStream& stream, uint16_t length, uint8_t classNum, uint8_t cType)
{
    stream.writeUint16Be(length);
    stream.writeByte(classNum);
    stream.writeByte(cType);
}

ObjectHeader readObjectHeader(MemoryInputStream& stream)
{
    ObjectHeader h;
    h.length = stream.readUint16Be();
    h.classNum = stream.readByte();
    h.cType = stream.readByte();
    return h;
}

// C++17 has no std::bit_cast (that's C++20); a memcpy-based reinterpretation
// is the standard portable way to punch IEEE-754 float bits into/out of a
// uint32_t without violating strict aliasing.
template<typename To, typename From>
To bitCast(From from)
{
    static_assert(sizeof(To) == sizeof(From), "bitCast: size mismatch");
    To to;
    std::memcpy(&to, &from, sizeof(To));
    return to;
}

// Peeks the next object's Class-Num without consuming it (used to detect the
// optional ERO in Path messages and the optional per-flow RRO in Resv-family
// messages, both of which have no explicit presence flag on the wire).
uint8_t peekClassNum(MemoryInputStream& stream)
{
    B pos = stream.getPosition();
    stream.readUint16Be(); // length, discarded
    uint8_t classNum = stream.readByte();
    stream.seek(pos);
    return classNum;
}

} // namespace

void RsvpTeSerializer::serializeSession(MemoryOutputStream& stream, const SessionObj& session)
{
    // Body: DestAddress(4) + Reserved(2) + Tunnel_Id(2) + Extended_Tunnel_Id(4) = 12
    // NOTE: setupPri/holdingPri are NOT on the wire -- see the non-features list above.
    writeObjectHeader(stream, 16, CLASSNUM_SESSION, CTYPE_LSP_TUNNEL_IPV4);
    stream.writeIpv4Address(session.DestAddress);
    stream.writeUint16Be(0); // reserved
    stream.writeUint16Be(static_cast<uint16_t>(session.Tunnel_Id));
    stream.writeUint32Be(static_cast<uint32_t>(session.Extended_Tunnel_Id));
}

SessionObj RsvpTeSerializer::deserializeSession(MemoryInputStream& stream)
{
    readObjectHeader(stream); // assumed SESSION/LSP_TUNNEL_IPV4
    SessionObj session;
    session.DestAddress = stream.readIpv4Address();
    stream.readUint16Be(); // reserved
    session.Tunnel_Id = stream.readUint16Be();
    session.Extended_Tunnel_Id = stream.readUint32Be();
    session.setupPri = 0; // not on the wire (see non-features list)
    session.holdingPri = 0;
    return session;
}

void RsvpTeSerializer::serializeRsvpHop(MemoryOutputStream& stream, const RsvpHopObj& hop)
{
    writeObjectHeader(stream, 12, CLASSNUM_RSVP_HOP, CTYPE_IPV4);
    stream.writeIpv4Address(hop.Next_Hop_Address);
    stream.writeIpv4Address(hop.Logical_Interface_Handle);
}

RsvpHopObj RsvpTeSerializer::deserializeRsvpHop(MemoryInputStream& stream)
{
    readObjectHeader(stream);
    RsvpHopObj hop;
    hop.Next_Hop_Address = stream.readIpv4Address();
    hop.Logical_Interface_Handle = stream.readIpv4Address();
    return hop;
}

void RsvpTeSerializer::serializeTimeValues(MemoryOutputStream& stream, int refreshPeriod)
{
    writeObjectHeader(stream, 8, CLASSNUM_TIME_VALUES, CTYPE_IPV4);
    stream.writeUint32Be(static_cast<uint32_t>(refreshPeriod));
}

int RsvpTeSerializer::deserializeTimeValues(MemoryInputStream& stream)
{
    readObjectHeader(stream);
    return static_cast<int>(stream.readUint32Be());
}

void RsvpTeSerializer::serializeLabelRequest(MemoryOutputStream& stream, const LabelRequestObj& lr)
{
    writeObjectHeader(stream, 8, CLASSNUM_LABEL_REQUEST, CTYPE_IPV4);
    stream.writeUint16Be(0); // reserved
    stream.writeUint16Be(static_cast<uint16_t>(lr.l3pid));
}

LabelRequestObj RsvpTeSerializer::deserializeLabelRequest(MemoryInputStream& stream)
{
    readObjectHeader(stream);
    LabelRequestObj lr;
    stream.readUint16Be(); // reserved
    lr.l3pid = stream.readUint16Be();
    return lr;
}

void RsvpTeSerializer::serializeSenderTemplate(MemoryOutputStream& stream, const SenderTemplateObj& st, uint8_t classNum)
{
    writeObjectHeader(stream, 12, classNum, CTYPE_LSP_TUNNEL_IPV4);
    stream.writeIpv4Address(st.SrcAddress);
    stream.writeUint16Be(0); // reserved
    stream.writeUint16Be(static_cast<uint16_t>(st.Lsp_Id));
}

SenderTemplateObj RsvpTeSerializer::deserializeSenderTemplate(MemoryInputStream& stream)
{
    readObjectHeader(stream); // assumed SENDER_TEMPLATE or FILTER_SPEC, same body
    SenderTemplateObj st;
    st.SrcAddress = stream.readIpv4Address();
    stream.readUint16Be(); // reserved
    st.Lsp_Id = stream.readUint16Be();
    return st;
}

// RFC 2210 Section 3.1 (SENDER_TSPEC) / 3.2.1 (FLOWSPEC, Controlled-Load):
// message header word + per-service header word + per-parameter header word
// + 5 token-bucket parameter words (r, b, p, m, M) = 8 words = 32 bytes body.
void RsvpTeSerializer::serializeTspec(MemoryOutputStream& stream, double reqBandwidth, uint8_t classNum)
{
    writeObjectHeader(stream, 36, classNum, CTYPE_INTSERV);
    // Message header: version(4 bits)=0, reserved(12 bits), overall length = 7 words
    stream.writeNBitsOfUint64Be(0, 4);
    stream.writeNBitsOfUint64Be(0, 12);
    stream.writeUint16Be(7);
    // Per-service header: service number (1=default/global for Tspec, 5=Controlled-Load
    // for Flowspec -- both accepted identically on read, since this model only ever
    // requests Controlled-Load), break bit + reserved, service data length = 6 words
    stream.writeByte(classNum == CLASSNUM_FLOWSPEC ? 5 : 1);
    stream.writeByte(0);
    stream.writeUint16Be(6);
    // Per-parameter header: Token_Bucket_TSpec parameter 127, no flags, length = 5 words
    stream.writeByte(127);
    stream.writeByte(0);
    stream.writeUint16Be(5);
    float r = static_cast<float>(reqBandwidth);
    float b = r; // not modeled independently -- see non-features list
    float p = r;
    stream.writeUint32Be(bitCast<uint32_t>(r));
    stream.writeUint32Be(bitCast<uint32_t>(b));
    stream.writeUint32Be(bitCast<uint32_t>(p));
    stream.writeUint32Be(20); // Minimum Policed Unit [m]: not modeled, fixed placeholder
    stream.writeUint32Be(1500); // Maximum Packet Size [M]: not modeled, fixed placeholder
}

double RsvpTeSerializer::deserializeTspec(MemoryInputStream& stream)
{
    readObjectHeader(stream);
    stream.readNBitsToUint64Be(4); // version
    stream.readNBitsToUint64Be(12); // reserved
    stream.readUint16Be(); // overall length
    stream.readByte(); // service number
    stream.readByte(); // break bit + reserved
    stream.readUint16Be(); // service data length
    stream.readByte(); // parameter id
    stream.readByte(); // parameter flags
    stream.readUint16Be(); // parameter length
    uint32_t rBits = stream.readUint32Be();
    stream.readUint32Be(); // [b], mirrors [r], discarded
    stream.readUint32Be(); // [p], mirrors [r], discarded
    stream.readUint32Be(); // [m], not modeled, discarded
    stream.readUint32Be(); // [M], not modeled, discarded
    return static_cast<double>(bitCast<float>(rBits));
}

void RsvpTeSerializer::serializeLabel(MemoryOutputStream& stream, int label)
{
    writeObjectHeader(stream, 8, CLASSNUM_LABEL, CTYPE_IPV4);
    stream.writeUint32Be(static_cast<uint32_t>(label));
}

int RsvpTeSerializer::deserializeLabel(MemoryInputStream& stream)
{
    readObjectHeader(stream);
    return static_cast<int>(stream.readUint32Be());
}

// RFC 2205 Section 3.1.1 STYLE C-Type 1: Flags(1) + Option Vector(3). This
// model always uses the Shared-Explicit (SE) reservation style (no per-message
// style field exists anywhere in RsvpResvMsg/RsvpTe.cc), so the Option Vector
// is a fixed constant, not a representation gap: Sharing Control = 2 (Shared),
// Sender Selection = 2 (Explicit) => 0x000012.
void RsvpTeSerializer::serializeStyle(MemoryOutputStream& stream)
{
    writeObjectHeader(stream, 8, CLASSNUM_STYLE, CTYPE_IPV4);
    stream.writeByte(0); // flags
    stream.writeUint24Be(0x000012); // Option Vector: Shared-Explicit (SE)
}

void RsvpTeSerializer::deserializeStyle(MemoryInputStream& stream)
{
    readObjectHeader(stream);
    stream.readByte(); // flags
    stream.readUint24Be(); // Option Vector, assumed SE (the model's only style)
}

// RFC 2205 Section 3.1.1 ERROR_SPEC C-Type 1 (IPv4): Error Node Address(4) +
// Flags(1) + Error Code(1) + Error Value(2). The model's errorCode is one of
// this file's own PATH_ERR_*/RESV_ERR_* dispatch constants (RsvpTe.cc), which
// fit comfortably in the 1-byte Error Code field; Error Value is unused.
void RsvpTeSerializer::serializeErrorSpec(MemoryOutputStream& stream, Ipv4Address errorNode, int errorCode)
{
    writeObjectHeader(stream, 12, CLASSNUM_ERROR_SPEC, CTYPE_IPV4);
    stream.writeIpv4Address(errorNode);
    stream.writeByte(0); // flags
    stream.writeByte(static_cast<uint8_t>(errorCode));
    stream.writeUint16Be(0); // error value, not modeled
}

void RsvpTeSerializer::deserializeErrorSpec(MemoryInputStream& stream, Ipv4Address& errorNode, int& errorCode)
{
    readObjectHeader(stream);
    errorNode = stream.readIpv4Address();
    stream.readByte(); // flags
    errorCode = stream.readByte();
    stream.readUint16Be(); // error value
}

// RFC 3209 Section 4.3.3.1: ERO IPv4 prefix subobject = L bit + Type(7 bits),
// Length(1, =8), Address(4), Prefix Length(1), Reserved(1) = 8 bytes/hop.
// Prefix Length is fixed at 32 (host route) -- this model's EroObj carries no
// prefix length, only single router addresses.
void RsvpTeSerializer::serializeEro(MemoryOutputStream& stream, const EroVector& ero)
{
    if (ero.empty())
        return;
    writeObjectHeader(stream, static_cast<uint16_t>(4 + 8 * ero.size()), CLASSNUM_ERO, CTYPE_IPV4);
    for (const auto& hop : ero) {
        stream.writeByte((hop.L ? 0x80 : 0x00) | 0x01); // L bit + Type=1 (IPv4 prefix)
        stream.writeByte(8); // subobject length
        stream.writeIpv4Address(hop.node);
        stream.writeByte(32); // prefix length
        stream.writeByte(0); // reserved
    }
}

EroVector RsvpTeSerializer::deserializeEro(MemoryInputStream& stream)
{
    ObjectHeader oh = readObjectHeader(stream);
    int n = (oh.length - 4) / 8;
    EroVector ero;
    for (int i = 0; i < n; i++) {
        uint8_t typeByte = stream.readByte();
        stream.readByte(); // subobject length, assumed 8
        EroObj hop;
        hop.L = (typeByte & 0x80) != 0;
        hop.node = stream.readIpv4Address();
        stream.readByte(); // prefix length, assumed 32
        stream.readByte(); // reserved
        ero.push_back(hop);
    }
    return ero;
}

// RFC 3209 Section 4.4.1: RRO IPv4 address subobject = Type(1, =1), Length(1,
// =8), Address(4), Prefix Length(1), Flags(1) = 8 bytes/hop.
void RsvpTeSerializer::serializeRro(MemoryOutputStream& stream, const Ipv4AddressVector& rro)
{
    if (rro.empty())
        return;
    writeObjectHeader(stream, static_cast<uint16_t>(4 + 8 * rro.size()), CLASSNUM_RRO, CTYPE_IPV4);
    for (const auto& addr : rro) {
        stream.writeByte(0x01); // Type=1 (IPv4 address)
        stream.writeByte(8); // subobject length
        stream.writeIpv4Address(addr);
        stream.writeByte(32); // prefix length
        stream.writeByte(0); // flags
    }
}

Ipv4AddressVector RsvpTeSerializer::deserializeRro(MemoryInputStream& stream)
{
    ObjectHeader oh = readObjectHeader(stream);
    int n = (oh.length - 4) / 8;
    Ipv4AddressVector rro;
    for (int i = 0; i < n; i++) {
        stream.readByte(); // type, assumed 1
        stream.readByte(); // subobject length, assumed 8
        rro.push_back(stream.readIpv4Address());
        stream.readByte(); // prefix length, assumed 32
        stream.readByte(); // flags
    }
    return rro;
}

// RFC 2205 Appendix A: a Resv-family flow descriptor list is
// { FLOWSPEC FILTER_SPEC [LABEL] }* per style; this model always includes a
// LABEL (RFC 3209 label distribution) and optionally an RRO (RFC 3209 Section
// 4.4, recorded route), and reuses this same layout for Resv, ResvTear and
// ResvError alike (mirroring RsvpTe.cc's shared computeFlowDescriptorListLength()).
void RsvpTeSerializer::serializeFlowDescriptorList(MemoryOutputStream& stream, const FlowDescriptorVector& flows)
{
    for (const auto& flow : flows) {
        serializeTspec(stream, flow.Flowspec_Object.req_bandwidth, CLASSNUM_FLOWSPEC);
        serializeSenderTemplate(stream, flow.Filter_Spec_Object, CLASSNUM_FILTER_SPEC);
        serializeLabel(stream, flow.label);
        serializeRro(stream, flow.RRO);
    }
}

FlowDescriptorVector RsvpTeSerializer::deserializeFlowDescriptorList(MemoryInputStream& stream, B remainingLength)
{
    FlowDescriptorVector flows;
    B endPos = stream.getPosition() + remainingLength;
    while (stream.getPosition() < endPos) {
        FlowDescriptor_t flow;
        flow.Flowspec_Object.req_bandwidth = deserializeTspec(stream);
        SenderTemplateObj st = deserializeSenderTemplate(stream);
        flow.Filter_Spec_Object.SrcAddress = st.SrcAddress;
        flow.Filter_Spec_Object.Lsp_Id = st.Lsp_Id;
        flow.label = deserializeLabel(stream);
        if (stream.getPosition() < endPos && peekClassNum(stream) == CLASSNUM_RRO)
            flow.RRO = deserializeRro(stream);
        flows.push_back(flow);
    }
    return flows;
}

void RsvpTeSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& msg = staticPtrCast<const RsvpMessage>(chunk);
    B totalLength = msg->getChunkLength();
    int rsvpKind = msg->getRsvpKind();
    int wireMsgType = rfcMsgTypeForRsvpKind(rsvpKind);

    // RFC 2205 Section 3.1.2 common header (8 bytes)
    stream.writeNBitsOfUint64Be(1, 4); // version
    stream.writeNBitsOfUint64Be(0, 4); // flags, not modeled
    stream.writeByte(static_cast<uint8_t>(wireMsgType));
    stream.writeUint16Be(0); // checksum, not computed by this model
    stream.writeByte(255); // Send_TTL, not modeled
    stream.writeByte(0); // reserved
    stream.writeUint16Be(totalLength.get<B>());

    switch (rsvpKind) {
        case PATH_MESSAGE: {
            const auto& pm = staticPtrCast<const RsvpPathMsg>(chunk);
            serializeSession(stream, pm->getSession());
            serializeRsvpHop(stream, pm->getHop());
            serializeTimeValues(stream, pm->getRefreshPeriod());
            serializeLabelRequest(stream, pm->getLabel_request());
            serializeEro(stream, pm->getERO());
            serializeSenderTemplate(stream, pm->getSender_descriptor().Sender_Template_Object, CLASSNUM_SENDER_TEMPLATE);
            serializeTspec(stream, pm->getSender_descriptor().Sender_Tspec_Object.req_bandwidth, CLASSNUM_SENDER_TSPEC);
            break;
        }
        case PTEAR_MESSAGE: {
            const auto& pt = staticPtrCast<const RsvpPathTear>(chunk);
            serializeSession(stream, pt->getSession());
            serializeRsvpHop(stream, pt->getHop());
            serializeSenderTemplate(stream, pt->getSenderTemplate(), CLASSNUM_SENDER_TEMPLATE);
            break;
        }
        case PERROR_MESSAGE: {
            const auto& pe = staticPtrCast<const RsvpPathError>(chunk);
            serializeSession(stream, pe->getSession());
            serializeErrorSpec(stream, pe->getErrorNode(), pe->getErrorCode());
            serializeSenderTemplate(stream, pe->getSender_descriptor().Sender_Template_Object, CLASSNUM_SENDER_TEMPLATE);
            serializeTspec(stream, pe->getSender_descriptor().Sender_Tspec_Object.req_bandwidth, CLASSNUM_SENDER_TSPEC);
            break;
        }
        case RESV_MESSAGE: {
            const auto& rm = staticPtrCast<const RsvpResvMsg>(chunk);
            serializeSession(stream, rm->getSession());
            serializeRsvpHop(stream, rm->getHop());
            serializeTimeValues(stream, rm->getRefreshPeriod());
            serializeStyle(stream);
            serializeFlowDescriptorList(stream, rm->getFlowDescriptor());
            break;
        }
        case RTEAR_MESSAGE: {
            const auto& rt = staticPtrCast<const RsvpResvTear>(chunk);
            serializeSession(stream, rt->getSession());
            serializeRsvpHop(stream, rt->getHop());
            serializeStyle(stream);
            serializeFlowDescriptorList(stream, rt->getFlowDescriptor());
            break;
        }
        case RERROR_MESSAGE: {
            const auto& re = staticPtrCast<const RsvpResvError>(chunk);
            serializeSession(stream, re->getSession());
            serializeRsvpHop(stream, re->getHop());
            serializeErrorSpec(stream, re->getErrorNode(), re->getErrorCode());
            serializeStyle(stream);
            serializeFlowDescriptorList(stream, re->getFlowDescriptor());
            break;
        }
        case HELLO_MESSAGE: {
            const auto& hm = staticPtrCast<const RsvpHelloMsg>(chunk);
            // RFC 3209 Appendix A HELLO object: Src Instance(4) + Dst Instance(4).
            writeObjectHeader(stream, 12, CLASSNUM_HELLO, hm->getRequest() ? CTYPE_HELLO_REQUEST : CTYPE_HELLO_ACK);
            stream.writeUint32Be(static_cast<uint32_t>(hm->getSrcInstance()));
            stream.writeUint32Be(static_cast<uint32_t>(hm->getDstInstance()));
            break;
        }
        default:
            throw cRuntimeError("RsvpTeSerializer: cannot serialize unknown rsvpKind %d", rsvpKind);
    }
}

const Ptr<Chunk> RsvpTeSerializer::deserialize(MemoryInputStream& stream) const
{
    B msgStart = stream.getPosition();
    stream.readNBitsToUint64Be(4); // version, assumed 1
    stream.readNBitsToUint64Be(4); // flags, not modeled
    int wireMsgType = stream.readByte();
    stream.readUint16Be(); // checksum, not validated
    stream.readByte(); // Send_TTL, not modeled
    stream.readByte(); // reserved
    uint16_t totalLength = stream.readUint16Be();
    B endPos = msgStart + B(totalLength); // one-past-the-last byte of this RSVP message

    int rsvpKind = rsvpKindForRfcMsgType(wireMsgType);
    Ptr<RsvpMessage> msg;

    switch (rsvpKind) {
        case PATH_MESSAGE: {
            auto pm = makeShared<RsvpPathMsg>();
            pm->setSession(deserializeSession(stream));
            pm->setHop(deserializeRsvpHop(stream));
            pm->setRefreshPeriod(deserializeTimeValues(stream));
            pm->setLabel_request(deserializeLabelRequest(stream));
            if (stream.getPosition() < endPos && peekClassNum(stream) == CLASSNUM_ERO)
                pm->setERO(deserializeEro(stream));
            else
                pm->setERO(EroVector());
            SenderDescriptor_t sd;
            sd.Sender_Template_Object = deserializeSenderTemplate(stream);
            sd.Sender_Tspec_Object.req_bandwidth = deserializeTspec(stream);
            pm->setSender_descriptor(sd);
            msg = pm;
            break;
        }
        case PTEAR_MESSAGE: {
            auto pt = makeShared<RsvpPathTear>();
            pt->setSession(deserializeSession(stream));
            pt->setHop(deserializeRsvpHop(stream));
            pt->setSenderTemplate(deserializeSenderTemplate(stream));
            pt->setForce(false); // model-internal state, not on the wire
            msg = pt;
            break;
        }
        case PERROR_MESSAGE: {
            auto pe = makeShared<RsvpPathError>();
            pe->setSession(deserializeSession(stream));
            Ipv4Address errorNode;
            int errorCode;
            deserializeErrorSpec(stream, errorNode, errorCode);
            pe->setErrorNode(errorNode);
            pe->setErrorCode(errorCode);
            SenderDescriptor_t sd;
            sd.Sender_Template_Object = deserializeSenderTemplate(stream);
            sd.Sender_Tspec_Object.req_bandwidth = deserializeTspec(stream);
            pe->setSender_descriptor(sd);
            msg = pe;
            break;
        }
        case RESV_MESSAGE: {
            auto rm = makeShared<RsvpResvMsg>();
            rm->setSession(deserializeSession(stream));
            rm->setHop(deserializeRsvpHop(stream));
            rm->setRefreshPeriod(deserializeTimeValues(stream));
            deserializeStyle(stream);
            rm->setFlowDescriptor(deserializeFlowDescriptorList(stream, endPos - stream.getPosition()));
            msg = rm;
            break;
        }
        case RTEAR_MESSAGE: {
            auto rt = makeShared<RsvpResvTear>();
            rt->setSession(deserializeSession(stream));
            rt->setHop(deserializeRsvpHop(stream));
            deserializeStyle(stream);
            rt->setFlowDescriptor(deserializeFlowDescriptorList(stream, endPos - stream.getPosition()));
            msg = rt;
            break;
        }
        case RERROR_MESSAGE: {
            auto re = makeShared<RsvpResvError>();
            re->setSession(deserializeSession(stream));
            re->setHop(deserializeRsvpHop(stream));
            Ipv4Address errorNode;
            int errorCode;
            deserializeErrorSpec(stream, errorNode, errorCode);
            re->setErrorNode(errorNode);
            re->setErrorCode(errorCode);
            deserializeStyle(stream);
            re->setFlowDescriptor(deserializeFlowDescriptorList(stream, endPos - stream.getPosition()));
            msg = re;
            break;
        }
        case HELLO_MESSAGE: {
            auto hm = makeShared<RsvpHelloMsg>();
            ObjectHeader oh = readObjectHeader(stream); // HELLO
            hm->setRequest(oh.cType == CTYPE_HELLO_REQUEST);
            hm->setAck(oh.cType == CTYPE_HELLO_ACK);
            hm->setSrcInstance(static_cast<int>(stream.readUint32Be()));
            hm->setDstInstance(static_cast<int>(stream.readUint32Be()));
            msg = hm;
            break;
        }
        default: {
            auto unknown = makeShared<RsvpMessage>();
            unknown->markIncorrect();
            msg = unknown;
            break;
        }
    }

    msg->setRsvpKind(rsvpKind);
    msg->setChunkLength(B(totalLength));
    return msg;
}

} // namespace inet
