
#include <fstream>

#include "AnalysisEnergy.h"

#include "BasicBattery.h"   // provides: access to the node batteries

#define coreEV (ev.isDisabled()||!mCoreDebug) ? std::cout : ev << "AnalysisEnergy: "

Define_Module(AnalysisEnergy);

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ===================================

void AnalysisEnergy::initialize(int aStage)
{
    cSimpleModule::initialize(aStage);

    if (0 == aStage)
    {
        ev << "initializing AnalysisEnergy\n";

        mCreateSnapshot = new cMessage("mCreateSnapshot");
        scheduleAt(1000, mCreateSnapshot);

        // read the parameters from the .ini file
        mCoreDebug = (bool) par("coreDebug");
        mNumHosts = par("numHosts");
        mpHostModuleName.assign(par("hostModuleName").stdstringValue());

        mNumHostsDepleted = 0;
    }
}

void AnalysisEnergy::finish()
{
    delete mCreateSnapshot;
    SnapshotLifetimes();
}

//============================= OPERATIONS ===================================

/** The only kind of messages this module has to handle are self messages */
void AnalysisEnergy::handleMessage(cMessage* apMsg)
{
    if (apMsg->isSelfMessage())
    {
        if (apMsg == mCreateSnapshot) // msg not scheduled in initialize!!
        {
            SnapshotEnergies();

            if (simTime() < 1900)
                scheduleAt(2000, mCreateSnapshot);
        }

        return;
    }
    else
    {
        delete apMsg;
    }
}

/** This function gets called by the battery module each time a host energy
    drops to zero
    Then decide whether to take an energy snapshot, a lifetime snapshot, or
    no snapshot at all.
  */
void AnalysisEnergy::Snapshot()
{
    mNumHostsDepleted++;

    if (mNumHostsDepleted == 1)
    {
        SnapshotEnergies();
    }
    else if (mNumHostsDepleted >= mNumHosts)
    {
        SnapshotLifetimes();
        // alle hosts sind tot: simulation beenden!
        endSimulation();
    }

    ev << "hosts depleted: " << mNumHostsDepleted << endl;
}



/////////////////////////////// PRIVATE  ///////////////////////////////////

//============================= OPERATIONS ===================================

void AnalysisEnergy::SnapshotEnergies()
{
    ev << "Creating energy snapshot..." << endl;

    // energy values are written into a file
    // the filename is composited like this:
    // "energies-$network-$run-$time.snapshot
    std::stringstream filename;

    filename << "energies-" << simulation.getSystemModule()->getName() << "-"
             << "-" << simTime() << ".snapshot";
    std::ofstream fout(filename.str().c_str());

    for (int i = 0; i < mNumHosts; i++)
    {
        // build string with module path for each host in mNumHosts
        std::stringstream host_path;
        host_path << mpHostModuleName << "[" << i << "]";

        // test if the host is present
        if (!simulation.getModuleByPath(host_path.str().c_str()))
            error("Host not found");

        host_path << ".battery";

        // read the host energy
        double ene = dynamic_cast<BasicBattery*>(simulation.getModuleByPath(host_path.str().c_str()))->GetEnergy();

        // write the energy values into a file
        fout << host_path << "\t" << ene << endl;
        ev << host_path << ": " << ene << endl;
    }

    fout.close();
}

void AnalysisEnergy::SnapshotLifetimes()
{
    ev << "Creating lifetime snapshot...";

    // lifetime values are written into a file
    // the filename is composited like this:
    // "lifetimes-$network-$run-$time.snapshot
    std::stringstream filename;
    filename << "lifetimes-" << simulation.getSystemModule()->getName() << "-"
             << "-" << simTime() << ".snapshot";
    std::ofstream fout(filename.str().c_str());

    for (int i = 0; i < mNumHosts; i++)
    {
        // build string with module path for each host in mNumHosts
        std::stringstream host_path;
        host_path << mpHostModuleName << "[" << i << "]";

        // test if the host is present
        if (NULL == simulation.getModuleByPath(host_path.str().c_str()))
        {
            error("Host not found");
        }

        host_path << ".battery";

        // read the host lifetime
        simtime_t lt = dynamic_cast<BasicBattery*>(simulation.getModuleByPath(host_path.str().c_str()))->GetLifetime();

        // write the lifetime values into a file
        fout << host_path << "\t" << lt << endl;
        ev << host_path << ": " << lt << endl;
    }

    fout.close();
}
