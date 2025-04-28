//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef INET_APPLICATIONS_QUIC_TRANSPORTPARAMETER_H_
#define INET_APPLICATIONS_QUIC_TRANSPORTPARAMETER_H_

#include "Quic.h"

namespace inet {
namespace quic {

class TransportParameter {
public:
    TransportParameter(Quic *quicSimpleMod);
    virtual ~TransportParameter();

    omnetpp::simtime_t maxAckDelay = omnetpp::SimTime(25, omnetpp::SimTimeUnit::SIMTIME_MS); //TODO make it as parameter in ini
    int ackDelayExponent = 3;

    uint64_t initial_max_data = 0;
    uint64_t initial_max_stream_data = 0;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_TRANSPORTPARAMETER_H_ */
