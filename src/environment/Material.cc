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

#include "inet/environment/Material.h"
#include "inet/common/INETMath.h"

namespace inet {

std::map<const std::string, const Material *> Material::materials;

Material::Material(const char *name, Ohmm resistivity, double relativePermittivity, double relativePermeability) :
    cNamedObject(name),
    resistivity(resistivity),
    relativePermittivity(relativePermittivity),
    relativePermeability(relativePermeability)
{
}

double Material::getDielectricLossTangent(Hz frequency) const
{
    return unit(1.0 / (2 * M_PI * frequency * resistivity * relativePermittivity * e0)).get();
}

double Material::getRefractiveIndex() const
{
    return std::sqrt(relativePermittivity * relativePermeability);
}

mps Material::getPropagationSpeed() const
{
    return mps(SPEED_OF_LIGHT) / getRefractiveIndex();
}

void Material::addMaterial(const Material *material)
{
    materials.insert(std::pair<const std::string, const Material *>(material->getName(), material));
}

const Material *Material::getMaterial(const char *name)
{
    if (materials.size() == 0)
    {
        // TODO: check values?
        addMaterial(new Material("vacuum", Ohmm(sNaN), 1, 1));
        addMaterial(new Material("air", Ohmm(sNaN), 1.00058986, 1.00000037));
        addMaterial(new Material("copper", Ohmm(1.68), sNaN, sNaN));
        addMaterial(new Material("aluminium", Ohmm(2.65), sNaN, sNaN));
        addMaterial(new Material("wood", Ohmm(1E+15), 5, 1.00000043));
        addMaterial(new Material("brick", Ohmm(3E+3), 4.5, 1));
        addMaterial(new Material("concrete", Ohmm(1E+2), 4.5, 1));
        addMaterial(new Material("glass", Ohmm(1E+12), 7, 1));
    }
    std::map<const std::string, const Material *>::iterator it = materials.find(name);
    return it != materials.end() ? it->second : NULL;
}

} // namespace inet

