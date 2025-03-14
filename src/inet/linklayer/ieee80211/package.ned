//
// This package contains a comprehensive implementation of the IEEE 802.11 wireless LAN protocol 
// family, supporting multiple PHY variants (a, b, g, n, ac, ax) and MAC layer features. The 
// implementation includes modules for the MAC layer (DCF, PCF, HCF with EDCA and HCCA), LLC 
// sublayer, management entities, and interfaces to the physical layer. It provides support for 
// infrastructure and ad hoc modes, QoS features, frame aggregation, block acknowledgment, 
// protection mechanisms, and various rate control algorithms. The modular architecture allows 
// for easy extension and customization of different aspects of the protocol.
//
// @see ~inet.linklayer.ieee80211.mac, ~inet.linklayer.ieee80211.mgmt, ~inet.linklayer.ieee80211.llc
//
package inet.linklayer.ieee80211;

@namespace(inet::ieee80211);
