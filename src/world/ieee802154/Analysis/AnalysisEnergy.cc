
#include "AnalysisEnergy.h"

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
        mCoreDebug      = hasPar("coreDebug") ? (bool) par("coreDebug") : false;
        mNumHosts       = par("numHosts");
        mpHostModuleName.assign(par("hostModuleName").stdstringValue());

        mpCc = dynamic_cast<ChannelControl *>(simulation.getModuleByPath("channelcontrol"));
        if (0 == mpCc)
        {
            error("Could not find channelcontrol module");
        }

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

    for (int i=0; i<mNumHosts; i++)
    {
        // build string with module path for each host in mNumHosts
        std::stringstream host_path;
        host_path << mpHostModuleName << "[" << i << "]";

        // test if the host is present
        if (NULL == simulation.getModuleByPath(host_path.str().c_str()))
        {
            error("Host not found");
        }

        // read the host position
        mTempHostRef = mpCc->lookupHost(getParentModule()->
                                        getModuleByRelativePath(host_path.str().c_str()));
        Coord pos = mpCc->getHostPosition(mTempHostRef);

        host_path << ".battery";

        // read the host energy
        double ene = dynamic_cast<BasicBattery*>(simulation.getModuleByPath(host_path.str().c_str()))->GetEnergy();

        // write the energy values into a file
        fout << pos.x << "\t" << pos.y << "\t" << ene << endl;
        ev << pos.x << "/" << pos.y << ", " << ene << endl;
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

    for (int i=0; i<mNumHosts; i++)
    {
        // build string with module path for each host in mNumHosts
        std::stringstream host_path;
        host_path << mpHostModuleName << "[" << i << "]";

        // test if the host is present
        if (NULL == simulation.getModuleByPath(host_path.str().c_str()))
        {
            error("Host not found");
        }

        // read the host position
        mTempHostRef = mpCc->lookupHost(getParentModule()->
                                        getModuleByRelativePath(host_path.str().c_str()));
        Coord pos = mpCc->getHostPosition(mTempHostRef);

        host_path << ".battery";

        // read the host lifetime
        simtime_t lt = dynamic_cast<BasicBattery*>(simulation.getModuleByPath(host_path.str().c_str()))->GetLifetime();

        // write the lifetime values into a file
        fout << pos.x << "\t" << pos.y << "\t" << lt << endl;
        ev << pos.x << "/" << pos.y << ", " << lt << endl;
    }

    fout.close();

}
