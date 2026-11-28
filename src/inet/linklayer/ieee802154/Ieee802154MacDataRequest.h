// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154MACDATAREQUEST_H
#define __INET_IEEE802154MACDATAREQUEST_H

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "inet/linklayer/ieee802154/Ieee802154Address.h"
#include "inet/linklayer/ieee802154/Ieee802154MacServiceResult.h"

namespace inet {

/**
 * IEEE Std 802.15.4-2024, Table 8-2 security parameter descriptor.
 * Level zero is the unsecured M1 representation: key mode, source, and index
 * are retained as raw values but have no security meaning. Key source lengths
 * are four or eight octets for key modes 2 and 3, respectively.
 */
struct Ieee802154MacSecurityParameters
{
    uint8_t securityLevel = 0; ///< 0x00..0x07; M1 uses zero.
    uint8_t keyIdMode = 0; ///< 0x00..0x03; ignored at security level zero.
    std::vector<uint8_t> keySource; ///< Four or eight octets for key modes 2 or 3.
    uint8_t keyIndex = 0; ///< 0x01..0xff when applicable; zero means ignored.
};

/**
 * Bounded M1 MCPS-DATA.request metadata. Source identity comes from the
 * authoritative MAC PIB. The service fixes LegacyTx=true and DataRate=0;
 * GTS, indirect, ranging, IE, and multipurpose options are outside this API.
 */
struct Ieee802154MacDataRequest
{
    Ieee802154Address::AddressType sourceAddressMode = Ieee802154Address::AddressType::NONE; ///< Source identity is read from the MAC PIB.
    Ieee802154Address destinationAddress; ///< NONE, SHORT, or EXTENDED; native value is never narrowed.
    uint16_t destinationPanId = 0; ///< IEEE Std 802.15.4-2024, Table 8-30, 0x0000..0xffff.
    uint8_t msduHandle = 0; ///< IEEE Std 802.15.4-2024, Table 8-30, 0x00..0xff.
    bool ackTx = false; ///< Direct acknowledged transmission selection.
    Ieee802154MacSecurityParameters security;
};

struct Ieee802154MacDataConfirm
{
    uint8_t msduHandle;
    Ieee802154MacServiceResult result;
    std::optional<uint32_t> timestamp;
    std::optional<uint8_t> numBackoffs;
    std::optional<bool> ackFramePending;
    std::optional<uint8_t> ackRssi; ///< Numeric uint8 representation is a local choice; Table 8-31 calls this Boolean and gives no range.

    /**
     * Timestamp is an optional raw 24-bit value and is absent for M1.
     * numBackoffs is present exactly for an IEEE SUCCESS result and absent for
     * every failure or local abort. ACK metadata is present only for an actual
     * ACK; an unsupported or unmeasured zero RSSI is represented as absent.
     */
    Ieee802154MacDataConfirm(uint8_t msduHandle, Ieee802154MacServiceResult result) : msduHandle(msduHandle), result(std::move(result)) {}
    Ieee802154MacDataConfirm() = delete;
};

/**
 * Normal MCPS-DATA.indication metadata; the callback packet contains the MSDU
 * only. sourcePanId is wire-carried; destinationPanId is always effective,
 * using the receiver PAN when the frame omitted it. effectiveSourcePanId is a
 * separate local context value and is never a substitute for sourcePanId.
 */
struct Ieee802154MacDataIndication
{
    std::optional<uint16_t> sourcePanId;
    uint16_t destinationPanId = 0;
    std::optional<uint16_t> effectiveSourcePanId; ///< Local context; when both are present it agrees with sourcePanId.
    Ieee802154Address sourceAddress;
    Ieee802154Address destinationAddress;
    std::optional<uint8_t> dsn; ///< Required for selected legacy frame versions 0 and 1; optional only for future suppressed forms.
    bool framePending = false;
    bool ackSent = false;
    uint8_t frameVersion = 0; ///< M1 normal legacy reception is version 0 or 1.
    uint8_t dataRate = 0; ///< M1 O-QPSK representation is fixed to DataRate 0.
    Ieee802154MacSecurityParameters security; ///< M1 normal delivery is unsecured (level zero).
    uint8_t lqi = 0; ///< Table 8-32, 0x00..0xff.
    uint8_t rssi = 0; ///< Local numeric 8-bit representation of the Table 8-32 RSSI.
    std::optional<uint32_t> timestamp; ///< Optional future-profile raw 24-bit value; absent for M1.
};

/**
 * Promiscuous metadata for a raw MHR-plus-payload callback packet; decoded
 * addresses, DSN, and ACK-sent state are deliberately absent.
 */
struct Ieee802154MacPromiscuousIndication
{
    uint8_t lqi = 0; ///< The only decoded reception quality field retained here.
    uint8_t rssi = 0; ///< Local numeric 8-bit representation of reception RSSI.
    std::optional<uint32_t> timestamp; ///< Optional future-profile raw 24-bit value; absent for M1; raw frame excludes FCS.
};

/**
 * MLME-COMM-STATUS.indication metadata; plaintext is a nullable callback
 * packet and security metadata is absent when early processing failed.
 */
struct Ieee802154MacCommStatusIndication
{
    Ieee802154MacStatus status; ///< This indication is not a DATA.confirm.
    std::optional<uint16_t> panId;
    std::optional<Ieee802154Address> sourceAddress;
    std::optional<Ieee802154Address> destinationAddress;
    std::optional<Ieee802154MacSecurityParameters> security;

    explicit Ieee802154MacCommStatusIndication(Ieee802154MacStatus status) : status(status) {}
    Ieee802154MacCommStatusIndication() = delete;
};

} // namespace inet

#endif
