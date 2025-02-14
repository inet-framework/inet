/*
 * NoResponseException.h
 *
 *  Created on: 27 Apr 2021
 *      Author: msvoelker
 */

#ifndef INET_TRANSPORTLAYER_QUIC_NORESPONSEEXCEPTION_H_
#define INET_TRANSPORTLAYER_QUIC_NORESPONSEEXCEPTION_H_

#include <exception>

namespace inet {
namespace quic {

class NoResponseException: public std::runtime_error {
public:
    NoResponseException(const std::string& msg) : std::runtime_error(msg) { }
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_NORESPONSEEXCEPTION_H_ */
