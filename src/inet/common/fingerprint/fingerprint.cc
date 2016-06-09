//==========================================================================
//   FINGERPRINT.CC
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2016 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "inet/common/fingerprint/fingerprint.h"

#include "inet/applications/pingapp/PingApp.h"
#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/networklayer/common/EchoProtocol.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"
#include "inet/networklayer/icmpv6/ICMPv6.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/ipv4/ICMP.h"
#include "inet/networklayer/ipv4/ICMPMessage_m.h"

namespace inet {

Register_Class(CustomFingerprintCalculator);

void CustomFingerprintCalculator::addEvent(cEvent *event)
{
    if (addEvents) {
        const MatchableObject matchableEvent(event);
        if (eventMatcher == nullptr || eventMatcher->matches(&matchableEvent)) {
            cMessage *message = nullptr;
            cPacket *packet = nullptr;
            cObject *controlInfo = nullptr;
            cModule *module = nullptr;
            if (event->isMessage()) {
                message = static_cast<cMessage *>(event);
                if (message->isPacket())
                    packet = static_cast<cPacket *>(message);
                controlInfo = message->getControlInfo();
                module = message->getArrivalModule();
            }

            // ICMP*: direct send from PingApp
            if (module && packet && !packet->isSelfMessage() && (packet->getSenderGateId() == -1)
                    && typeid(*packet) == typeid(PingPayload)&& (
                    (typeid(*module) == typeid(ICMP)) || (typeid(*module) == typeid(ICMPv6)) || (typeid(*module) == typeid(EchoProtocol))
                    )) {
                std::cout << "ICMP module arrived PingPayload with directSend() from PingApp, skipped" << std::endl;
                return;
            }
            // ICMP*: ICMP_ECHO_REPLY from network protocol module
            if (module && packet && !packet->isSelfMessage() && (
                    (typeid(*module) == typeid(ICMP) && packet->getKind() == ICMP_ECHO_REPLY)
                    || (typeid(*module) == typeid(ICMPv6) && packet->getKind() == ICMPv6_ECHO_REPLY)
                    || (typeid(*module) == typeid(EchoProtocol) && packet->getKind() == ICMP_ECHO_REPLY)
                    || (typeid(*module) == typeid(EchoProtocol) && packet->getKind() == ICMPv6_ECHO_REPLY)
                    )) {
                std::cout << "ICMP module arrived echo reply, skipped" << std::endl;
                return;
            }
            // PingApp arrived packet:
            //     before: receiving only icmp_echo_reply in PingPayload,
            //     after: receiving all ICMPMessage
            if (module && packet && typeid(*module) == typeid(PingApp)) {
                if (ICMPMessage *icmp = dynamic_cast<ICMPMessage*>(packet)) {
                    if (icmp->getType() != ICMP_ECHO_REPLY)
                        return;
                    packet = packet->getEncapsulatedPacket();
                    message = packet;
                }
                else if (dynamic_cast<ICMPv6Message*>(packet)) {
                    if (!dynamic_cast<ICMPv6EchoReplyMsg*>(packet))
                        return;
                    packet = packet->getEncapsulatedPacket();
                    message = packet;
                }
                else if (EchoPacket *icmp = dynamic_cast<EchoPacket*>(packet)) {
                    if (icmp->getType() != ECHO_PROTOCOL_REPLY)
                        return;
                    packet = packet->getEncapsulatedPacket();
                    message = packet;
                }
                else if (dynamic_cast<PingPayload*>(packet)) {
                    // update fingerprint
                }
                else
                    throw cRuntimeError("Unknown arrived message at PingApp: type:%s", packet->getClassName());

                if (PingPayload *pingPayload = dynamic_cast<PingPayload*>(packet)) {
                    if (pingPayload->getOriginatorId() != static_cast<PingApp *>(module)->getPid()) {
                        std::cout << "PingApp module arrived foreign echo reply from network layer, skipped" << std::endl;
                        return;
                    }
                }
            }
            if (module && message && !packet && typeid(*module) == typeid(PingApp)) {
                if (message->isSelfMessage() && message->getKind() == 8888 /* PING_DO_SEND */)
                    return;
            }

            // skip L3SocketCommands
            if (module && message && packet==nullptr && controlInfo && dynamic_cast<L3SocketCommand*>(controlInfo)) {
                std::cout << "L3SocketCommand arrived, skipped" << std::endl;
                return;
            }

            MatchableObject matchableModule(module);
            if (module == nullptr || moduleMatcher == nullptr || moduleMatcher->matches(&matchableModule)) {
                for (std::string::iterator it = ingredients.begin(); it != ingredients.end(); ++it) {
                    FingerprintIngredient ingredient = (FingerprintIngredient) *it;
                    if (!addEventIngredient(event, ingredient)) {
                        switch (ingredient) {
                            case EVENT_NUMBER:
                                hasher->add(getSimulation()->getEventNumber()); break;
                            case SIMULATION_TIME:
                                hasher->add(simTime().raw()); break;
                            case MESSAGE_FULL_NAME:
                                hasher->add(event->getFullName()); break;
                            case MESSAGE_CLASS_NAME:
                                hasher->add(event->getClassName()); break;
                            case MESSAGE_KIND:
                                if (message != nullptr)
                                    hasher->add(message->getKind());
                                break;
                            case MESSAGE_BIT_LENGTH:
                                if (packet != nullptr)
                                    hasher->add(packet->getBitLength());
                                break;
                            case MESSAGE_CONTROL_INFO_CLASS_NAME:
                                if (controlInfo != nullptr)
                                    hasher->add(controlInfo->getClassName());
                                break;
                            case MESSAGE_DATA:
                                // skip message data
                                break;
                            case MODULE_ID:
                                if (module != nullptr)
                                    hasher->add(module->getId());
                                break;
                            case MODULE_FULL_NAME:
                                if (module != nullptr)
                                    hasher->add(module->getFullName());
                                break;
                            case MODULE_FULL_PATH:
                                if (module != nullptr) {
                                    std::string fullPath = module->getFullPath();
                                    hasher->add(fullPath.c_str());
                                }
                                break;
                            case MODULE_CLASS_NAME:
                                if (module != nullptr)
                                    hasher->add(module->getComponentType()->getClassName());
                                break;
                            case RANDOM_NUMBERS_DRAWN:
                                for (int i = 0; i < getEnvir()->getNumRNGs(); i++)
                                    hasher->add(getEnvir()->getRNG(i)->getNumbersDrawn());
                                break;
                            case CLEAN_HASHER:
                                hasher->reset();
                                break;
                            case RESULT_SCALAR:
                            case RESULT_STATISTIC:
                            case RESULT_VECTOR:
                            case EXTRA_DATA:
                                // not processed here
                                break;
                            default:
                                throw cRuntimeError("Unknown fingerprint ingredient '%c' (%d)", ingredient, ingredient);
                        }
                    }
                }
            }
        }
    }
}

} // namespace omnetpp


