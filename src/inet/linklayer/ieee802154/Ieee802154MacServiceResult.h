// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154MACSERVICERESULT_H
#define __INET_IEEE802154MACSERVICERESULT_H

#include <cstdint>
#include <variant>

namespace inet {

using Ieee802154MacServiceRequestId = uint64_t;

/**
 * Zero is reserved and cannot identify a service request. IDs are global
 * across DATA, GET, SET, RESET, and every refused or accepted attempt; after
 * UINT64_MAX the client cannot issue another request and must never wrap.
 */
constexpr Ieee802154MacServiceRequestId IEEE802154_INVALID_REQUEST_ID = 0;

/**
 * Local admission of a service request, before an IEEE primitive completes.
 * Refusals retain caller-owned DATA packets and produce no confirmation.
 */
enum class Ieee802154MacServiceAdmission : uint8_t {
    ACCEPTED,
    BUSY,
    UNAVAILABLE,
    INVALID_REQUEST,
    UNSUPPORTED_OPERATION,
};

/**
 * Result of a local cancellation attempt. A successful cancellation detaches
 * the request and delivers CANCELLED exactly once before cancel() returns.
 */
enum class Ieee802154MacServiceCancelResult : uint8_t {
    CANCELLED,
    NOT_PENDING,
    NOT_CANCELLABLE,
};

/**
 * Local terminal outcomes kept separate from IEEE primitive statuses. RESET
 * and STOPPED are lifecycle barriers; a disappearing client receives no call.
 */
enum class Ieee802154MacServiceAbortReason : uint8_t {
    CANCELLED,
    RESET,
    STOPPED,
};

/**
 * Bounded IEEE status vocabulary used by the selected service surface. The
 * security entries are representable clause 8.2.2 outcomes, not a claim that
 * cryptographic processing is implemented. INVALID_INDEX is retained only as
 * standard status vocabulary; this bounded string-attribute API has no index
 * field. Values are local enum ordinals, never IEEE wire codes.
 */
enum class Ieee802154MacStatus : uint8_t {
    SUCCESS,
    INVALID_ADDRESS,
    INVALID_PARAMETER,
    FRAME_TOO_LONG,
    CHANNEL_ACCESS_FAILURE,
    NO_ACK,
    TRANSACTION_OVERFLOW,
    TRANSACTION_EXPIRED,
    UNSUPPORTED_DATARATE,
    UNSUPPORTED_SECURITY,
    ACK_RCVD_NODSN_NOSA,
    UNSUPPORTED_ATTRIBUTE,
    READ_ONLY,
    INVALID_INDEX,
    COUNTER_ERROR,
    IMPROPER_IE_SECURITY,
    IMPROPER_KEY_TYPE,
    IMPROPER_SECURITY_LEVEL,
    KEY_LENGTH_MISMATCH,
    SECURITY_ERROR,
    UNAVAILABLE_DEVICE,
    UNAVAILABLE_KEY,
    UNAVAILABLE_SECURITY_LEVEL,
    UNSUPPORTED_ALGORITHM,
    UNSUPPORTED_LEGACY,
};

/**
 * A terminal result keeps local cancellation/lifecycle outcomes separate from
 * IEEE primitive statuses. Enum values are local API ordinals, not wire codes.
 */
using Ieee802154MacServiceResult = std::variant<Ieee802154MacStatus, Ieee802154MacServiceAbortReason>;

} // namespace inet

#endif
