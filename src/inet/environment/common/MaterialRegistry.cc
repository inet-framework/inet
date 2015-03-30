//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/environment/common/MaterialRegistry.h"
#include "inet/common/INETMath.h"

namespace inet {

namespace physicalenvironment {

MaterialRegistry MaterialRegistry::singleton;

MaterialRegistry::MaterialRegistry()
{
}

MaterialRegistry::~MaterialRegistry()
{
    for (auto & entry : materials)
        delete entry.second;
}

void MaterialRegistry::addMaterial(const Material *material) const
{
    materials.insert(std::pair<const std::string, const Material *>(material->getName(), material));
}

const Material *MaterialRegistry::getMaterial(const char *name) const
{
    if (materials.size() == 0)
    {
        // TODO: verify values
        addMaterial(new Material("vacuum", Ohmm(NaN), 1, 1));
        addMaterial(new Material("air", Ohmm(NaN), 1.00058986, 1.00000037));
        addMaterial(new Material("copper", Ohmm(1.68), NaN, NaN));
        addMaterial(new Material("aluminium", Ohmm(2.65), NaN, NaN));
        addMaterial(new Material("wood", Ohmm(1E+15), 5, 1.00000043));
        addMaterial(new Material("brick", Ohmm(3E+3), 4.5, 1));
        addMaterial(new Material("concrete", Ohmm(1E+2), 4.5, 1));
        addMaterial(new Material("glass", Ohmm(1E+12), 7, 1));
    }
    auto it = materials.find(name);
    return it != materials.end() ? it->second : nullptr;
}

} // namespace physicalenvironment

} // namespace inet

