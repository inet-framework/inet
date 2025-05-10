//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONCLOSEDEXCEPTION_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONCLOSEDEXCEPTION_H_

#include "ConnectionDiedException.h"

namespace inet {
namespace quic {

class ConnectionClosedException: public ConnectionDiedException {
public:
    ConnectionClosedException(const std::string& msg) : ConnectionDiedException(msg) { }
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONCLOSEDEXCEPTION_H_ */
