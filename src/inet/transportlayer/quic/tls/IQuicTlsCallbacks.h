//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IQUICTLSCALLBACKS_H
#define __INET_IQUICTLSCALLBACKS_H

#include <vector>
#include <cstdint>

namespace inet {
namespace quic {

// Forward declaration - EncryptionLevel is defined in Connection.h
enum class EncryptionLevel;  // Initial=0, ZeroRtt=1, Handshake=2, OneRtt=3

/**
 * Interface for callbacks from the TLS implementation to the QUIC connection.
 *
 * The TLS implementation uses this interface to notify the QUIC connection
 * about key updates, handshake data to send, and TLS alerts. This allows
 * the QUIC implementation to be decoupled from the specific TLS library used.
 */
class IQuicTlsCallbacks
{
  public:
    virtual ~IQuicTlsCallbacks() = default;

    /**
     * Provides the QUIC connection with read and write secrets negotiated
     * for the specified encryption level.
     *
     * The encryption level refers to one of the four QUIC packet encryption
     * levels: Initial, Handshake, Application (1-RTT) and EarlyData (0-RTT).
     *
     * @param level The encryption level these secrets apply to.
     * @param readSecret The secret for decrypting incoming packets at this level.
     *                   May be empty if not available (e.g., client 0-RTT send-only).
     * @param writeSecret The secret for encrypting outgoing packets at this level.
     *                    May be empty if not available (e.g., server 0-RTT receive-only).
     */
    virtual void setEncryptionSecrets(EncryptionLevel level,
                                      const std::vector<uint8_t>& readSecret,
                                      const std::vector<uint8_t>& writeSecret) = 0;

    /**
     * Adds TLS handshake data to be sent to the peer via CRYPTO frames.
     *
     * The encryption level specifies which QUIC packet type should be used
     * to send the CRYPTO frame. This method can be called multiple times
     * to accumulate handshake data before flushing.
     *
     * @param level The encryption level for sending this data.
     * @param data The TLS handshake data to send.
     */
    virtual void addHandshakeData(EncryptionLevel level, const std::vector<uint8_t>& data) = 0;

    /**
     * Called after addHandshakeData() to inform the QUIC connection that
     * the accumulated handshake data should be sent.
     *
     * This allows the TLS implementation to batch multiple handshake messages
     * before triggering transmission.
     */
    virtual void flushHandshakeData() = 0;

    /**
     * When the TLS implementation encounters an error, this function is used
     * to provide the TLS alert code which is then used to construct a
     * CONNECTION_CLOSE frame for terminating the connection.
     *
     * @param level The encryption level at which the error occurred.
     * @param alertCode The TLS alert code (e.g., 10 for unexpected_message,
     *                  20 for bad_record_mac, 40 for handshake_failure, etc.).
     */
    virtual void sendTlsAlert(EncryptionLevel level, int alertCode) = 0;
};

} // namespace quic
} // namespace inet

#endif // __INET_IQUICTLSCALLBACKS_H
