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

#include "inet/applications/generic/ExtApp.h"

namespace inet {

Define_Module(ExtApp);

void ExtApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        std::vector<std::string> prefixStrings = check_and_cast<cValueArray *>(par("commandPrefix").objectValue())->asStringVector();
        std::vector<std::string> commandStrings = check_and_cast<cValueArray *>(par("command").objectValue())->asStringVector();

        std::string netns = par("namespace").stringValue();

        std::vector<const char *> args;

        for (const auto& c : prefixStrings)
            args.push_back(c.c_str());

        for (const auto& c : commandStrings)
            args.push_back(c.c_str());

        NetworkNamespaceContext ctxt(netns.c_str());
        execCommand(args, false, false);
    }
}


// EXECUTE_ON_SHUTDOWN(killProcess());

} // namespace inet

