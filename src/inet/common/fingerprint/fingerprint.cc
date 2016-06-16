//==========================================================================
//   FINGERPRINT.CC
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2016 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "inet/common/fingerprint/fingerprint.h"

#include "inet/networklayer/arp/ipv4/ARP.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/networklayer/ipv4/IPv4.h"

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

            // ARPPacket in ARP or IPv4 module:
            if (module && packet && typeid(*packet) == typeid(ARPPacket)
                    && (typeid(*module) == typeid(IPv4) || typeid(*module) == typeid(ARP))) {
                return;
            }
            // IPv4 self messages:
            if (module && message && message->isSelfMessage() && typeid(*module) == typeid(IPv4)) {
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


