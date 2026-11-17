// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqttsn/MqttSnProtocol.h"
#include "inet/common/ProtocolGroup.h"

namespace inet::mqttsn {
const Protocol mqttSnProtocol("mqttsn", "MQTT-SN 1.2");
EXECUTE_PRE_NETWORK_SETUP(ProtocolGroup::getUdpProtocolGroup()->addProtocol(1884, &mqttSnProtocol));
}
