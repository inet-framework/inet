# INET Framework Improvement Plan

## 1. Code Style Fixes

### 1a. Opening brace on new line for functions

All function definitions should have the opening `{` on a new line (Allman style),
consistent with the dominant INET style:
```cpp
// BEFORE:
void Foo::bar() {
    ...
}

// AFTER:
void Foo::bar()
{
    ...
}
```

**Exception:** Single-line functions (typically inline functions in headers) remain
on one line:
```cpp
int getCount() const { return count; }
```

### 1b. Remove `typedef struct`

Replace C-style `typedef struct { ... } Name;` patterns with plain C++ `struct Name { ... };`.

### 1c. Use appropriate integer types

Use `int32_t` / `uint32_t` only where fixed-width is required (wire formats, serialization).
In general code, use:
- `int` or `long` for general counters and arithmetic
- `size_t` for container sizes and loop indices over containers
- Especially in `for()` loops: prefer `int`, `long`, or `size_t` over `int32_t`

---

## 2. Add WATCH() for All Member Variables

**Goal:** Make every module's internal state inspectable at runtime via the OMNeT++ GUI inspector.

**Details:**
- Use the unified `WATCH()` macro everywhere. OMNeT++ 6.x supports it for all types
  (map, vector, set, list, pointer, etc.), so the old type-specific variants
  (`WATCH_MAP`, `WATCH_VECTOR`, `WATCH_PTR`, `WATCH_LIST`, `WATCH_SET`,
  `WATCH_OBJ`, `WATCH_RW`, `WATCH_EXPR`) are no longer necessary.
- Replace all ~124 occurrences across ~72 files that still use the old variants.
- Audit every module: compare member variables declared in the `.h` file against
  `WATCH()` calls in `initialize()`. Add missing watches for all member variables.
- This is a prerequisite for item 3: once every variable is WATCH-ed, the
  `displayStringTextFormat` mechanism can reference them via `{variableName}` syntax.

**Scope:** ~200+ source files, mechanical but broad.

---

## 3. Unified Display String Text via displayStringTextFormat and WATCH Variables

**Goal:** Every module should display 2 lines of useful runtime information on its icon,
driven by the `displayStringTextFormat` NED parameter, using WATCH-ed variables.

**Details:**

### 3a. Use the existing ModuleMixin mechanism consistently

The infrastructure already exists in `ModuleMixin<T>::refreshDisplay()`:
- It reads the `displayStringTextFormat` NED parameter.
- It calls `StringFormat::formatString()` which resolves `{expression}` tokens
  by looking up WATCH objects via `cCollectObjectsVisitor`.

What needs to change:
- **Keep the existing `resolveDirective()` overrides** — the legacy `%s`, `%r`, `%d`,
  `%b`, etc. single-character directives remain available for backward compatibility
  and as convenient shorthands. New display formats should prefer `{watchName}` syntax
  but the old `%x` directives are not removed.
- **Remove direct `getDisplayString().setTagArg("t", ...)` calls** from
  `refreshDisplay()` in modules that should use the NED-driven mechanism instead
  (currently 50+ files bypass the mechanism).
- **Add meaningful 2-line default values** for `displayStringTextFormat` in every
  module's NED file. Use `{watchName}` syntax. Examples:
  - Ppp: `"rate: {datarate}\nsent: {numSent}, rcvd: {numRcvdOK}"`
  - UdpBasicApp: `"sent: {numSent}\nrcvd: {numReceived}"`
  - Tcp: `"conn: {numConnections}\nsent: {numSent}"`
  - Mobility: `"pos: {lastPosition}\nspd: {lastVelocity}"`

### 3b. Refactor C++ quantity formatting

Many modules manually format quantities with if-else ladders for SI prefix selection.
This code is duplicated and inconsistent. Replace it with the OMNeT++ 6.1 quantity
formatter (or a centralized INET utility function).

**Patterns to eliminate:**

1. **Datarate if-else ladder** (duplicated in `Ppp.cc`, `EthernetMacBase.cc`,
   `ThruputMeteringChannel.cc`):
   ```cpp
   // BEFORE (duplicated 3+ times):
   if (datarate >= 1e9)
       sprintf(buf, "%gGbps", datarate / 1e9);
   else if (datarate >= 1e6)
       sprintf(buf, "%gMbps", datarate / 1e6);
   else if (datarate >= 1e3)
       sprintf(buf, "%gkbps", datarate / 1e3);
   else
       sprintf(buf, "%gbps", datarate);

   // AFTER: single call
   formatQuantity(datarate, "bps");  // -> "100 Mbps"
   ```

2. **SI prefix selection in `Units.h`** for `W`, `Hz`, `bps` types — the `operator<<`
   specializations contain manual if-else chains (marked with `// TODO extract these
   SI prefix printing fallback mechanisms`). Replace with a generic SI prefix
   formatter template.

3. **Byte volume formatting** in `ThruputMeteringChannel.cc` (`B`, `KiB`, `MiB`
   if-ladder).

4. **`long2string` / `double2string` / `ulong2string`** in `.msg` files and generated
   `_m.cc` code (~947 occurrences in ~145 files). Update the `@toString` annotations
   in `.msg` files to use quantity-aware formatting where applicable.

5. **Ad-hoc `/1000`, `/1e6`, `/1000000` divisions** scattered throughout the
   codebase for display purposes.

**The three sub-items are connected:** Once WATCH `str()` output uses proper quantity
formatting, the `{watchName}` display string mechanism automatically shows
human-readable values (e.g. `160 MHz` instead of `1.6e+08`).

---

## 4. Restructure StandardHost and Network Node Hierarchy

**Goal:** Eliminate copy-pasted `StandardHost` / base class content from other network
node definitions. Specialized nodes should inherit from the standard base class chain
and only add or override what is specific to them — never duplicate the base structure.

**Current problems:**
- `LdpMplsRouter` and `RsvpMplsRouter` (~160 lines each) are standalone modules that
  duplicate the entire internal structure (InterfaceTable, MessageDispatchers,
  Ipv4NetworkLayer, tcp, udp, loopback/ppp interfaces, gates, connections) instead
  of inheriting from the base class chain.
- Many node types (`StandardHost6`, `CorrespondentNode6`, `MobileHost6`,
  `WirelessHost6`) are near-copies with minor parameter differences.
- `Router` adds routing protocols but duplicates connection patterns.

**Approach:**
- Use the `@reconnect` NED property to allow derived modules to rewire connections
  (e.g. insert MPLS between network and link layers) without copying the entire
  base module structure.
- Refactor `LdpMplsRouter` and `RsvpMplsRouter` to inherit from the standard
  base class chain and only add/override what is MPLS-specific.
- Consolidate IPv6-specific node types into `StandardHost` with parameters.
- Review all 45 node types under `src/inet/node/` for simplification opportunities.

**Risk:** High — this is a breaking change for user simulations that reference
the old module types. Requires an NED compatibility/alias strategy.

---

## 5. Additional INET-Specific Fingerprint Ingredients

**Goal:** Extend `FingerprintCalculator` with new ingredient characters that hash
protocol-level state, enabling more precise regression testing.

**Current state:** `FingerprintCalculator.h` defines 5 INET-specific ingredients:
`~` (network communication filter), `U` (packet update filter), `N` (network node path),
`I` (network interface path), `D` (packet data).

**Proposed new ingredients:**

| Char | Name | What it hashes |
|------|------|---------------|
| `C` | Network interface configuration | IP addresses, MAC, MTU, up/down, carrier state |
| `R` | Routing table | Route entries: destination, gateway, metric, interface |
| `M` | Mobility state | Position (x,y,z), speed, heading |
| `A` | ARP/neighbor table | MAC-to-IP mappings |
| `Q` | Queue state | Queue length, drop count |

**Key design challenge:** Unlike existing ingredients (which are derived from the
event/packet being delivered), the new ingredients represent **global protocol state**
that changes in modules unrelated to the one receiving the current event. The change
to a routing table or interface configuration happens in one module, but the current
event might be processed in a completely different module. Therefore, the
`addEventIngredient()` approach does not work here.

Furthermore, the `FingerprintCalculator` must **not** be tightly coupled to specific
modules, signals, or data structures (routing tables, interface tables, mobility, etc.).
The knowledge of what constitutes fingerprint-relevant state must reside in the modules
that own that state.

**Implementation approach — dedicated listener modules:**

The responsibility is split into three layers, keeping each one clean:

1. **`FingerprintCalculator`** remains generic — it only exposes a hasher API:
   ```cpp
   void addExtraData(char ingredientChar, const char *data);
   ```
   It checks whether the given ingredient character is active; if so, feeds `data`
   into the hasher. It knows nothing about signals, routing, mobility, etc.

2. **State-owning modules** (`Ipv4RoutingTable`, `InterfaceTable`, mobility modules,
   `Arp`, etc.) remain unchanged — they already emit signals when their state changes
   (see `Simsignals.cc`). No fingerprint-related code is added to them.

3. **Separate dedicated listener modules** bridge the gap. Each one:
   - Subscribes to the relevant signals (e.g. `routeAddedSignal`, `routeChangedSignal`).
   - In the signal callback, serializes the relevant state into a hashable string.
   - Calls `FingerprintCalculator::addExtraData(ingredientChar, ...)`.

   For example:
   - `RoutingTableFingerprintListener` — listens to `routeAdded/Deleted/Changed`
     signals, hashes route entries under ingredient `'R'`.
   - `InterfaceConfigFingerprintListener` — listens to `interfaceConfigChanged`,
     `interfaceStateChanged`, etc., hashes under `'C'`.
   - `MobilityFingerprintListener` — listens to `mobilityStateChanged`, hashes
     position/velocity under `'M'`.
   - `ArpFingerprintListener` — listens to ARP table changes, hashes under `'A'`.

   These listener modules can be optionally included in network nodes (or at the
   network level), controlled by NED parameters or by the active ingredient characters.

**Benefits of this approach:**
- **No coupling in any direction:** The FingerprintCalculator knows nothing about
  domain-specific state. The state-owning modules know nothing about fingerprinting.
  The listener modules are the only ones that know both sides.
- **State-owning modules stay clean:** No fingerprint code pollutes `Ipv4RoutingTable`,
  `InterfaceTable`, mobility modules, etc.
- **Extensible:** Adding a new ingredient = adding a new small listener module.
  No changes needed in FingerprintCalculator or in existing protocol modules.
- **Optional:** Listener modules are only instantiated when the corresponding
  ingredient is needed — zero overhead otherwise.

---

## 6. Naming Convention and Systematic Renaming

**Goal:** Establish and enforce a consistent naming convention for all named entities
in INET: modules, parameters, gates, signals, statistics, labels, C++ classes,
and member variables.

**Current inconsistencies (examples):**

**Process (4 phases):**
1. **Naming convention document** — write a comprehensive style guide (with AI help)
   covering all entity types.
2. **Rename mapping** — produce an `old name → new name` table for every affected
   entity, with impact analysis.
3. **Rename tool** — build/use an automated tool that handles NED, MSG, C++, and INI
   files, including string literals, `@class` annotations, and fingerprint baselines.
4. **Execute rename** — run the tool, update all tests, verify fingerprints.

**Risk:** Very high — affects the entire codebase and all downstream user projects.
Requires a deprecation/migration strategy.

---

