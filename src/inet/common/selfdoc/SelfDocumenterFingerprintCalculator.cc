//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/selfdoc/SelfDocumenterFingerprintCalculator.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Register_Class(SelfDocumenterFingerprintCalculator);

static bool isNum (char c) { return (c >= '0' && c <= '9'); }

void SelfDocumenterFingerprintCalculator::addEvent(cEvent *event)
{
    if (SelfDoc::generateSelfdoc && event->isMessage()) {
        cMessage *msg = static_cast<cMessage *>(event);
        auto ctrl = msg->getControlInfo();
        auto context = msg->getContextPointer();

        auto fromModule = msg->getSenderModule();
        auto fromModuleName = fromModule->getComponentType()->getFullName();
        auto toModule = msg->getArrivalModule();
        auto toModuleName = toModule->getComponentType()->getFullName();
        if (msg->isSelfMessage()) {
            std::ostringstream os;
            std::string msgName = msg->getName();
            std::replace_if(msgName.begin(), msgName.end(), isNum, 'N');
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", fromModuleName)
                    << ", " << SelfDoc::keyVal("action", "SCHEDULE")
                    << ", " << SelfDoc::val("details") << " : {"
                    << SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                    << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), fromModule->getProperties(), "selfMessageKinds", nullptr, ""))
                    << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                    << ", " << SelfDoc::tagsToJson("tags", msg)
                    << ", " << SelfDoc::keyVal("msgname", msgName)
                    << ", " << SelfDoc::keyVal("context", context ? "filled" : "")
                    << " } }"
                   ;
            globalSelfDoc.insert(os.str());
        }
        else {
            auto senderGate = msg->getSenderGate();
            auto arrivalGate = msg->getArrivalGate();
            if (senderGate == nullptr) {
                std::ostringstream os;
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", fromModuleName)
                        << ", " << SelfDoc::keyVal("action", "OUTPUTDIRECT")
                        << ", " << SelfDoc::val("details") << " : {"
                        << SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                        << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), fromModule->getProperties(), "directSendKinds", arrivalGate->getProperties(), "messageKinds"))
                        << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                        << ", " << SelfDoc::tagsToJson("tags", msg)
                        << ", " << SelfDoc::keyVal("destModule", toModuleName)
                        << " } }"
                       ;
                globalSelfDoc.insert(os.str());
            }
            else {
                std::ostringstream os;
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", fromModuleName)
                        << ", " << SelfDoc::keyVal("action", "OUTPUT")
                        << ", " << SelfDoc::val("details") << " : {"
                        << SelfDoc::keyVal("gate", SelfDoc::gateInfo(senderGate))
                        << ", "<< SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                        << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), senderGate->getProperties(), "messageKinds", arrivalGate->getProperties(), "messageKinds"))
                        << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                        << ", " << SelfDoc::tagsToJson("tags", msg)
                        << ", " << SelfDoc::keyVal("destModule", toModuleName)
                        << " } }"
                       ;
                globalSelfDoc.insert(os.str());
            }

            {
                std::ostringstream os;
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", toModuleName)
                        << ", " << SelfDoc::keyVal("action", "INPUT")
                        << ", " << SelfDoc::val("details") << " : {"
                        << SelfDoc::keyVal("gate", SelfDoc::gateInfo(arrivalGate))
                        << ", " << SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                        << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), arrivalGate->getProperties(), "messageKinds", senderGate ? senderGate->getProperties() : nullptr, "messageKinds"))
                        << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                        << ", " << SelfDoc::tagsToJson("tags", msg)
                        << ", " << SelfDoc::keyVal("srcModule", fromModuleName)
                        << " } }"
                       ;
                globalSelfDoc.insert(os.str());
            }
        }
    }
    cSingleFingerprintCalculator::addEvent(event);
}

} // namespace

