//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ted/LinkStateRoutingSerializer.h"

#include <cstring>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/ted/Ted.h" // for Ted::MAX_SRLGS, the single source of truth for the srlgs[] cap

namespace inet {

Register_Serializer(LinkStateMsg, LinkStateRoutingSerializer);

// ===========================================================================
// LinkStateSerializer format v2 -- NOT an RFC wire format; see the class
// comment in LinkStateRoutingSerializer.h. This is the ONE place the
// per-record byte layout is defined.
//
// TeLinkStateInfo record (164 bytes, Ted.msg field order):
//
//   Field              Wire width   Notes
//   -----              ----------   -----
//   advrouter          4 (Ipv4)
//   linkid             4 (Ipv4)
//   local              4 (Ipv4)
//   remote             4 (Ipv4)
//   metric             8            IEEE-754 double, raw bit pattern
//   MaxBandwidth       8            IEEE-754 double, raw bit pattern
//   UnResvBandwidth[8] 64 (8x8)     IEEE-754 doubles, raw bit pattern
//   timestamp          8            simtime_t is NOT meaningfully portable
//                                   across a serialize/deserialize round trip
//                                   (it is a simulation-run-relative fixed
//                                   point value); written as 8 zero bytes and
//                                   read back as SIMTIME_ZERO, per the plan's
//                                   "skipped-as-0" instruction. This field
//                                   still occupies its 8 bytes on the wire so
//                                   the record stays fixed-width.
//   sourceId           4
//   messageId          4
//   state              1            bool as uint8 (0/1)
//   pad                3            reserved, always 0 -- rounds the v1
//                                   portion up to a 4-byte boundary (113 data
//                                   bytes -> 116; this is the v1/v2 boundary)
//   ----- v2 additions (D3: TE metric/administrative group/SRLGs) -----
//   teMetric           8            IEEE-754 double, raw bit pattern; 0 means
//                                   "fall back to metric", see Ted::getTeMetric()
//   adminGroup         4            uint32, RFC 3630 Section 2.5.9-style color bits
//   srlgsCount         4            uint32, number of valid entries in srlgs[]
//                                   below (0..Ted::MAX_SRLGS)
//   srlgs[MAX_SRLGS]   32 (8x4)     uint32 each; entries at index >= srlgsCount
//                                   are always 0 and ignored on read. MAX_SRLGS
//                                   (8) must match Ted::MAX_SRLGS (Ted.h) and
//                                   the fixed array size in Ted.msg.
//   -----
//   total              164          (116 v1 bytes + 48 v2 bytes; 48 is
//                                   already a multiple of 4, so no additional
//                                   padding is needed to keep the record
//                                   4-byte aligned)
//
// Message = 4-byte header (command, flags, count) + count * 164-byte records.
//
// LinkStateRouting.cc previously computed the chunk length as a bare
// B(72) * recordCount, with no message header and a per-record width (72)
// that does not correspond to any real field-by-field encoding of
// TeLinkStateInfo (72 < 113 data bytes -- it looks like a leftover
// placeholder guess predating any serializer). A prior commit corrected that
// formula to B(4) + B(116) * recordCount (format v1) to match this
// serializer's actual byte output (RsvpTe.cc's compute*MessageLength()
// precedent: the serializer is the source of truth, the model's
// setChunkLength() call must match it). This commit grows the per-record
// width to 164 bytes for the D3 TE-attribute fields; LinkStateRouting.cc's
// formula is updated in lockstep. Every MPLS example floods LinkStateMsg at
// startup (MplsRouterBase.ned wires LinkStateRouting unconditionally, and
// LinkStateRouting::initialize() schedules an announce independent of which
// signaling protocol -- LDP or RSVP-TE -- the router actually uses), so all
// fingerprint rows that exercise the MPLS examples/showcases are expected to
// move (both the serialization ingredients from the wider record, and
// possibly `tplx` from the larger packets taking longer to transmit).
// ===========================================================================

namespace {

constexpr int RECORD_BYTES = 164;

// See RsvpTeSerializer.cc: C++17 has no std::bit_cast; memcpy-based
// reinterpretation is the portable way to punch IEEE-754 bits into/out of an
// integer without violating strict aliasing.
template<typename To, typename From>
To bitCast(From from)
{
    static_assert(sizeof(To) == sizeof(From), "bitCast: size mismatch");
    To to;
    std::memcpy(&to, &from, sizeof(To));
    return to;
}

} // namespace

void LinkStateRoutingSerializer::serializeLinkStateInfo(MemoryOutputStream& stream, const TeLinkStateInfo& info)
{
    stream.writeIpv4Address(info.advrouter);
    stream.writeIpv4Address(info.linkid);
    stream.writeIpv4Address(info.local);
    stream.writeIpv4Address(info.remote);
    stream.writeUint64Be(bitCast<uint64_t>(info.metric));
    stream.writeUint64Be(bitCast<uint64_t>(info.MaxBandwidth));
    for (int i = 0; i < 8; i++)
        stream.writeUint64Be(bitCast<uint64_t>(info.UnResvBandwidth[i]));
    stream.writeUint64Be(0); // timestamp: skipped-as-0, see the format note above
    stream.writeUint32Be(info.sourceId);
    stream.writeUint32Be(info.messageId);
    stream.writeByte(info.state ? 1 : 0);
    stream.writeByte(0); // pad
    stream.writeUint16Be(0); // pad
    // --- v2 additions (D3) ---
    stream.writeUint64Be(bitCast<uint64_t>(info.teMetric));
    stream.writeUint32Be(info.adminGroup);
    if ((int)info.srlgsCount > Ted::MAX_SRLGS)
        throw cRuntimeError("LinkStateRoutingSerializer: TeLinkStateInfo.srlgsCount=%u exceeds MAX_SRLGS=%d", info.srlgsCount, Ted::MAX_SRLGS);
    stream.writeUint32Be(info.srlgsCount);
    for (int i = 0; i < Ted::MAX_SRLGS; i++)
        stream.writeUint32Be(i < (int)info.srlgsCount ? info.srlgs[i] : 0);
}

TeLinkStateInfo LinkStateRoutingSerializer::deserializeLinkStateInfo(MemoryInputStream& stream)
{
    TeLinkStateInfo info;
    info.advrouter = stream.readIpv4Address();
    info.linkid = stream.readIpv4Address();
    info.local = stream.readIpv4Address();
    info.remote = stream.readIpv4Address();
    info.metric = bitCast<double>(stream.readUint64Be());
    info.MaxBandwidth = bitCast<double>(stream.readUint64Be());
    for (int i = 0; i < 8; i++)
        info.UnResvBandwidth[i] = bitCast<double>(stream.readUint64Be());
    stream.readUint64Be(); // timestamp: not meaningful across a round trip, discarded
    info.timestamp = SIMTIME_ZERO;
    info.sourceId = stream.readUint32Be();
    info.messageId = stream.readUint32Be();
    info.state = stream.readByte() != 0;
    stream.readByte(); // pad
    stream.readUint16Be(); // pad
    // --- v2 additions (D3) ---
    info.teMetric = bitCast<double>(stream.readUint64Be());
    info.adminGroup = stream.readUint32Be();
    info.srlgsCount = stream.readUint32Be();
    if ((int)info.srlgsCount > Ted::MAX_SRLGS)
        throw cRuntimeError("LinkStateRoutingSerializer: received srlgsCount=%u exceeds MAX_SRLGS=%d -- corrupt or incompatible wire data", info.srlgsCount, Ted::MAX_SRLGS);
    for (int i = 0; i < Ted::MAX_SRLGS; i++) {
        uint32_t value = stream.readUint32Be();
        if (i < (int)info.srlgsCount)
            info.srlgs[i] = value;
    }
    return info;
}

void LinkStateRoutingSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& msg = staticPtrCast<const LinkStateMsg>(chunk);
    size_t count = msg->getLinkInfoArraySize();

    stream.writeByte(static_cast<uint8_t>(msg->getCommand()));
    stream.writeByte(msg->getRequest() ? 0x01 : 0x00); // flags: bit0 = request
    stream.writeUint16Be(static_cast<uint16_t>(count));

    for (size_t i = 0; i < count; i++)
        serializeLinkStateInfo(stream, msg->getLinkInfo(i));
}

const Ptr<Chunk> LinkStateRoutingSerializer::deserialize(MemoryInputStream& stream) const
{
    auto msg = makeShared<LinkStateMsg>();

    msg->setCommand(stream.readByte());
    uint8_t flags = stream.readByte();
    msg->setRequest((flags & 0x01) != 0);
    uint16_t count = stream.readUint16Be();

    msg->setLinkInfoArraySize(count);
    for (uint16_t i = 0; i < count; i++)
        msg->setLinkInfo(i, deserializeLinkStateInfo(stream));

    msg->setChunkLength(B(4) + B(RECORD_BYTES) * count);
    return msg;
}

} // namespace inet
