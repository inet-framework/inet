//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/storage/IdealEpEnergyStorage.h"

namespace inet {

namespace power {

Define_Module(IdealEpEnergyStorage);

void IdealEpEnergyStorage::initialize(int stage)
{
    EpEnergyStorageBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        energyBalance = J(0);
        lastEnergyBalanceUpdate = 0;
        WATCH(energyBalance);
    }
}

void IdealEpEnergyStorage::updateTotalPowerConsumption()
{
    updateEnergyBalance();
    EpEnergyStorageBase::updateTotalPowerConsumption();
}

void IdealEpEnergyStorage::updateTotalPowerGeneration()
{
    updateEnergyBalance();
    EpEnergyStorageBase::updateTotalPowerGeneration();
}

void IdealEpEnergyStorage::updateEnergyBalance()
{
    simtime_t currentSimulationTime = simTime();
    if (currentSimulationTime != lastEnergyBalanceUpdate) {
        energyBalance += s((currentSimulationTime - lastEnergyBalanceUpdate).dbl()) * (totalPowerGeneration - totalPowerConsumption);
        lastEnergyBalanceUpdate = currentSimulationTime;
        emit(residualEnergyCapacityChangedSignal, energyBalance.get());
    }
}

} // namespace power

} // namespace inet

