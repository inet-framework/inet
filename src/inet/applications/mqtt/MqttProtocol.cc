// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttProtocol.h"
#include "inet/common/ProtocolGroup.h"
namespace inet::mqtt {
const Protocol mqttProtocol("mqtt", "MQTT 3.1.1");
EXECUTE_PRE_NETWORK_SETUP(ProtocolGroup::getTcpProtocolGroup()->addProtocol(1883, &mqttProtocol));
}
