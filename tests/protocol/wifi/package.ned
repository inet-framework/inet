//
// Conformance-oriented WiFi protocol test suite (up to & including WiFi 5).
//
// Self-contained sibling of tests/protocol/lib: its own NED package root, its own
// build (build.sh -> wifitests executable) and its own omnetpp.ini. It references the
// framework in ../lib (the ProtocolTester NED type + libprotocoltest.so) but never
// modifies it.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
package inet.protocoltest.wifi;

@namespace(inet::protocoltest);
