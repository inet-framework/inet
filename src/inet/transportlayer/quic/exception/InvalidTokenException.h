//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_INVALIDTOKENEXCEPTION_H_
#define INET_TRANSPORTLAYER_QUIC_INVALIDTOKENEXCEPTION_H_

#include <exception>

namespace inet {
namespace quic {

class InvalidTokenException: public std::runtime_error {
public:
    InvalidTokenException(const std::string& msg) : std::runtime_error(msg) { }
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_INVALIDTOKENEXCEPTION_H_ */
