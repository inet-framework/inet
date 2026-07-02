//
// Conformance-oriented IPv6 protocol test suite: Neighbor Discovery (RFC 4861)
// and Duplicate Address Detection / SLAAC (RFC 4862).
//
// Self-contained sibling of tests/protocol/lib and tests/protocol/wifi: its own NED
// package root, its own build (run-tests.sh -> ipv6tests executable) and its own inis.
// It references the framework in ../lib (the ProtocolTester NED type +
// libprotocoltest.so) but never modifies it.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
package inet.protocoltest.ipv6;

@namespace(inet::protocoltest);
