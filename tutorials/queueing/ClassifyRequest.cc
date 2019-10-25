#include "inet/queueing/function/PacketClassifierFunction.h"
#include "inet/common/packet/Packet.h"

using namespace inet; 

static int classifyRequest(Packet *packet)
{
    return packet->getId() % 2;
}

Register_Packet_Classifier_Function(RequestClassifier, classifyRequest);
