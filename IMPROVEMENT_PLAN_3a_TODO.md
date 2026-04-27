# 3a TODO: displayStringTextFormat migration remaining cases

## Already migrated (clean cases)

| Module | Format string |
|--------|--------------|
| UdpBasicApp | `rcvd: {numReceived} pks\nsent: {numSent} pks` |
| UdpBasicBurst | `rcvd: {numReceived} pks\nsent: {numSent} pks` |
| UdpSink | `rcvd: {numReceived} pks` |
| EthernetSocketIo | `rcvd: {numReceived} pks\nsent: {numSent} pks` |
| Ieee8022LlcSocketIo | `rcvd: {numReceived} pks\nsent: {numSent} pks` |
| IpSocketIo | `rcvd: {numReceived} pks\nsent: {numSent} pks` |
| TcpGenericServerApp | `rcvd: {msgsRcvd} pks {bytesRcvd} bytes\nsent: {msgsSent} pks {bytesSent} bytes` |
| EthernetEncapsulation | `passed up: {totalFromMAC}\nsent: {totalFromHigherLayer}` |
| ExtEthernetSocket | `device: {device}\nsnt: {numSent} rcv: {numReceived}` |
| ExtEthernetTapDevice | `TAP device: {device}\nrcv: {numReceived} snt: {numSent}` |
| ExtIpv4Socket | `snt: {numSent} rcv: {numReceived}` |
| ExtIpv4TunDevice | `TUN device: {device}\nrcv: {numReceived} snt: {numSent}` |

---

## TODO: Function-call output (needs resolveDirective() or WATCH of str-convertible type)

These modules display a string produced by a helper function (enum→string, socket
state name, etc.). The raw WATCH value would show an integer, not a human-readable
name. Options: add `resolveDirective()` override, or add a `std::string` member that
mirrors the state name and is kept in sync.

| Module | Current display | Problem |
|--------|----------------|---------|
| `TcpAppBase` | `TcpSocket::stateName(socket.getState())` | function call |
| `SctpClient` | `SctpSocket::stateName(socket.getState())` | function call |
| `DhcpClient` | `getStateName(clientState)` | enum→string function; `clientState` IS WATCHed but str() gives int |
| `AarfRateControl` | `currentMode->getName()` | pointer dereference |
| `OnoeRateControl` | `currentMode->getName()` | pointer dereference |

---

## TODO: Complex computed state (involves conditionals, containers, or computed text)

These modules build their display string from multiple fields including container
sizes, conditional formatting, or runtime-computed text that cannot be expressed
with `{watchName}` tokens alone.

| Module | Problem |
|--------|---------|
| `Tcp` | Large state machine display; iterates `tcpConnMap`; express-mode guard |
| `TcpLwip` | Same pattern as Tcp |
| `Sctp` | Implementation is `#if 0`-ed out; needs complete rework |
| `TcpSessionApp` | Derives TcpAppBase (socket state) + bytesSent/bytesRcvd |
| `TcpEchoApp` | `socketMap.size()` (container) + bytes |
| `TcpSinkApp` / `TcpSinkAppThread` | Thread count + socket state |
| `TcpServerHostApp` / `TcpServerThreadBase` | `socketMap.size()` + socket state |
| `NetPerfMeter` | Iterates `SenderStatisticsMap`; computes totals at display time |
| `BehaviorAggregateClassifier` | Conditional: hides text when `numRcvd == 0` |
| `XMac` | FSM state machine with 11 setTagArg calls in a switch |
| `BMac` | FSM state machine (6 cases) |
| `Ieee802154Mac` | FSM state machine |
| `Ieee80211Mib` | Multi-field: address + SSID + mode + BSS type + QoS + association |
| `Rx` (ieee80211) | Medium busy/free + NAV list |
| `Tx` (ieee80211) | Current frame name + state name |
| `Dcf` / `Hcf` | Frame sequence history string (conditionally shows or removes tag) |
| `Dcaf` / `Edcaf` | Ownership + contention state |
| `Contention` | `updateDisplayString()` — called from non-refreshDisplay path |
| `InProgressFrames` | `inProgressFrames.size()` (container) |
| `EthernetCsmaMac` | FSM state + stringstream |
| `EthernetCsmaPhy` | FSM state name |
| `EthernetPlca` | Two FSM state names + node ID info |
| `InterfaceTable` | `getNumInterfaces()` — method call, not a WATCH-ed member |

---

## TODO: Sets OTHER modules' display strings (not applicable as-is)

These modules write to the display string of *other* modules (NIC interfaces,
switch module, gates) rather than their own `"t"` tag. The `displayStringTextFormat`
mechanism only applies to the module's own display string and cannot be used here.
Requires a different approach (e.g. dedicated visualizer or keeping the manual code).

| Module | What it sets |
|--------|-------------|
| `Mrp` | NIC module display strings (port role/state) |
| `StpBase` | Switch module icon + text; NIC module text; gate link colors |
| `InterfaceTableCanvasVisualizer` | Gate display strings |

---

## TODO: Special cases

| Module | Problem |
|--------|---------|
| `ThruputMeteringChannel` | It's a `cChannel`, not a `cModule`; `displayStringTextFormat` parameter and `ModuleMixin` do not apply |
| `NodeStatus` | Sets both `"t"` (text) and `"i2"` (icon overlay) in the same `refreshDisplay()`; the text part could use the mechanism but the icon coloring must stay |
| `SctpNatPeer` / `SctpPeer` | `setTagArg("t",...)` called from `socketDataArrived()` and `setStatusString()`, not from `refreshDisplay()` — restructuring needed |
| `TrafficgenSimple` | `setStatusString()` called from many places; display driven by state transitions, not a periodic refresh |
