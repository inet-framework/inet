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

#include "TestCM.h"
#include <asserts.h>

Define_Module(TestCM);


double TestCM::calcInterfDist() {
	return par("maxInterferenceDistance").doubleValue();
}

void TestCM::updateConnections(int nicID, const Coord* oldPos, const Coord* newPos) {
	BaseConnectionManager::updateConnections(nicID, oldPos, newPos);

	NicEntry* nic = nics[nicID];
	displayPassed = false;
	assertTrue("NicID should exists.", nic != 0);

	for(NicEntries::iterator i = nics.begin(); i != nics.end(); ++i)
	{
		NicEntry* nic_i = i->second;

		// no recursive connections
		if ( nic_i->nicId == nicID ) continue;

		double distance;

		if(useTorus)
		{
			distance = nic->pos.sqrTorusDist(nic_i->pos, playgroundSize);
		} else {
			distance = nic->pos.sqrdist(nic_i->pos);
		}

		bool inRange = (distance <= maxDistSquared);
		bool connected = nic->isConnected(nic_i);

		assertEqual("Nics in range should be connected.", inRange, connected);
	}
	displayPassed = true;
}
