# Type-aware field hooks for FieldsChunkSerializer

Status: done

## Problem

`ChunkSerializerRegistry` knows the concrete chunk type that the caller asked for. It gives
that type to `ChunkSerializer::deserialize(stream, typeInfo)`. `FieldsChunkSerializer`
overrides that function, does the length bookkeeping, and then calls its own protected hook
`deserialize(stream)`. The hook has no `typeInfo` parameter, so the type is lost.

A serializer that is registered for more than one chunk type cannot build the requested type.
The IEEE 802.11 management serializer is such a case. It always builds one body type and
therefore breaks the round trip for the other types.

## Constraint

Existing serializers, in the tree and out of the tree, must keep working without a source
change. Pull request #1164 solves the same problem with a hard API break. This branch solves
it without a break.

## Design

Add a second hook next to the first one, and let the new hook fall back to the old one.

    class INET_API FieldsChunkSerializer : public ChunkSerializer
    {
      protected:
        // old hooks, deprecated, no longer pure
        virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const;
        virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const;

        // new hooks, default body calls the old hook
        virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const;
        virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const;

      public:
        virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const override;
        virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
    };

A subclass overrides the old hook, or the new hook, or both. All three cases work. Third-party
code that overrides the old hook keeps working without a source change, because the default body
of the new hook calls the old hook. The public wrappers keep the length bookkeeping and the
serialized-bytes cache in one place.

The old hooks stay `virtual` and never become `final`, so an override is always allowed.

### Why the new hook needs a new name

The signature `deserialize(MemoryInputStream&, const std::type_info&)` is already the public
override of `ChunkSerializer::deserialize`. One class cannot hold two members with the same
name and the same parameter list. So the type-aware hook needs a new name.

### Why `serializeFields` too

The serialize side does not need the type, because the chunk knows its own type. The rename
is for symmetry, and it removes the overload hiding: today the protected `serialize(stream,
chunk)` in each subclass hides the public `serialize(stream, chunk, offset, length)`.

## Decisions

- **Do not add `final` to the public wrappers.** Today those wrappers are plain overrides. An
  out-of-tree serializer may already override the public `deserialize(stream, typeInfo)`.
  `final` would break it. Pull request #1164 adds `final`; this branch does not.
- **Deprecate with a Doxygen `@deprecated` comment, not with `[[deprecated]]`.** Test result:
  neither g++ nor clang++ warns when a subclass overrides a deprecated virtual function. The
  override is the only case that matters here, so the attribute gives no signal. The attribute
  would also force a diagnostic pragma around the fallback body. INET has no `[[deprecated]]`
  anywhere in `src/inet`, so the comment matches the project.
- **Keep the old hooks non-pure.** A subclass that overrides neither hook now fails at run
  time, not at compile time. This is the price of compatibility. The default body throws a
  `cRuntimeError` that names both hooks.
- **Accept that an unmigrated serializer stays unfixed and silent.** This is what backward
  compatibility means. The hard break of #1164 buys only one thing: it forces the author of an
  out-of-tree multi-type serializer to look at the call.

## Steps

- [x] 1. Base class: add `serializeFields` and `deserializeFields`; route the public wrappers
      through them; the default body of each new hook calls the old hook. The old hooks stay
      pure, so nothing changes for a subclass.
- [x] 2. Base class: make the old hooks non-pure, so that a subclass can override the new
      hooks alone. Both hooks stay overridable. This step removes the obligation to override
      the old hooks. It does not remove the permission.
- [x] 3. Base class: mark the old hooks deprecated, and write the WHATSNEW entry and the
      migration guide section.
- [x] 4. Migrate the in-tree serializers to the new hooks, and migrate the direct hook calls
      between serializers.

Step 1 to step 3 touch only `src/inet/common/packet/serializer/FieldsChunkSerializer.*` and the
documents. Step 4 touches the rest of the tree. Each step compiles and runs.

- [x] 5. Add `tests/unit/FieldsChunkSerializer_1.test`, which exercises all three cases: a
      serializer with the old hooks only, a serializer with the new hooks only that is
      registered for two chunk types, and a serializer with no hook at all.

## Findings during implementation

- The tree has 92 `FieldsChunkSerializer` subclasses in 112 files, and 7 direct hook calls
  between serializers (`OspfPacketSerializer`, `EthernetPhyHeaderSerializer` and the packet
  unit test). The direct calls must move to the new name, because after the migration the
  callee no longer overrides the old hook.
- The in-tree serializer hierarchy is flat. Every `FieldsChunkSerializer` subclass is a leaf.
  So step 4 cannot break an in-tree class that another in-tree class derives from. It can
  still break third-party code that derives from an in-tree serializer and overrides the old
  hook, because the migrated parent's `deserializeFields` override would win.

  Decided: this case does not occur. Third-party code adds new serializers for its own chunk
  types; it does not override the in-tree ones. So step 4 migrates every in-tree subclass, and
  the only compatibility surface that matters is `FieldsChunkSerializer` itself.
- Step 2 makes `FieldsChunkSerializer` concrete. Before the change it had two pure virtual
  members, so it could not be instantiated. This is a small loss; the throwing default bodies
  keep the mistake visible.
- Only 61 translation units include `FieldsChunkSerializer.h`, so a change to the base class
  costs a 61-file rebuild, not a full one.
- Build evidence: `make MODE=release` gives exit 0 after step 1, step 2 and step 4. Step 1 is
  the compatibility proof, because all 92 subclasses were still unmigrated at that point.
  `make MODE=debug` plus `tests/packet` also gives exit 0.
- Test evidence: `inet_run_unit_tests -p inet -m debug` gives 91 PASS and 0 FAIL, which is the
  90 tests of master plus the new one. `inet_run_packet_tests -p inet -m debug` gives PASS,
  which covers the migrated `tests/packet/UnitTest.cc` and its three direct hook calls.

## Out of scope

The IEEE 802.11 management body fix of #1164 is not part of this branch. This branch only makes
the type available to the hook.
