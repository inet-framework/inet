# Sealing — status

The authoritative list of **sealed** paths. The rules that govern sealing — what "sealed" means, the
audit-before-seal workflow, how to add and remove entries — are in [sealing.md](sealing.md).

**Default: unsealed.** Every file in the repository is freely modifiable unless it matches an entry
below. Paths are relative to `src/inet/`. An entry is either:

- a **file** (no trailing slash) — that one file is sealed; or
- a **folder** (trailing `/`) — **recursive**: every file under it, at any depth, including files
  added later.

A file is sealed iff it is listed or lives under a listed folder. `🔒` marks each entry for scanning;
absence from this list *is* the unsealed state — there are no `⬜` rows to maintain.

## Sealed paths

### Packet subsystem — the packet API

- 🔒 `common/packet/` *(recursive)* — the packet/chunk API and its implementation, the umbrella
  behind [`common/packet/PacketAPI.h`](../../src/inet/common/packet/PacketAPI.h): chunks
  (`chunk/`, incl. `ChunkAPI.h`), `Packet`, `ChunkBuffer`, `ChunkQueue`, `Message`, the reassembly/
  reorder buffers and `PacketFilter`, plus the region-tag, serializer, dissector, printer, and
  recorder subtrees. Frozen as a unit: this is INET's most-depended-upon value-type surface, and its
  representation, chunk algebra, and introspection contracts are settled — new packet files land
  under this seal by default.

<!--
To seal more, append a group heading and rows here in review order, e.g.:

### Core definitions
- 🔒 `common/INETDefs.h`
- 🔒 `common/Units.h`

Seal a whole subsystem with a trailing-slash folder entry; seal an individual file with a bare path.
Add the entry in the same commit that records the file's compliant state (see sealing.md).
-->
