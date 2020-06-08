//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_EXTAPP_H
#define __INET_EXTAPP_H

#include "inet/common/INETDefs.h"
#include "inet/common/LinuxUtils.h"
#include "inet/common/NetworkNamespaceContext.h"

namespace inet {

class INET_API ExtApp : public cSimpleModule
{

  public:

    int numInitStages() const override {
        return NUM_INIT_STAGES;
    }

    void initialize(int stage) override {
        if (stage == INITSTAGE_LOCAL) {
            std::vector<std::string> prefixStrings = check_and_cast<cValueArray *>(par("commandPrefix").objectValue())->asStringVector();
            std::vector<std::string> commandStrings = check_and_cast<cValueArray *>(par("command").objectValue())->asStringVector();

            std::string netns = par("namespace").stringValue();

            std::vector<const char *> args;

            /*
            args.push_back("ip");
            args.push_back("netns");
            args.push_back("exec");
            args.push_back(netns.c_str());
            */
            for (const auto& c : prefixStrings)
                args.push_back(c.c_str());

            for (const auto& c : commandStrings) {
                args.push_back(c.c_str());
            }
            auto c = NetworkNamespaceContext(netns.c_str());
            run_command(args, false, false);
        }
    }
    void handleMessage(cMessage *msg) override {}

};

} // namespace inet

#endif // ifndef __INET_EXTAPP_H

