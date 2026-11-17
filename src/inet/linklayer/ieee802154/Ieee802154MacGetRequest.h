// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154MACGETREQUEST_H
#define __INET_IEEE802154MACGETREQUEST_H

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "inet/linklayer/ieee802154/Ieee802154Address.h"
#include "inet/linklayer/ieee802154/Ieee802154MacServiceResult.h"

namespace inet {

/** Bounded MAC-only PIB value representation; no generic string value is hidden here. */
using Ieee802154MacPibValue = std::variant<bool, int64_t, uint64_t, Ieee802154Address>;

/**
 * String identifiers are preserved verbatim, including unknown names. The
 * bounded API intentionally has no implicit hierarchical/index syntax.
 */
struct Ieee802154MacGetRequest
{
    std::string attribute;
};

struct Ieee802154MacGetConfirm
{
    std::string attribute;
    Ieee802154MacServiceResult result;
    std::optional<Ieee802154MacPibValue> value; ///< Absent with UNSUPPORTED_ATTRIBUTE.

    Ieee802154MacGetConfirm(std::string attribute, Ieee802154MacServiceResult result) : attribute(std::move(attribute)), result(std::move(result)) {}
    Ieee802154MacGetConfirm() = delete;
};

struct Ieee802154MacSetRequest
{
    std::string attribute;
    Ieee802154MacPibValue value;
};

struct Ieee802154MacSetConfirm
{
    std::string attribute;
    Ieee802154MacServiceResult result;

    Ieee802154MacSetConfirm(std::string attribute, Ieee802154MacServiceResult result) : attribute(std::move(attribute)), result(std::move(result)) {}
    Ieee802154MacSetConfirm() = delete;
};

struct Ieee802154MacResetRequest
{
    bool setDefaultPib = false; ///< IEEE Std 802.15.4-2024, Table 8-12.
};

struct Ieee802154MacResetConfirm
{
    bool setDefaultPib;
    Ieee802154MacServiceResult result;

    Ieee802154MacResetConfirm(bool setDefaultPib, Ieee802154MacServiceResult result) : setDefaultPib(setDefaultPib), result(std::move(result)) {}
    Ieee802154MacResetConfirm() = delete;
};

} // namespace inet

#endif
