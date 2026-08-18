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

// --- coverage audit: per chunk class, which wire fields a filler actually set. Every
// wire field it visits is recorded in g_allWire and the ones carrying a value a fresh
// default does not is recorded in g_filled; their difference is what the coverage gate
// reports, i.e. the fields a filler forgot (or knowingly leaves alone -- those go on
// ChunkRoundTripConfig::knownUnfilled). The audit only READS through the class
// descriptors, so it needs no @editable/@fromString annotation on the model. ---
bool g_reportUnfilled = getenv("CHUNK_RTT_NO_UNFILLED") == nullptr;
std::map<std::string, std::set<std::string>> g_allWire; // every wire-field path seen
std::map<std::string, std::set<std::string>> g_filled;  // paths whose value differs from a fresh default

bool isWireField(cClassDescriptor *desc, int f)
{
    const char *pd = desc->getFieldProperty(f, "packetData");
    if (pd != nullptr && strcmp(pd, "false") == 0)
        return false; // @packetData(false): simulation-internal state, not wire data
    const char *fn = desc->getFieldName(f);
    // cObject/cNamedObject-synthesized getters (no @packetData) and the base Chunk's
    // byte-dump/info renderings are not wire content either -- keep them out of the report.
    // ... and neither are the cObject bookkeeping members a compound field of a cObject
    // subclass (a cArray of reception reports, ...) brings into the field list.
    static const char *const notWire[] = {"rawBin", "rawHex", "info", "name", "fullName", "fullPath", "className",
                                          "owner", "takeOwnership", "isOwnedObject"};
    for (const char *n : notWire)
        if (strcmp(fn, n) == 0)
            return false;
    return true;
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

// After the fill, walk the chunk against a fresh default instance and record (per class)
// every wire-field path seen, plus those the filler actually set -- a value the default
// does not carry, an array grown or given different elements, a null pointer made
// non-null, or (the "meaningful default" rule below) a field whose default value is
// already a real, non-0/-1 value that a corrupt serialization would change. What the two
// sets differ in is what the coverage gate reports as unfilled. Guarded: never throws.
void collectFilled(any_ptr a, any_ptr fr, cClassDescriptor *desc, const std::string& prefix,
        int depth, const std::string& className)
{
    if (desc == nullptr || depth > 6)
        return;
    for (int f = 0; f < desc->getFieldCount(); f++) {
        if (!isWireField(desc, f))
            continue;
        std::string path = prefix + desc->getFieldName(f);
        // the type is carried along for the human-readable report only; the gate keys on
        // the path alone (everything before the " : ")
        g_allWire[className].insert(path + " : " + desc->getFieldTypeString(f));
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
                    // the members now stand for the compound in the report: keep the gate
                    // pointing at the exact member left unset, not at its container too
                    g_allWire[className].erase(path + " : " + desc->getFieldTypeString(f));
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

// Run one round-trip case: instantiate `name`, apply commonSetup and the filler,
// then serialize -> deserialize -> serialize and compare. Records the outcome under
// `label` into passes/failures/crashes and recovers from a SIGSEGV/SIGABRT in the
// serializer.
void runCase(const std::string& name, cObjectFactory *factory, const ChunkFiller *r,
        const ChunkRoundTripConfig& config, std::vector<std::string>& passes,
        std::vector<std::string>& failures, std::vector<std::string>& crashes)
{
    std::string label = name + (!r->label.empty() ? " [" + r->label + "]" : "");
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
        g_phase = "fill";
        if (config.commonSetup)
            config.commonSetup(chunk.get());
        r->fill(chunk.get());

        // coverage: record which wire fields the filler set, by diffing the finished
        // chunk against a fresh default. Best-effort -- a failure to build the reference
        // must not fail the round-trip case.
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
        // a chunk may declare a bit-granular length, while the byte stream can only end on
        // an octet boundary, so the serialized size must be the chunkLength rounded up
        else if ((int64_t)b1.size() != (chunk->getChunkLength().get<b>() + 7) / 8) {
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

    // index fillers by type name -- a type may have several (variants)
    std::map<std::string, std::vector<const ChunkFiller *>> fillerMap;
    for (const auto& r : config.fillers) {
        if (!r.fill)
            throw cRuntimeError("chunk filler for '%s' has no fill function", r.typeName.c_str());
        fillerMap[r.typeName].push_back(&r);
    }

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
        // one case per filler (variants). A type with no filler is untested: fail rather
        // than pass silently, so a chunk type added to INET cannot slip in uncovered --
        // either write its filler (lib/fillers/) or put it on skipTypes with a reason.
        auto it = fillerMap.find(name);
        if (it == fillerMap.end()) {
            failures.push_back(name + ": no filler (add one in lib/fillers/, or skip-list it)");
            continue;
        }
        for (const ChunkFiller *r : it->second)
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
        // Real gaps = every wire field seen minus the ones a filler (or commonSetup) set,
        // i.e. the fields still carrying their default value after all fillers ran. With
        // variants a field filled by any one variant counts as covered.
        std::map<std::string, std::set<std::string>> realGaps;
        const std::set<std::string> noneFilled;
        for (const auto& e : g_allWire) {
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
            // "//" starts a comment line, so the list can be grouped by reason
            if (b != std::string::npos && ln.compare(b, 2, "//") != 0)
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
    // *types*, OK/FAILED/CRASHED count *cases*. A type with several filler variants
    // contributes one run type but several cases, so cases >= run types (= registered - skipped).
    EV_WARN << "Types:  " << total << " registered, " << (total - (long)skips.size())
            << " run, " << skips.size() << " skipped\n";
    EV_WARN << "Cases:  " << cases << " run  OK: " << passes.size()
            << "  FAILED: " << failures.size() << "  CRASHED: " << crashes.size() << "\n";
}

} // namespace inet
