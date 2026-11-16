//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.dtls;

import inet.common.SimpleModule;
import inet.transportlayer.contract.IDtls;

// Models DTLS 1.3 over UDP. Cryptographic content and verification are
// abstracted: this model cannot provide actual confidentiality or integrity.
simple Dtls extends SimpleModule like IDtls
{
    parameters:
        @class(dtls::Dtls);
        @display("i=block/transport");
        string localAddress = default("");
        int localPort = default(-1);
        bool cookieRequired = default(true);
        bool offerInitialKeyShare = default(true);
        bool clientAuthentication = default(false);
        bool resumptionEnabled = default(false); // cache/issue modeled stateful tickets
        bool earlyDataEnabled = default(false); // Requires resumptionEnabled; explicit application admission only
        int earlyDataBytes @unit(B) = default(4096B); // Allowance in newly issued tickets when enabled
        int maxSessions = default(64); // aggregate client/server ticket entries per engine
        double ticketLifetime @unit(s) = default(3600s); // integral seconds, 1..604800
        @signal[sessionResumed](type=long);
        string profile = default("psk");
        string cipherSuites = default("TLS_AES_128_GCM_SHA256"); // Ordered allowed suites, server preference wins
        string keyExchangeGroups = default("x25519");
        string signatureSchemes = default("ecdsa_secp256r1_sha256");
        volatile double aes128GcmHandshakeInDelay @unit(s) = default(handshakeInDelay);
        volatile double aes128GcmHandshakeOutDelay @unit(s) = default(handshakeOutDelay);
        volatile double aes128GcmProtectInDelay @unit(s) = default(protectInDelay);
        volatile double aes128GcmProtectOutDelay @unit(s) = default(protectOutDelay);
        volatile double aes256GcmHandshakeInDelay @unit(s) = default(handshakeInDelay);
        volatile double aes256GcmHandshakeOutDelay @unit(s) = default(handshakeOutDelay);
        volatile double aes256GcmProtectInDelay @unit(s) = default(protectInDelay);
        volatile double aes256GcmProtectOutDelay @unit(s) = default(protectOutDelay);
        volatile double chacha20HandshakeInDelay @unit(s) = default(handshakeInDelay);
        volatile double chacha20HandshakeOutDelay @unit(s) = default(handshakeOutDelay);
        volatile double chacha20ProtectInDelay @unit(s) = default(protectInDelay);
        volatile double chacha20ProtectOutDelay @unit(s) = default(protectOutDelay);
        string identity = default("inet");
        int certificateBytes @unit(B) = default(512B);
        int signatureBytes @unit(B) = default(64B);
        int udpPayloadLimit @unit(B) = default(1200B);
        int maxAssociations = default(64);
        int maxHalfOpen = default(16);
        int maxReassemblyBytes @unit(B) = default(65536B);
        int maxPendingBytes @unit(B) = default(262144B);
        int maxProcessingBytes @unit(B) = default(1048576B);
        int replayWindowSize = default(64);
        int rekeyAfterRecords = default(8388608);
        double handshakeTimeout @unit(s) = default(120s);
        double cookieLifetime @unit(s) = default(60s);
        double closeTimeout @unit(s) = default(2s);
        volatile double protectOutDelay @unit(s) = default(0s);
        volatile double protectInDelay @unit(s) = default(0s);
        volatile double handshakeOutDelay @unit(s) = default(0s);
        volatile double handshakeInDelay @unit(s) = default(0s);
        @lifecycleSupport();
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetSentToUpper](type=inet::Packet);
        // Structural record framing succeeded; the value is the plaintext
        // epoch or the two transmitted epoch bits for a protected record.
        // It does not imply semantic processing or deprotection.
        @signal[recordAccepted](type=long);
        @signal[recordDropped](type=long);
        @signal[associationEstablished](type=long);
        @signal[associationFailed](type=long);
        @signal[establishmentTime](type=double);
        @signal[recordOverhead](type=long);
        // Sum of retained incomplete handshake body bytes across this engine.
        @signal[reassemblyBytes](type=long);
        @signal[handshakeRetransmission](type=long);
        @signal[processingDelay](type=double);
        @statistic[packetSent](source=packetSent; record=count,"sum(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](source=packetReceived; record=count,"sum(packetBytes)"; interpolationmode=none);
        @statistic[recordDropped](source=recordDropped; record=count; interpolationmode=none);
        @statistic[associationEstablished](source=associationEstablished; record=count; interpolationmode=none);
        @statistic[associationFailed](source=associationFailed; record=count; interpolationmode=none);
        @statistic[handshakeRetransmission](source=handshakeRetransmission; record=count; interpolationmode=none);
        @statistic[establishmentTime](source=establishmentTime; record=vector,mean; unit=s; interpolationmode=none);
        @statistic[recordOverhead](source=recordOverhead; record=vector,mean; unit=B; interpolationmode=none);
        @statistic[reassemblyBytes](source=reassemblyBytes; record=vector,max; unit=B; interpolationmode=sample-hold);
        @statistic[processingDelay](source=processingDelay; record=vector,mean; unit=s; interpolationmode=none);
    gates:
        input socketIn;
        output socketOut;
}
