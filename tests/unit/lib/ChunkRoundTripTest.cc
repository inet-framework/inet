//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "ChunkRoundTripTest.h"

#include <csetjmp>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "inet/common/MemoryInputStream.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

namespace {

// Turns EV logging off for its lifetime (serializers may log raw bytes on odd
// input; keep them off stdout). Exceptions still propagate.
struct LogSilencer
{
    LogLevel saved;
    LogSilencer() : saved(cLog::logLevel) { cLog::logLevel = LOGLEVEL_OFF; }
    ~LogSilencer() { cLog::logLevel = saved; }
};

// Escape non-printable bytes so diagnostic text stays valid UTF-8 for opp_test.
std::string sanitize(const std::string& s)
{
    std::string out;
    for (unsigned char c : s) {
        if (c >= 0x20 && c < 0x7f)
            out += (char)c;
        else {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\x%02x", c);
            out += buf;
        }
    }
    return out;
}

// A capped, space-separated hex dump of a byte buffer, so a failing chunk can be
// inspected/reproduced.
std::string hexDump(const std::vector<uint8_t>& bytes, size_t maxBytes = 48)
{
    std::string out;
    char buf[4];
    size_t n = bytes.size() < maxBytes ? bytes.size() : maxBytes;
    for (size_t i = 0; i < n; i++) {
        snprintf(buf, sizeof(buf), "%02x ", bytes[i]);
        out += buf;
    }
    if (bytes.size() > maxBytes)
        out += "... (+" + std::to_string(bytes.size() - maxBytes) + " B)";
    return out;
}

// A chunk length in bytes-and-bits, flagging the -1 "unset" sentinel that most
// "offset is out of range" failures come from.
std::string lenStr(b length)
{
    int64_t bits = length.get<b>();
    if (bits < 0)
        return "unset(-1)";
    return std::to_string(bits / 8) + "B" + (bits % 8 ? "+" + std::to_string(bits % 8) + "b" : "");
}

bool nameLooksLikeSize(const std::string& n)
{
    // *LengthField / *lengthField are the real on-the-wire length fields -- fill them like
    // any scalar (a recipe overrides the few that must equal the serialized length).
    if (n.size() >= 5 && n.compare(n.size() - 5, 5, "Field") == 0)
        return false;
    // more fields the substring match wrongly catches: ordinary data, not length drivers --
    // a hop count, a prefix length, an advertised (external) payload length, the
    // Ethernet/802.1 typeOrLength (an EtherType or a length, serialized as a plain uint16),
    // and the RTCP length word (written and read back verbatim, never validated against the
    // serialized size).
    if (n == "hopCount" || n == "homeNetworkPrefixLength" || n == "payloadLength" ||
            n == "typeOrLength" || n == "rtcpLength")
        return false;
    // other fields that drive the serialized length -- filling them arbitrarily makes
    // the serialized size disagree with chunkLength (assertion); leave at default
    auto has = [&](const char *s) { return n.find(s) != std::string::npos; };
    return has("ength") || has("Length") || has("ount") || has("Count") ||
           has("ArraySize") || n == "chunkLength" || has("headerLength") || has("dataLength");
}

bool isUnsignedType(const std::string& t)
{
    return t.find("unsigned") != std::string::npos || t.find("uint") != std::string::npos ||
           (!t.empty() && t[0] == 'u');
}

int fieldWidthBits(cClassDescriptor *desc, int field, const std::string& type)
{
    const char *bit = desc->getFieldProperty(field, "bit");
    if (bit != nullptr)
        return atoi(bit);
    if (type == "char" || type == "signed char" || type == "unsigned char" ||
        type == "int8_t" || type == "uint8_t")
        return 8;
    if (type == "short" || type == "short int" || type == "unsigned short" ||
        type == "int16_t" || type == "uint16_t")
        return 16;
    if (type == "int64_t" || type == "uint64_t" || type == "long" || type == "long long" ||
        type == "unsigned long" || type == "unsigned long long")
        return 64;
    return 32; // int / unsigned int / int32_t / uint32_t / enums
}

// A distinct, byte-asymmetric numeric value that fits the field's width: each
// byte is a distinct ascending counter value, so its byte-reversal differs
// (catches endianness) and adjacent fields differ (catches swaps/shifts).
std::string numericValue(cClassDescriptor *desc, int field, const std::string& type, unsigned& counter)
{
    int wbits = fieldWidthBits(desc, field, type);
    int usable = isUnsignedType(type) ? wbits : wbits - 1; // leave the sign bit for signed types
    if (usable > 62) usable = 62;
    if (usable < 1) usable = 1;
    unsigned c = counter;
    counter += 4;
    uint64_t v = 0;
    int nbytes = (usable + 7) / 8;
    for (int k = 0; k < nbytes; k++)
        v = (v << 8) | ((c + k) & 0xFF);
    v &= (usable >= 63) ? ((uint64_t(1) << 62) - 1) : ((uint64_t(1) << usable) - 1);
    if (v == 0)
        v = 1;
    return std::to_string(v);
}

// Concrete address value fields (MacAddress / Ipv4Address / Ipv6Address) need a
// syntactically valid address literal: the generic scalar path would feed a plain
// integer string, which the type's @fromString cannot parse, so the field would stay
// at its all-zero default -- meaning an address swap or a misplaced-address bug would
// not be caught (both sides stay zero). Now that these types are @editable with a
// @fromString, set them through the descriptor's string setter with a distinct,
// byte-asymmetric literal. This is the only path that works uniformly: the @primitive
// (@byValue) MacAddress / Ipv4Address fields have no getFieldStructValuePointer (it
// returns nullptr for a by-value field), while an @opaque Ipv6Address one does --
// setFieldValueAsString covers both. Returns true if @p f is an address field it filled.
//
// L3Address is polymorphic (holds an IPv4 / IPv6 / MAC / module-path address). It is
// @editable, so left to the generic scalar path its @fromString would get a plain integer
// string -- which L3Address does NOT reject but silently parses as a MODULEID address,
// breaking a serializer that expects an IPv4/IPv6 family. Default it to an IPv4 address
// here (the common case); where the context demands another family the recipe overrides it
// afterwards with a literal of the right type (see the AODV *_IPv6 recipes, and fillRerr).
//
// Scalar (non-array) fields only; an address vector, whose type string also matches the
// element type, has no string setter, so the set throws and it is left to the array path.
bool fillAddressField(cClassDescriptor *desc, any_ptr ap, int f, unsigned& counter)
{
    std::string type = desc->getFieldTypeString(f);
    // Give every byte of the address a distinct, ascending counter value, so a
    // byte-reversal of the field differs from the original (catches endianness) and
    // adjacent address fields differ (catches swaps) -- mirroring numericValue(). Distinct
    // ascending bytes also make the address non-zero (never the all-zero default).
    unsigned c = counter & 0xFF;
    auto byte = [c](int k) { return (c + k) & 0xFF; };
    char buf[48];
    int used;
    if (type.find("MacAddress") != std::string::npos) {
        snprintf(buf, sizeof(buf), "%02x-%02x-%02x-%02x-%02x-%02x",
                 byte(0), byte(1), byte(2), byte(3), byte(4), byte(5));
        used = 6;
    }
    else if (type.find("Ipv4Address") != std::string::npos || type.find("L3Address") != std::string::npos) {
        snprintf(buf, sizeof(buf), "%u.%u.%u.%u", byte(0), byte(1), byte(2), byte(3)); // L3Address defaults to IPv4
        used = 4;
    }
    else if (type.find("Ipv6Address") != std::string::npos) {
        snprintf(buf, sizeof(buf), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                 byte(0), byte(1), byte(2), byte(3), byte(4), byte(5), byte(6), byte(7),
                 byte(8), byte(9), byte(10), byte(11), byte(12), byte(13), byte(14), byte(15));
        used = 16;
    }
    else
        return false;
    try {
        desc->setFieldValueAsString(ap, f, 0, buf);
    }
    catch (const std::exception&) {
        // an address vector field's type string also matches the element type but it has
        // no string setter -- leave it to the array path, like any other array
        return false;
    }
    counter += used;
    return true;
}

// --- coverage diagnostic: record, per chunk class, the wire fields the fill leaves
// untouched and why, so coverage gaps are visible and can be triaged (per-chunk recipe
// vs. generalizable). Always collected; printed unless CHUNK_RTT_NO_UNFILLED is set.
// (A later step turns this into a pass/fail gate against a known-gaps allow-list.) ---
bool g_reportUnfilled = getenv("CHUNK_RTT_NO_UNFILLED") == nullptr;
std::map<std::string, std::set<std::string>> g_unfilled; // generic-fill gaps: "path : type [reason]"
std::map<std::string, std::set<std::string>> g_filled;   // paths whose value differs from a fresh default

bool isWireField(cClassDescriptor *desc, int f)
{
    const char *pd = desc->getFieldProperty(f, "packetData");
    if (pd != nullptr && strcmp(pd, "false") == 0)
        return false; // @packetData(false): simulation-internal state, not wire data
    const char *fn = desc->getFieldName(f);
    // cObject/cNamedObject-synthesized getters (no @packetData) and the base Chunk's
    // byte-dump/info renderings are not wire content either -- keep them out of the report.
    static const char *const notWire[] = {"rawBin", "rawHex", "info", "name", "fullName", "fullPath", "className"};
    for (const char *n : notWire)
        if (strcmp(fn, n) == 0)
            return false;
    return true;
}

void noteUnfilled(cObject *obj, cClassDescriptor *desc, int f, const std::string& prefix, const char *reason)
{
    if (!isWireField(desc, f))
        return;
    g_unfilled[obj->getClassName()].insert(
            prefix + desc->getFieldName(f) + " : " + desc->getFieldTypeString(f) + "  [" + reason + "]");
}

// An enum member name that marks an unset/undefined value rather than a real one.
bool isSentinelEnumName(const char *name)
{
    // clear "unset" markers only -- NONE/UNKNOWN are excluded, they are real values in some
    // enums (IP_PROT_NONE = IPv6 no-next-header, ETHERTYPE_UNKNOWN, ...).
    std::string n(name);
    auto has = [&](const char *s) { return n.find(s) != std::string::npos; };
    return has("UNDEF") || has("UNSPEC") || has("INVALID") || has("UNSET");
}

// True if the enum field holds a meaningful value: a member of the enum whose name is not a
// sentinel (a field at ..._UNDEFINED / -1 / a value outside the set is not meaningfully set).
// Falls back to the non-0/-1 rule when the enum is not in the registry.
bool enumValueIsMeaningful(cClassDescriptor *desc, any_ptr obj, int f, const char *enumName)
{
    omnetpp::intval_t v;
    try { v = desc->getFieldValue(obj, f, 0).intValue(); }
    catch (const std::exception&) { return false; }
    omnetpp::cEnum *e = omnetpp::cEnum::find(enumName);
    if (e == nullptr)
        return v != 0 && v != -1;
    const char *name = e->getStringFor(v);
    return name != nullptr && !isSentinelEnumName(name);
}

// Fill an object's flat scalar fields with distinct values, plus scalar address fields
// (via fillAddressField). Pointer / array and size-driving fields, discriminator
// bools/enums, and the checksum/FCS mode are left at their defaults (a recipe or
// commonSetup handles those where a round trip needs them). reportObj/prefix feed the
// unfilled-fields coverage diagnostic only. recurse descends into nested compound members
// (opt-in: it changes the serialized size and can reach nested discriminators, so it is
// off for the default flat fill and enabled only via RecursiveFiller); depth bounds it.
void fillFields(any_ptr ap, cClassDescriptor *desc, unsigned& counter,
        cObject *reportObj, const std::string& prefix, int depth, bool recurse)
{
    if (desc == nullptr)
        return;
    // The coverage report must reflect the last (deepest) fill applied to a chunk, not
    // the earlier flat pass: a chunk filled flat first and then by a RecursiveFiller would
    // otherwise still list the compound the recipe went on to fill. Drop the prior record
    // at the start of each top-level fill so only this pass's gaps remain.
    if (depth == 0 && reportObj != nullptr)
        g_unfilled.erase(reportObj->getClassName());
    for (int f = 0; f < desc->getFieldCount(); f++) {
        std::string name = desc->getFieldName(f);
        if (desc->getFieldIsArray(f) || desc->getFieldIsPointer(f)) {
            noteUnfilled(reportObj, desc, f, prefix, desc->getFieldIsArray(f) ? "array" : "pointer");
            continue;
        }
        // address fields are editable but only accept a valid address literal; fill
        // them here before the generic path would feed them an unparseable integer
        if (fillAddressField(desc, ap, f, counter))
            continue;
        if (desc->getFieldIsCompound(f)) {
            // descend into the nested struct (only when recurse is set) instead of leaving
            // it at its default; a @byValue compound yields no pointer
            // (getFieldStructValuePointer == null), and cObject subclasses (cArray, ...)
            // have no useful wire fields -- record those as unfilled compound instead.
            const char *sn = desc->getFieldStructName(f);
            cClassDescriptor *sd = (recurse && sn != nullptr && depth < 4) ? cClassDescriptor::getDescriptorFor(sn) : nullptr;
            any_ptr sp(nullptr);
            if (sd != nullptr) {
                try { sp = desc->getFieldStructValuePointer(ap, f, 0); }
                catch (const std::exception&) { sp = any_ptr(nullptr); }
            }
            if (sd != nullptr && sp != nullptr)
                fillFields(sp, sd, counter, reportObj, prefix + name + ".", depth + 1, recurse);
            else
                noteUnfilled(reportObj, desc, f, prefix, "compound");
            continue;
        }
        if (!desc->getFieldIsEditable(f)) {
            noteUnfilled(reportObj, desc, f, prefix, "non-editable");
            continue;
        }
        if (nameLooksLikeSize(name)) {
            noteUnfilled(reportObj, desc, f, prefix, "size");
            continue;
        }
        std::string type = desc->getFieldTypeString(f);
        // An enum sitting at its 0/-1 default gets a valid non-0/-1 member from the cEnum
        // registry, so data enums (status codes, ...) carry a meaningful value. An enum
        // already at a non-default value is a subtype discriminator pinned by the .msg or a
        // recipe -- leave it, since another member would make the serializer reinterpret the
        // object as a sibling type. The few discriminators that default to 0/-1 and break
        // when filled are pinned back by their per-type recipe. (checksum/FCS mode is a
        // non-editable meta field handled by commonSetup.)
        if (const char *enumName = desc->getFieldProperty(f, "enum")) {
            // fill an enum at a sentinel/undefined (or out-of-set) value with a real,
            // non-sentinel member so it becomes meaningful; a meaningful one (a pinned
            // discriminator) is left alone. collectFilled credits whatever ends up meaningful.
            if (!enumValueIsMeaningful(desc, ap, f, enumName)) {
                if (omnetpp::cEnum *e = omnetpp::cEnum::find(enumName))
                    for (const auto& kv : e->getNameValueMap())
                        if (!isSentinelEnumName(kv.first.c_str())) {
                            try { desc->setFieldValueAsString(ap, f, 0, kv.first.c_str()); } catch (const std::exception&) {}
                            break;
                        }
            }
            noteUnfilled(reportObj, desc, f, prefix, "enum");
            continue;
        }
        std::string val;
        if (type == "string")
            val = std::string("s") + std::to_string(counter++);
        else if (type == "bool")
            val = (counter++ & 1) ? "true" : "false"; // alternate so adjacent flags differ (catches swaps)
        else if (type == "double" || type == "float")
            val = std::to_string(1.0 + counter++);
        else if (type == "omnetpp::simtime_t" || type == "simtime_t" || type.find("clocktime_t") != std::string::npos)
            val = std::to_string(1 + (counter++ & 7)) + "ms"; // small valid time; fits narrow wire encodings
        else
            val = numericValue(desc, f, type, counter);
        try {
            desc->setFieldValueAsString(ap, f, 0, val.c_str());
        }
        catch (const std::exception&) {
            // field type does not accept this string representation (e.g. a strictly
            // parsed address/enum); leave it at its default
            noteUnfilled(reportObj, desc, f, prefix, "threw");
        }
    }
}

void fillObject(cObject *obj, unsigned& counter)
{
    cClassDescriptor *desc = obj->getDescriptor();
    if (desc == nullptr)
        return;
    fillFields(toAnyPtr(obj), desc, counter, obj, "", 0, /*recurse=*/false);
}

// Recursively fill an object's nested compound members too (the flat fill leaves them at
// their defaults). Exposed for RecursiveFiller.
void fillObjectDeep(cObject *obj, unsigned& counter)
{
    cClassDescriptor *desc = obj->getDescriptor();
    if (desc == nullptr)
        return;
    fillFields(toAnyPtr(obj), desc, counter, obj, "", 0, /*recurse=*/true);
}

// A filled chunk can drive a serializer into a hard crash (SIGSEGV) or an
// assertion (SIGABRT) -- e.g. an unchecked dynamicPtrCast on a discriminator
// flag. Those can't be caught as C++ exceptions, so we recover with a signal
// handler + siglongjmp to keep the run going and record the offender. This is a
// probe harness; leaked state after recovery is acceptable (the process exits
// right after). NOTE: siglongjmp skips C++ destructors, so anything the recovery
// path relies on (here: the log level) must be restored by hand.
sigjmp_buf g_recover;
volatile sig_atomic_t g_active = 0;
// which step is running, so a SIGSEGV/SIGABRT (which skips normal exception
// handling) can still report where it happened; volatile so it survives the
// siglongjmp back into the recovery block.
const char *volatile g_phase = "";

void crashHandler(int sig)
{
    if (g_active)
        siglongjmp(g_recover, sig);
    signal(sig, SIG_DFL); // not inside a probe: restore default and re-raise
    raise(sig);
}

} // anonymous namespace

// Measure a FieldsChunk's natural serialized length from the serializer's own
// length-check report and set it as the chunkLength. Serialize once with an
// over-large declared length: the serializer writes N bytes and
// FieldsChunkSerializer throws "serialized=N", which we read back. No probing,
// no guessing. setChunkLength() clears the cached serialization via handleChange().
// Leaves the length unset if it cannot be measured (non-FieldsChunk, a self-sizing
// chunk that accepts the over-large length, or a body that fails for another
// reason) -- self-sizing chunks are given an explicit length by the caller instead.
void measureAndSetChunkLength(Chunk *c)
{
    Ptr<Chunk> chunk = c->shared_from_this();
    auto fc = dynamicPtrCast<FieldsChunk>(chunk);
    if (fc == nullptr)
        return;
    fc->setChunkLength(B(65536)); // large, so the offset range-check passes
    try {
        MemoryOutputStream tmp;
        Chunk::serialize(tmp, chunk);
        fc->setChunkLength(b(-1)); // accepted the over-large length: not measurable this way
    }
    catch (const std::exception& e) {
        std::string w = e.what();
        auto pos = w.find("serialized=");
        fc->setChunkLength(pos != std::string::npos
                ? b(atol(w.c_str() + pos + std::strlen("serialized=")))
                : b(-1));
    }
}

void setChunkLength(Chunk *c, b bits)
{
    if (auto fc = dynamic_cast<FieldsChunk *>(c))
        fc->setChunkLength(bits);
}

std::function<void(Chunk *)> SimpleFiller(std::vector<std::pair<std::string, std::string>> fields,
        b length)
{
    return [fields = std::move(fields), length](Chunk *c) {
        cClassDescriptor *desc = c->getDescriptor();
        any_ptr ap = toAnyPtr(c);
        for (const auto& fv : fields) {
            int f = desc != nullptr ? desc->findField(fv.first.c_str()) : -1;
            if (f >= 0) {
                try {
                    desc->setFieldValueAsString(ap, f, 0, fv.second.c_str());
                }
                catch (const std::exception&) {
                    // field rejected the string; leave it (surfaces downstream)
                }
            }
        }
        if (length < b(0))
            measureAndSetChunkLength(c);
        else
            setChunkLength(c, length);
    };
}

std::function<void(Chunk *)> RecursiveFiller(std::vector<std::pair<std::string, std::string>> fields,
        b length)
{
    return [fields = std::move(fields), length](Chunk *c) {
        unsigned counter = 0x41;
        fillObjectDeep(c, counter); // fill nested compound members the flat fill left empty
        // pin any explicit fields after the deep fill, so a discriminator the recursion
        // would clobber is restored (same semantics as SimpleFiller's field list)
        cClassDescriptor *desc = c->getDescriptor();
        any_ptr ap = toAnyPtr(c);
        for (const auto& fv : fields) {
            int f = desc != nullptr ? desc->findField(fv.first.c_str()) : -1;
            if (f >= 0) {
                try {
                    desc->setFieldValueAsString(ap, f, 0, fv.second.c_str());
                }
                catch (const std::exception&) {
                    // field rejected the string; leave it (surfaces downstream)
                }
            }
        }
        // deep filling changed the serialized size, so measure it back by default
        if (length < b(0))
            measureAndSetChunkLength(c);
        else
            setChunkLength(c, length);
    };
}

namespace {

// Compare two chunks of the same type field by field through the descriptor, and
// append "field: 'a' != 'b'" lines for each difference. Skips fields marked
// @packetData(false) in the .msg -- the non-wire, simulation-internal state (chunk
// id, chunkLength, mutable/complete/correct flags, region tags, ...), which the
// base Chunk already annotates. Everything else is wire data and must round-trip;
// a difference is a real finding. Recurses into compound fields and arrays.
void diffFields(any_ptr a, any_ptr b, cClassDescriptor *desc, const std::string& prefix,
        int depth, std::vector<std::string>& diffs)
{
    if (desc == nullptr || depth > 6)
        return;
    for (int f = 0; f < desc->getFieldCount(); f++) {
        const char *fn = desc->getFieldName(f);
        // fields not part of the wire content: @packetData(false) marks the
        // simulation-internal state (chunk id, chunkLength, flags, region tags, ...);
        // additionally skip the base Chunk's derived byte-dump renderings (rawBin/
        // rawHex -- packet data, but the same bytes we already compare directly) and
        // the getInfo() display string (redundant with the concrete field diffs).
        const char *pd = desc->getFieldProperty(f, "packetData");
        if ((pd != nullptr && strcmp(pd, "false") == 0)
                || strcmp(fn, "rawBin") == 0 || strcmp(fn, "rawHex") == 0 || strcmp(fn, "info") == 0)
            continue;
        std::string fname = prefix + fn;
        bool isArray = desc->getFieldIsArray(f);
        int na, nb;
        try {
            na = isArray ? desc->getFieldArraySize(a, f) : 1;
            nb = isArray ? desc->getFieldArraySize(b, f) : 1;
        }
        catch (const std::exception&) {
            continue;
        }
        if (na != nb) {
            diffs.push_back(fname + ": array size " + std::to_string(na) + " != " + std::to_string(nb));
            continue;
        }
        for (int i = 0; i < na; i++) {
            std::string idx = isArray ? "[" + std::to_string(i) + "]" : "";
            try {
                if (desc->getFieldIsCompound(f)) {
                    const char *sn = desc->getFieldStructName(f);
                    cClassDescriptor *sd = sn != nullptr ? cClassDescriptor::getDescriptorFor(sn) : nullptr;
                    any_ptr sa = desc->getFieldStructValuePointer(a, f, i);
                    any_ptr sb = desc->getFieldStructValuePointer(b, f, i);
                    if (sa != nullptr && sb != nullptr && sd != nullptr)
                        diffFields(sa, sb, sd, fname + idx + ".", depth + 1, diffs);
                    else if ((sa != nullptr) != (sb != nullptr))
                        diffs.push_back(fname + idx + ": one side null");
                }
                else {
                    std::string va = desc->getFieldValueAsString(a, f, i);
                    std::string vb = desc->getFieldValueAsString(b, f, i);
                    if (va != vb)
                        diffs.push_back(fname + idx + ": '" + sanitize(va) + "' != '" + sanitize(vb) + "'");
                }
            }
            catch (const std::exception&) {
                // field not comparable this way; skip
            }
        }
    }
}

// After all fills, mark (per class) the wire-field paths whose value differs from a
// fresh default instance -- i.e. fields something actually filled, be it the generic
// fill, commonSetup, or a recipe. The coverage report subtracts these from the recorded
// generic-fill gaps, so a gap a recipe fills stops counting (a pointer set by a lambda,
// the checksum mode set by commonSetup, a discriminator pinned by SimpleFiller, ...).
// Only fields already recorded as generic-fill gaps are affected, so a bool the generic
// fill set to its default value is never mistaken for unfilled. Guarded: never throws.
void collectFilled(any_ptr a, any_ptr fr, cClassDescriptor *desc, const std::string& prefix,
        int depth, const std::string& className)
{
    if (desc == nullptr || depth > 6)
        return;
    for (int f = 0; f < desc->getFieldCount(); f++) {
        if (!isWireField(desc, f))
            continue;
        std::string path = prefix + desc->getFieldName(f);
        try {
            if (desc->getFieldIsArray(f)) {
                int na = desc->getFieldArraySize(a, f), nf = desc->getFieldArraySize(fr, f);
                if (na != nf)
                    g_filled[className].insert(path); // elements added
                else {
                    // same size (e.g. a fixed-size array): its content may still be filled --
                    // an element that differs from the default, or (like the numeric rule) a
                    // numeric element already holding a meaningful non-0/-1 value (e.g. the BGP
                    // marker's 0xFF bytes), counts as covered
                    for (int i = 0; i < na; i++) {
                        std::string ea = desc->getFieldValueAsString(a, f, i);
                        if (ea != desc->getFieldValueAsString(fr, f, i)) {
                            g_filled[className].insert(path);
                            break;
                        }
                        char *end = nullptr;
                        long long v = strtoll(ea.c_str(), &end, 10);
                        if (end != ea.c_str() && *end == '\0' && v != 0 && v != -1) {
                            g_filled[className].insert(path);
                            break;
                        }
                    }
                }
            }
            else if (desc->getFieldIsPointer(f)) {
                if ((desc->getFieldStructValuePointer(a, f, 0) != nullptr)
                        != (desc->getFieldStructValuePointer(fr, f, 0) != nullptr))
                    g_filled[className].insert(path); // default null -> set non-null
            }
            else if (desc->getFieldIsCompound(f)) {
                const char *sn = desc->getFieldStructName(f);
                cClassDescriptor *sd = sn != nullptr ? cClassDescriptor::getDescriptorFor(sn) : nullptr;
                any_ptr sa = desc->getFieldStructValuePointer(a, f, 0);
                any_ptr sfr = desc->getFieldStructValuePointer(fr, f, 0);
                if (sd != nullptr && sa != nullptr && sfr != nullptr) {
                    std::set<std::string>& filled = g_filled[className];
                    size_t before = filled.size();
                    collectFilled(sa, sfr, sd, path + ".", depth + 1, className);
                    if (filled.size() > before)
                        filled.insert(path); // a nested field was filled -> the compound is too
                }
                else if (desc->getFieldValueAsString(a, f, 0) != desc->getFieldValueAsString(fr, f, 0))
                    // a @byValue compound (SequenceNumberCyclic, ...) yields no pointer to
                    // recurse into; compare it by value like a scalar to see a filler's change
                    g_filled[className].insert(path);
            }
            else {
                std::string va = desc->getFieldValueAsString(a, f, 0); // if the chunk value is unreadable -> outer catch -> stays a gap
                std::string vf;
                bool freshReadable = true;
                try { vf = desc->getFieldValueAsString(fr, f, 0); }
                catch (const std::exception&) { freshReadable = false; }
                if (!freshReadable)
                    // the default is an unreadable sentinel (e.g. SequenceNumberCyclic's -1,
                    // whose accessor asserts value != -1) but the chunk holds a readable value:
                    // a filler must have set it
                    g_filled[className].insert(path);
                else if (va != vf)
                    g_filled[className].insert(path); // a filler changed it
                else {
                    // a field already holding a meaningful value -- neither 0 nor the -1
                    // "unset" sentinel -- is effectively covered even at its default, since it
                    // serializes a non-trivial value a corruption would change. Numeric fields
                    // render as the number; enums render as a symbol, so read their numeric
                    // value from the descriptor (a type/kind discriminator pinned by a concrete
                    // subclass is covered this way).
                    char *end = nullptr;
                    long long v = strtoll(va.c_str(), &end, 10);
                    bool meaningful = end != va.c_str() && *end == '\0' && v != 0 && v != -1;
                    // an enum holding a valid, non-sentinel member is covered even at value 0
                    // (e.g. a discriminator pinned to ECHO_REPLY / SYNC / a config type)
                    if (!meaningful) {
                        if (const char *en = desc->getFieldProperty(f, "enum"))
                            meaningful = enumValueIsMeaningful(desc, a, f, en);
                    }
                    if (meaningful)
                        g_filled[className].insert(path);
                }
            }
        }
        catch (const std::exception&) {
            // field not comparable this way -- leave it (stays a gap, the conservative choice)
        }
    }
}

// Run one round-trip case: instantiate `name`, generic-fill it, apply commonSetup
// and (if given) the recipe, then serialize -> deserialize -> serialize and
// compare. Records the outcome under `label` into passes/failures/crashes and
// recovers from a SIGSEGV/SIGABRT in the serializer.
void runCase(const std::string& name, cObjectFactory *factory, const ChunkRecipe *r,
        const ChunkRoundTripConfig& config, std::vector<std::string>& passes,
        std::vector<std::string>& failures, std::vector<std::string>& crashes)
{
    std::string label = name + (r != nullptr && !r->label.empty() ? " [" + r->label + "]" : "");
    Chunk *chunkRaw = nullptr;
    try {
        chunkRaw = dynamic_cast<Chunk *>(factory->createOne());
    }
    catch (const std::exception&) {
        failures.push_back(label + ": createOne threw");
        return;
    }
    if (chunkRaw == nullptr) {
        failures.push_back(label + ": not a Chunk");
        return;
    }
    Ptr<Chunk> chunk = chunkRaw->shared_from_this();

    int sig = 0;
    g_active = 1;
    g_phase = "setup";
    if ((sig = sigsetjmp(g_recover, 1)) != 0) {
        // recovered from a SIGSEGV/SIGABRT during the round-trip below
        g_active = 0;
        cLog::logLevel = LOGLEVEL_WARN; // LogSilencer dtor was skipped by longjmp
        const char *ph = g_phase;
        crashes.push_back(label + ": CRASHED (" + (sig == SIGABRT ? "assert/abort" : "SIGSEGV")
                + ") during " + ph);
        return;
    }
    std::vector<uint8_t> b1; // serialized bytes; out here so the catch can dump them
    try {
        LogSilencer silence;
        unsigned counter = 0x11;
        g_phase = "fill";
        fillObject(chunk.get(), counter);
        if (config.commonSetup)
            config.commonSetup(chunk.get());
        if (r != nullptr && r->fill)
            r->fill(chunk.get());

        // coverage: record which recorded generic-fill gaps a filler actually set, by
        // diffing the finished chunk against a fresh default. Best-effort -- a failure to
        // build the reference must not fail the round-trip case.
        g_phase = "coverage";
        try {
            if (Chunk *freshRaw = dynamic_cast<Chunk *>(factory->createOne())) {
                Ptr<Chunk> fresh = freshRaw->shared_from_this();
                collectFilled(toAnyPtr(chunk.get()), toAnyPtr(fresh.get()),
                        chunk->getDescriptor(), "", 0, chunk->getClassName());
            }
        }
        catch (const std::exception&) {
        }

        g_phase = "serialize";
        {
            MemoryOutputStream s;
            Chunk::serialize(s, chunk);
            b1 = s.getData();
        }

        g_phase = "deserialize";
        MemoryInputStream in(b1);
        const auto& chunk2 = Chunk::deserialize(in, typeid(*chunk.get()));

        g_phase = "re-serialize";
        // Drop the deserialization byte cache so serialize() re-encodes from the parsed
        // fields instead of replaying the bytes it just read (see FieldsChunkSerializer).
        // Without this, chunk2 replays b1 verbatim, b2 == b1 always, the diff below never
        // runs, and a serialize()-only asymmetry passes silently. chunk2 is the sole owner
        // of a freshly deserialized (mutable) chunk.
        if (auto fc = dynamic_cast<FieldsChunk *>(chunk2.get())) {
            fc->markMutableIfExclusivelyOwned();
            fc->handleChange(); // clears serializedData
        }
        MemoryOutputStream out2;
        Chunk::serialize(out2, chunk2);
        std::vector<uint8_t> b2 = out2.getData();

        if (b1 != b2) {
            size_t off = 0;
            while (off < b1.size() && off < b2.size() && b1[off] == b2[off])
                off++;
            std::string msg = label + ": round-trip bytes differ at byte offset " + std::to_string(off)
                    + " (serialized " + std::to_string(b1.size()) + " B, re-serialized " + std::to_string(b2.size()) + " B)"
                    + "\n      serialized:    " + hexDump(b1)
                    + "\n      re-serialized: " + hexDump(b2);
            // point at the culprit: which wire field the deserialize changed
            std::vector<std::string> diffs;
            diffFields(toAnyPtr(chunk.get()), toAnyPtr(chunk2.get()), chunk->getDescriptor(), "", 0, diffs);
            for (const auto& d : diffs)
                msg += "\n      field diff: " + d;
            failures.push_back(msg);
        }
        else if (B(b1.size()) != chunk->getChunkLength()) {
            failures.push_back(label + ": serialized length (" + std::to_string(b1.size())
                    + " B) != chunkLength (" + lenStr(chunk->getChunkLength()) + ")"
                    + "\n      serialized: " + hexDump(b1));
        }
        else {
            passes.push_back(label);
            g_active = 0;
            return;
        }
    }
    catch (const std::exception& e) {
        std::string msg = label + ": EXCEPTION during " + g_phase
                + " (chunkLength=" + lenStr(chunk->getChunkLength()) + "): " + sanitize(e.what());
        if (!b1.empty())
            msg += "\n      serialized-so-far (" + std::to_string(b1.size()) + " B): " + hexDump(b1);
        failures.push_back(msg);
    }
    g_active = 0;
}

} // anonymous namespace

void testChunkRoundTrips(const ChunkRoundTripConfig& config)
{
    cLog::logLevel = LOGLEVEL_WARN;
    auto& registry = ChunkSerializerRegistry::getInstance();
    const auto& names = registry.getRegisteredTypeNames();

    // Serializer *class*-level coverage: the round trip instantiates every non-skipped type,
    // so a serializer class not invoked by the end is code no type exercises -- either an
    // abstract-base serializer whose concrete subclasses use a different class, or a
    // structural chunk not round-tripped standalone.
    std::set<std::string> usedSerializerClasses;
    registry.setUsedSerializerClassRecorder(&usedSerializerClasses);

    // index recipes by type name -- a type may have several (variants)
    std::map<std::string, std::vector<const ChunkRecipe *>> recipeMap;
    for (const auto& r : config.recipes)
        recipeMap[r.typeName].push_back(&r);

    struct sigaction sa;
    sa.sa_handler = crashHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;
    struct sigaction oldSegv, oldAbrt;
    sigaction(SIGSEGV, &sa, &oldSegv);
    sigaction(SIGABRT, &sa, &oldAbrt);

    int total = 0;
    std::vector<std::string> passes;
    std::vector<std::string> failures;
    std::vector<std::string> crashes;
    std::vector<std::string> skips;

    for (const auto& name : names) {
        total++;
        if (config.skipTypes.count(name) != 0) {
            skips.push_back(name + " (config skip-list)");
            continue;
        }
        cObjectFactory *factory = cObjectFactory::find(name.c_str());
        if (factory == nullptr || factory->isAbstract()) {
            skips.push_back(name + " (abstract / no factory)");
            continue;
        }
        // no recipe -> one default case (generic fill only); one or more recipes ->
        // one case each (variants) with the recipe's fields and chunkLength
        auto it = recipeMap.find(name);
        if (it == recipeMap.end())
            runCase(name, factory, nullptr, config, passes, failures, crashes);
        else
            for (const ChunkRecipe *r : it->second)
                runCase(name, factory, r, config, passes, failures, crashes);
    }

    sigaction(SIGSEGV, &oldSegv, nullptr);
    sigaction(SIGABRT, &oldAbrt, nullptr);

    // detail lists first, then the summary last, so the counts are at the very end
    // of the (potentially long) output.
    EV_WARN << "\n--- PASSED ---\n";
    for (const auto& s : passes)
        EV_WARN << "  " << s << "\n";
    EV_WARN << "\n--- FAILED ---\n";
    for (const auto& s : failures)
        EV_WARN << "  " << s << "\n";
    if (!crashes.empty()) {
        EV_WARN << "\n--- CRASHED (serializer null-deref/assert on the filled chunk) ---\n";
        for (const auto& s : crashes)
            EV_WARN << "  " << s << "\n";
    }
    EV_WARN << "\n--- SKIPPED ---\n";
    for (const auto& s : skips)
        EV_WARN << "  " << s << "\n";

    if (g_reportUnfilled) {
        // Real gaps = the generic-fill gaps minus the fields something actually filled
        // (their value differs from a fresh default). This subtracts recipe/commonSetup
        // fills -- a pointer a lambda set, the checksum mode, a pinned discriminator --
        // so the report reflects what is genuinely still unfilled after every filler ran.
        // (Only generic-fill gaps are considered, so a bool the fill set to its default
        // value is never counted as a gap.)
        std::map<std::string, std::set<std::string>> realGaps;
        const std::set<std::string> noneFilled;
        for (const auto& e : g_unfilled) {
            auto fit = g_filled.find(e.first);
            const std::set<std::string>& filled = fit != g_filled.end() ? fit->second : noneFilled;
            for (const auto& s : e.second)
                if (filled.find(s.substr(0, s.find(" : "))) == filled.end())
                    realGaps[e.first].insert(s);
        }
        EV_WARN << "\n--- UNFILLED WIRE FIELDS (still at default after all fillers; per chunk) ---\n";
        size_t nchunks = 0;
        for (const auto& e : realGaps) {
            if (e.second.empty())
                continue;
            nchunks++;
            EV_WARN << "  " << e.first << ":\n";
            for (const auto& s : e.second)
                EV_WARN << "      " << s << "\n";
        }
        EV_WARN << "  (" << nchunks << " chunk types with >= 1 unfilled wire field)\n";

        // Coverage gate: diff the real gaps against the known-gaps allow-list. An
        // unlisted gap (a new field, or one a change stopped filling) is a coverage
        // regression and fails the test; a listed gap no longer seen is stale (prune it).
        std::set<std::string> known;
        const std::string& text = config.knownUnfilled;
        for (size_t i = 0; i < text.size();) {
            size_t nl = text.find('\n', i);
            if (nl == std::string::npos) nl = text.size();
            std::string ln = text.substr(i, nl - i);
            size_t b = ln.find_first_not_of(" \t\r"), e = ln.find_last_not_of(" \t\r");
            if (b != std::string::npos)
                known.insert(ln.substr(b, e - b + 1));
            i = nl + 1;
        }
        std::set<std::string> current;
        for (const auto& c : realGaps)
            for (const auto& s : c.second)
                current.insert(c.first + "::" + s.substr(0, s.find(" : ")));
        std::vector<std::string> unexpected, stale;
        for (const auto& k : current)
            if (known.find(k) == known.end()) unexpected.push_back(k);
        for (const auto& k : known)
            if (current.find(k) == current.end()) stale.push_back(k);
        if (!unexpected.empty()) {
            EV_WARN << "\n--- UNEXPECTED unfilled wire fields (not on the known-gaps allow-list) ---\n";
            for (const auto& k : unexpected)
                EV_WARN << "  " << k << "\n";
        }
        if (!stale.empty()) {
            EV_WARN << "\n--- STALE allow-list entries (now filled -- prune them) ---\n";
            for (const auto& k : stale)
                EV_WARN << "  " << k << "\n";
        }
        EV_WARN << "\nCoverage gate: " << unexpected.size() << " unexpected, " << stale.size() << " stale\n";
    }

    registry.setUsedSerializerClassRecorder(nullptr);
    const auto& registeredClasses = registry.getRegisteredSerializerClassNames();
    std::vector<std::string> allowedUninvoked, unexpectedUninvoked;
    for (const auto& cls : registeredClasses) { // std::set -> sorted
        if (usedSerializerClasses.find(cls) != usedSerializerClasses.end())
            continue;
        if (config.skipSerializerClasses.find(cls) != config.skipSerializerClasses.end())
            allowedUninvoked.push_back(cls);
        else
            unexpectedUninvoked.push_back(cls);
    }
    std::vector<std::string> staleSkip; // allow-listed classes that are now invoked
    for (const auto& cls : config.skipSerializerClasses)
        if (usedSerializerClasses.find(cls) != usedSerializerClasses.end())
            staleSkip.push_back(cls);
    EV_WARN << "\n--- SERIALIZER CLASSES NEVER INVOKED (code no round-tripped type exercised) ---\n";
    EV_WARN << "  " << usedSerializerClasses.size() << " of " << registeredClasses.size()
            << " serializer classes invoked; " << allowedUninvoked.size() << " allow-listed, "
            << unexpectedUninvoked.size() << " unexpected\n";
    if (!unexpectedUninvoked.empty()) {
        EV_WARN << "\n--- UNEXPECTED untested serializer classes (not on skipSerializerClasses) ---\n";
        for (const auto& cls : unexpectedUninvoked)
            EV_WARN << "  " << cls << "\n";
    }
    if (!staleSkip.empty()) {
        EV_WARN << "\n--- STALE serializer-class skips (now invoked -- prune them) ---\n";
        for (const auto& cls : staleSkip)
            EV_WARN << "  " << cls << "\n";
    }
    EV_WARN << "\nSerializer-class gate: " << unexpectedUninvoked.size() << " unexpected, "
            << staleSkip.size() << " stale\n";

    // counts come from the list sizes so they always match the printed lists. With
    // variants a type contributes several cases, so OK+FAILED+CRASHED counts cases
    // (>= Registered - Skipped), while Registered/Skipped count types.
    long cases = (long)(passes.size() + failures.size() + crashes.size());
    EV_WARN << "\n=== Chunk round-trip coverage ===\n";
    // Two dimensions that do NOT add up to the same total: registered/skipped/run count
    // *types*, OK/FAILED/CRASHED count *cases*. A type with several recipe variants
    // contributes one run type but several cases, so cases >= run types (= registered - skipped).
    EV_WARN << "Types:  " << total << " registered, " << (total - (long)skips.size())
            << " run, " << skips.size() << " skipped\n";
    EV_WARN << "Cases:  " << cases << " run  OK: " << passes.size()
            << "  FAILED: " << failures.size() << "  CRASHED: " << crashes.size() << "\n";
}

} // namespace inet
