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

        auto from = msg->getSenderModule();
        if (msg->isSelfMessage()) {
            std::ostringstream os;
            std::string msgName = msg->getName();
            std::replace_if(msgName.begin(), msgName.end(), isNum, 'N');
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", from->getComponentType()->getFullName())
                    << ", " << SelfDoc::keyVal("action", "SCHEDULE")
                    << ", " << SelfDoc::val("details") << " : {"
                    << SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                    << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), from->getProperties(), "selfMessageKinds", nullptr, ""))
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
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", from->getComponentType()->getFullName())
                        << ", " << SelfDoc::keyVal("action", "OUTPUTDIRECT")
                        << ", " << SelfDoc::val("details") << " : {"
                        << SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                        << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), from->getProperties(), "directSendKinds", arrivalGate->getProperties(), "messageKinds"))
                        << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                        << ", " << SelfDoc::tagsToJson("tags", msg)
                        << " } }"
                       ;
                globalSelfDoc.insert(os.str());
            }
            else {
                std::ostringstream os;
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", from->getComponentType()->getFullName())
                        << ", " << SelfDoc::keyVal("action", "OUTPUT")
                        << ", " << SelfDoc::val("details") << " : {"
                        << SelfDoc::keyVal("gate", SelfDoc::gateInfo(senderGate))
                        << ", "<< SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                        << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), senderGate->getProperties(), "messageKinds", arrivalGate->getProperties(), "messageKinds"))
                        << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                        << ", " << SelfDoc::tagsToJson("tags", msg)
                        << " } }"
                       ;
                globalSelfDoc.insert(os.str());
            }

            {
                std::ostringstream os;
                auto to = msg->getArrivalModule();
                os << "=SelfDoc={ " << SelfDoc::keyVal("module", to->getComponentType()->getFullName())
                        << ", " << SelfDoc::keyVal("action", "INPUT")
                        << ", " << SelfDoc::val("details") << " : {"
                        << SelfDoc::keyVal("gate", SelfDoc::gateInfo(arrivalGate))
                        << ", " << SelfDoc::keyVal("msg", opp_typename(typeid(*msg)))
                        << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(msg->getKind(), arrivalGate->getProperties(), "messageKinds", senderGate ? senderGate->getProperties() : nullptr, "messageKinds"))
                        << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                        << ", " << SelfDoc::tagsToJson("tags", msg)
                        << " } }"
                       ;
                globalSelfDoc.insert(os.str());
            }
        }
    }
    cSingleFingerprintCalculator::addEvent(event);
}

} // namespace

