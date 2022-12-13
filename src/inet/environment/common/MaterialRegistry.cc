//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/common/MaterialRegistry.h"

#include "inet/common/INETMath.h"

namespace inet {

namespace physicalenvironment {

MaterialRegistry::MaterialRegistry()
{
}

MaterialRegistry::~MaterialRegistry()
{
    for (auto& entry : materials)
        delete entry.second;
}

void MaterialRegistry::addMaterial(const Material *material) const
{
    materials.insert(std::pair<const std::string, const Material *>(material->getName(), material));
}

const Material *MaterialRegistry::getMaterial(const char *name) const
{
    if (materials.size() == 0) {
        // TODO verify values
        addMaterial(new Material("vacuum", Ohmm(NaN), 1, 1));
        addMaterial(new Material("air", Ohmm(NaN), 1.00058986, 1.00000037));
        addMaterial(new Material("copper", Ohmm(1.68), NaN, NaN));
        addMaterial(new Material("aluminium", Ohmm(2.65), NaN, NaN));
        addMaterial(new Material("wood", Ohmm(1E+15), 5, 1.00000043));
        addMaterial(new Material("forest", Ohmm(37E+3), 1.6, 1));
        addMaterial(new Material("brick", Ohmm(3E+3), 4.5, 1));
        addMaterial(new Material("concrete", Ohmm(1E+2), 4.5, 1));
        addMaterial(new Material("glass", Ohmm(1E+12), 7, 1));
    }
    auto it = materials.find(name);
    return it != materials.end() ? it->second : nullptr;
}

MaterialRegistry& MaterialRegistry::getInstance()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::MaterialRegistry::instance");
    return getSimulationOrSharedDataManager()->getSharedVariable<MaterialRegistry>(handle);
}

} // namespace physicalenvironment

} // namespace inet

