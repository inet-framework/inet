//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "TransportParameters.h"

namespace inet {
namespace quic {

void TransportParameters::readParameters(Quic *quicSimpleMod)
{
    initialMaxData = quicSimpleMod->par("initialMaxData");
    initialMaxStreamData = quicSimpleMod->par("initialMaxStreamData");
}

void TransportParameters::readExtension(Ptr<const TransportParametersExtension> transportParametersExt)
{
    initialMaxData = transportParametersExt->getInitialMaxData();
    initialMaxStreamData = transportParametersExt->getInitialMaxStreamDataUni();
    if (initialMaxStreamData != transportParametersExt->getInitialMaxStreamDataBidiLocal()
     || initialMaxStreamData != transportParametersExt->getInitialMaxStreamDataBidiRemote()) {
        EV_WARN << "inial_max_stream_data parameters not equal in the received transport parameter. Use only initial_max_stream_data_uni for all streams." << endl;
    }
}

} /* namespace quic */
} /* namespace inet */
