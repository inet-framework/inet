//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.linklayer.ieee80211.mac.Ieee80211Frame;

namespace inet::ieee80211;

// IEEE Std 802.11-2024, 9.4.2.96 and Tables 9-279--9-283.
enum MeshPathMetricId
{
    MESH_AIRTIME_METRIC = 1;
    MESH_HIGH_PHY_RATE_AIRTIME_METRIC = 2;
}

cplusplus {{
// IEEE Std 802.11-2024, 9.4.1.4/Figure 9-140 and 9.4.2.3/Table 9-131.
constexpr uint16_t MESH_EPD_CAPABILITY = 1 << 13;
constexpr uint8_t MESH_EPD_MEMBERSHIP_SELECTOR = 0x80 | 124;
// IEEE Std 802.11-2024, 9.2.4.8.1/Table9-34 and9.3.3.1:
// non-HT management Frame Body2304 octets minus Category/MeshAction2.
const B MAX_MESH_ACTION_BODY_LENGTH = B(2302);
}}

enum MeshActionCategory
{
    MESH_ACTION = 13;
    MESH_SELF_PROTECTED_ACTION = 15;
}

enum MeshAction
{
    MESH_LINK_METRIC_REPORT = 0;
    MESH_HWMP_PATH_SELECTION = 1;
}

enum MeshPeeringAction
{
    MESH_PEERING_OPEN = 1;
    MESH_PEERING_CONFIRM = 2;
    MESH_PEERING_CLOSE = 3;
}

struct MeshConfiguration
{
    uint8_t pathSelectionProtocolId = 1;
    uint8_t pathSelectionMetricId = MESH_AIRTIME_METRIC;
    uint8_t congestionControlId = 0;
    uint8_t synchronizationId = 1;
    uint8_t authenticationId = 0;
    uint8_t formationInfo = 0;
    uint8_t capability = 9; // Accept additional peerings; forwarding enabled.
}

// IEEE Std 802.11-2024, 9.4.2.100. Presence depends on the peering action.
struct MeshPeering
{
    uint16_t protocolId = 0;
    uint16_t localLinkId = 0;
    bool peerLinkIdPresent = false;
    uint16_t peerLinkId = 0;
    bool reasonCodePresent = false;
    uint16_t reasonCode = 0;
    bool chosenPmkPresent = false;
    uint8_t chosenPmk[16];
}

// IEEE Std 802.11-2024, 9.4.2.5: compressed partial virtual bitmap.
struct MeshTim
{
    uint8_t dtimCount = 0;
    uint8_t dtimPeriod = 1;
    uint8_t bitmapControl = 0;
    uint16_t bitmapLength = 1;
    uint8_t partialVirtualBitmap[251];
}

// Common optional mesh elements in beacons/probes and mesh action bodies.
// Defaults add no bytes to existing non-mesh frames.
class Ieee80211MeshFrame extends FieldsChunk
{
    bool meshIdPresent = false;
    uint8_t meshId[];
    bool meshConfigurationPresent = false;
    MeshConfiguration meshConfiguration;
    bool meshPeeringPresent = false;
    MeshPeering meshPeering;
    bool timPresent = false;
    MeshTim tim;
    bool meshAwakeWindowPresent = false;
    uint16_t meshAwakeWindow = 0; // TUs
    uint8_t opaqueMeshElements[]; // complete optional action IEs; retained for profile/security admission
}

struct HwmpPreqTarget
{
    uint8_t flags = 0; // TO: bit 0; USN: bit 2.
    MacAddress address;
    uint32_t sequenceNumber = 0;
}

// IEEE Std 802.11-2024, 9.4.2.111. At most 20 targets.
class HwmpPreq
{
    @packetData;
    uint8_t flags = 0;
    uint8_t hopCount = 0;
    uint8_t ttl = 0;
    uint32_t discoveryId = 0;
    MacAddress originator;
    uint32_t originatorSequenceNumber = 0;
    MacAddress originatorExternalAddress; // present iff flags bit 6 is 1
    uint32_t lifetime = 0; // TUs
    uint32_t metric = 0; // units of the mesh-wide metric ID
    HwmpPreqTarget targets[];
}

struct HwmpPrep
{
    uint8_t flags = 0;
    uint8_t hopCount = 0;
    uint8_t ttl = 0;
    MacAddress target;
    uint32_t targetSequenceNumber = 0;
    MacAddress targetExternalAddress; // present iff flags bit 6 is 1
    uint32_t lifetime = 0; // TUs
    uint32_t metric = 0;
    MacAddress originator;
    uint32_t originatorSequenceNumber = 0;
}

struct HwmpPerrDestination
{
    uint8_t flags = 0;
    MacAddress address;
    uint32_t sequenceNumber = 0;
    MacAddress externalAddress; // present iff flags bit 6 is 1
    uint16_t reasonCode = 0;
}

class HwmpPerr
{
    @packetData;
    uint8_t ttl = 0;
    HwmpPerrDestination destinations[];
}

struct HwmpRann
{
    uint8_t flags = 0;
    uint8_t hopCount = 0;
    uint8_t ttl = 0;
    MacAddress root;
    uint32_t sequenceNumber = 0;
    uint32_t interval = 0; // TUs
    uint32_t metric = 0;
}

// The MAC/action identity is a separate, always parseable 26-byte header.
// A malformed variable body never suppresses the MAC's normal receive/ACK path.
class Ieee80211MeshActionFrame extends Ieee80211ActionFrame
{
    chunkLength = B(26);
    category = MESH_ACTION;
    uint8_t meshAction = MESH_HWMP_PATH_SELECTION;
}

class Ieee80211HwmpFrame extends Ieee80211MeshFrame
{
    HwmpPreq preqs[];
    HwmpPrep preps[];
    HwmpPerr perrs[];
    HwmpRann ranns[];
    uint8_t unmodelledElements[]; // complete optional IEs, including ID/Length
}

class Ieee80211MeshLinkMetricFrame extends Ieee80211MeshFrame
{
    bool request = false;
    uint32_t metric = 0; // both supported metric IDs encode unsigned 32-bit values
}

class Ieee80211MeshPeeringOpenFrame extends Ieee80211MeshFrame
{
    uint16_t capabilityInformation = 0; // includes the EPD capability subfield
    uint8_t supportedRates[]; // wire-coded rates and BSS membership selectors
    uint8_t extendedSupportedRates[];
}

class Ieee80211MeshPeeringConfirmFrame extends Ieee80211MeshPeeringOpenFrame
{
    uint16_t associationId = 0; // distinct from either Link ID
}

class Ieee80211MeshPeeringCloseFrame extends Ieee80211MeshFrame
{
}
