// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.dtls;

import inet.common.scenario.ScenarioManager;

// Application actions are opt-in; the original demonstration topology is unchanged.
network DtlsScenarioNetwork extends DtlsNetwork
{
    submodules:
        scenarioManager: ScenarioManager;
}
