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

#include "TransportParameter.h"

namespace inet {
namespace quic {

TransportParameter::TransportParameter(Quic *quicSimpleMod) {

    initial_max_data = quicSimpleMod->par("initialMaxData");
    initial_max_stream_data_bidi_local = quicSimpleMod->par("initialMaxStreamDataBidiLocal");
    initial_max_stream_data_bidi_remote = quicSimpleMod->par("initialMaxStreamDataBidiRemote");
    initial_max_stream_data_uni = quicSimpleMod->par("initialMaxStreamDataUni");
}

TransportParameter::~TransportParameter() {
    // TODO Auto-generated destructor stub
}

} /* namespace quic */
} /* namespace inet */
