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

#include "inet/icancloud/Users/UserGenerator/core/AbstractUserGenerator.h"

namespace inet {

namespace icancloud {


const string AbstractUserGenerator::OUTPUT_DIRECTORY="results";


AbstractUserGenerator::AbstractUserGenerator() {
    name.empty();
    users_created_counter = -1;
    behavior.empty();
    distributionName.empty();
    distributionParams.clear();
    vmSelect.clear();
    userJobSet.clear();
    remoteFileSystemType.empty();
    userManagementPtr = nullptr;
    logName.clear();
    printResults = false;

}

AbstractUserGenerator::~AbstractUserGenerator() {
    vmSelect.clear();

    for (int i = 0; i < (int)userJobSet.size();){
        userJobSet.erase(userJobSet.begin());
    }

    icancloud_Base::finish();
}

void AbstractUserGenerator::initialize(int stage) {

    // Init the superclass
    icancloud_Base::initialize (stage);

    if (stage == INITSTAGE_LOCAL) {
        struct tm * timeinfo;
        time_t rawtime;
        bool printResults;
        char log_name[NAME_SIZE];

        // Check if current run directory exists...
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // Get the ned parameters
        name = par("name").stringValue();
        behavior = par("behavior").stringValue();
        remoteFileSystemType = par("remoteServers").stringValue();
        printResults = par("printResults").boolValue();

        // Get distribution parameters

        distributionParams.clear();

        distributionName = getParentModule()->getSubmodule("distribution")->par("name").stringValue();
        const char *parameters = getParentModule()->getSubmodule("distribution")->par("params").stringValue();
        cStringTokenizer tokenizer(parameters);

        while (tokenizer.hasMoreTokens())
            distributionParams.push_back(atof(tokenizer.nextToken()));

        if (printResults) {

            // Create the folder (if it doesn't exists)
            DIR *logDir;

            // Check if log Directory exists
            logDir = opendir(OUTPUT_DIRECTORY.c_str());

            // If dir do not exists, then create it!
            if (logDir == nullptr) {

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
                _mkdir(OUTPUT_DIRECTORY.c_str());
#else
                mkdir(OUTPUT_DIRECTORY.c_str(), 00777);
#endif
                //closedir (logDir);
            }
            else {
                closedir(logDir);
            }

            // Continue with the file creation.
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            sprintf(log_name, "%s/%s_%04d-%02d-%02d_%02d:%02d:%02d.txt",
                    "results", OUTPUT_DIRECTORY.c_str(),
                    timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                    timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
                    timeinfo->tm_sec);
            logName = log_name;
        } else {
            logName = "";
        }

        users_created_counter = 0;

        // Define.
        string initialVIP;
        cModule* auxMod;

        // Get the cloud manager pointer
        auxMod = getParentModule()->getParentModule()->getParentModule()->getSubmodule("manager");
        userManagementPtr = check_and_cast<UserManagement*>(auxMod);

        // Create the user core!
        // Initialize ..
        vmSelection* vm;
        int vmsQuantity;
        jobSelection* jobSel;
        int numApps;

        // Get vms configuration
        vmsQuantity = getParentModule()->getSubmodule("vmDefinition")->par(
                "vmsToRentTypesQuantity").intValue();

        for (int i = 0; i < vmsQuantity; i++) {
            vm = new vmSelection();
            vm->vmtype =
                    getParentModule()->getSubmodule("vmDefinition")->getSubmodule("vmToRent", i)->par("name").stringValue();
            vm->quantity =
                    getParentModule()->getSubmodule("vmDefinition")->getSubmodule("vmToRent", i)->par("quantity").intValue();
            vmSelect.push_back(vm);
        }

        // Get the app definition
        numApps = getParentModule()->getSubmodule("appDefinition")->par("appQuantity").intValue();

        for (int i = 0; i < numApps; i++) {
            jobSel = new jobSelection();
            jobSel->replicas =
                    getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                            "application", i)->par("copies").intValue();
            jobSel->appName =
                    getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                            "application", i)->par("name").stringValue();
            auxMod =
                    getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                            "application", i)->getSubmodule("app");
            jobSel->job = dynamic_cast<UserJob*>(auxMod);

            jobSel->job->setOriginalName(jobSel->appName);
            jobSel->job->setAppType(
                    getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                            "application", i)->par("appType").stringValue());
            jobSel->job->setNumCopies(jobSel->replicas);

            // Get the app and set the values..
            fsStructure_T* fs;
            preload_T* pr;
            int numFSRoutes =
                    getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                            "application", i)->par("numFSRoutes").intValue();
            for (int j = 0; j < numFSRoutes; j++) {
                fs = new fsStructure_T();
                fs->fsType =
                        (getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                                "application", i)->getSubmodule("FSConfig", j))->par(
                                "type").stringValue();
                fs->fsRoute =
                        (getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                                "application", i)->getSubmodule("FSConfig", j))->par(
                                "route").stringValue();
                jobSel->job->setFSElement(fs);
            }

            int numFiles =
                    getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                            "application", i)->par("numFiles").intValue();
            for (int j = 0; j < numFiles; j++) {
                pr = new preload_T();
                pr->fileName =
                        (getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                                "application", i)->getSubmodule("preloadFiles",
                                j))->par("name").stringValue();
                pr->fileSize =
                        (getParentModule()->getSubmodule("appDefinition")->getSubmodule(
                                "application", i)->getSubmodule("preloadFiles",
                                j))->par("size_KiB").intValue();
                jobSel->job->setPreloadFile(pr);

            }
            userJobSet.push_back(jobSel);
        }
    }
}

void AbstractUserGenerator::finish(){
    icancloud_Base::finish();
}

cGate* AbstractUserGenerator::getOutGate (cMessage *msg){
    return nullptr;
}

void AbstractUserGenerator::finalizeUserGenerator(bool allowToExecute){

    userManagementPtr->finalizeUserGenerator(this, allowToExecute);
};

void AbstractUserGenerator::createUser() {

    //Set up the behaviorMod
    cModuleType *modBehavior;
    cModule* behaviorMod;
    AbstractUser* user;
    AbstractCloudUser* userVM;
    std::ostringstream behaviorPath;
    cModule* vm;
    SimTime nextEvent;

    ostringstream userNameBuild;
    string userID;
    int vmSelectionSize, vmSelectionQuantity;
    string vmSelectionType;
    string jobName, appName;
    unsigned int j, k, jobSetSize;

    try {
        // Build the name..
        userNameBuild << name << "_" << users_created_counter;
        users_created_counter++;
        userID = userNameBuild.str().c_str();
        printf("AbstractUserGenerator::createUser->%s\n", userID.c_str());

        behaviorPath << "inet.icancloud.Users.Profiles." << behavior.c_str()
                << "." << behavior.c_str();

        // the user behaviorMod is created in the root of the cloud (cloud.manager.userGenerator.user)
        modBehavior = cModuleType::get(behaviorPath.str().c_str());

        behaviorMod = modBehavior->create(userID.c_str(), getParentModule()->getParentModule()->getParentModule());
        behaviorMod->setName(userID.c_str());
        behaviorMod->finalizeParameters();
        behaviorMod->buildInside();
        behaviorMod->callInitialize();

        user = check_and_cast<AbstractUser*>(behaviorMod);
        // Initialize the user ..
        user->initParameters(behavior, userID.c_str(), logName);

        // Get virtual machines selection
        vmSelectionSize = vmSelect.size();

        for (j = 0; ((int) j) < (vmSelectionSize); j++) {

            vmSelectionQuantity = (*(vmSelect.begin() + j))->quantity;
            vmSelectionType = (*(vmSelect.begin() + j))->vmtype;

            cModule* vmImages;
            bool br = false;
            vmImages = getParentModule()->getParentModule()->getParentModule()->getSubmodule("vmSet");

            for (int k = 0; (k < vmImages->getVectorSize()) && (!br); k++) {
                vm = vmImages->getSubmodule("vmImage", k);
                if (strcmp(vm->par("id").stringValue(), vmSelectionType.c_str())
                        == 0) {
                    br = true;
                }
            }

            //vm = getParentModule()->getParentModule()->getParentModule()->getSubmodule("vmSet")->getSubmodule(vmSelectionType.c_str());
            if (vm == nullptr) {
                throw cRuntimeError(
                        "AbstractUserGenerator::createUser ->VM image to create user vms[%s] is unknown\n",
                        vmSelectionType.c_str());
            }
            userVM = check_and_cast<AbstractCloudUser*>(user);
            userVM->createVMSet(vmSelectionQuantity,
                    vm->par("numCores").intValue(),
                    vm->par("memorySize_MB").intValue(),
                    vm->par("storageSize_GB").intValue(), 1,
                    vmSelectionType.c_str());
        }

        // Clone job definitions to be linked to user's waiting queue
        jobSetSize = userJobSet.size();

        for (j = 0; ((unsigned int) j) < jobSetSize; j++) {
            jobSelection* jobSelect;
            UserJob* newJob;
            int rep;

            // Get the job
            jobSelect = (*(userJobSet.begin() + j));
            rep = jobSelect->replicas;

            for (k = 0; ((int) k) < rep; k++) {
                // Clone the job
                ostringstream appNameBuild;
                appNameBuild << jobSelect->appName.c_str() << "_" << k << ":" << userID;
                newJob = cloneJob(jobSelect->job, behaviorMod, appNameBuild.str().c_str());
                // Insert into users waiting queue
                user->addParsedJob(newJob);
            }
        }

        user->setFSType(remoteFileSystemType);

        // Notify to cloud manager the arrival of a new user..
        userManagementPtr->newUser(user);

    } catch (exception& e) {
        throw cRuntimeError("UserGenerator_cell::createUser->Error creating user %s\n", userID.c_str());
    }
}

double AbstractUserGenerator::selectDistribution(){

	double delay_double;

	string distribution = distributionName;

	if (strcmp (distribution.c_str(), "uniform") == 0){

		delay_double = uniform((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "exponential") == 0){

		delay_double = exponential((*(distributionParams.begin())));

	} else if (strcmp (distribution.c_str(), "normal") == 0){

		delay_double = normal((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "truncnormal") == 0){

		delay_double = truncnormal((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "gamma_d") == 0){

		delay_double = gamma_d((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "beta") == 0){

		delay_double = beta((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "erlang_k") == 0){

		delay_double = erlang_k((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "chi_square") == 0){

		delay_double = chi_square((*(distributionParams.begin())));

	} else if (strcmp (distribution.c_str(), "student_t") == 0){

		delay_double = student_t((*(distributionParams.begin())));

	} else if (strcmp (distribution.c_str(), "cauchy") == 0){

		delay_double = cauchy((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "triang") == 0){

		delay_double = triang((*(distributionParams.begin())), (*(distributionParams.begin() + 1)), (*(distributionParams.begin() + 2)));

	} else if (strcmp (distribution.c_str(), "lognormal") == 0){

		delay_double =  lognormal((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "weibull") == 0){

		delay_double = weibull((*(distributionParams.begin())), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "pareto_shifted") == 0){

		delay_double = pareto_shifted((*(distributionParams.begin())), (*(distributionParams.begin() + 1)), (*(distributionParams.begin() + 2)));

	} else if (strcmp (distribution.c_str(), "intuniform") == 0){

	    delay_double = intuniform(static_cast<int> ((*(distributionParams.begin()))), static_cast<int> ((*(distributionParams.begin() + 1))));


	} else if (strcmp (distribution.c_str(), "bernoulli") == 0){

	    delay_double = bernoulli(static_cast<int> ((*(distributionParams.begin()))));

	} else if (strcmp (distribution.c_str(), "binomial") == 0){

	    delay_double = binomial(static_cast<int>((*(distributionParams.begin()))), (*(distributionParams.begin() + 1)));

	} else if (strcmp (distribution.c_str(), "geometric") == 0){

	    delay_double = geometric(static_cast<int> ((*(distributionParams.begin()))));

	} else if (strcmp (distribution.c_str(), "negbinomial") == 0){

	    delay_double = negbinomial(static_cast<int> ((*(distributionParams.begin()))), static_cast<int> ((*(distributionParams.begin() + 1))));

	} else if (strcmp (distribution.c_str(), "poisson") == 0){

	    delay_double = poisson(static_cast<int> ((*(distributionParams.begin()))));

	} else {
	    throw cRuntimeError("Unknown user generator distribution: %s\n", distribution.c_str());
	}

	return delay_double;

}

UserJob* AbstractUserGenerator::cloneJob (UserJob* app, cModule* userMod, string appName){

    // Define ...
        cModule *cloneApp;
        AbstractUser* user;
        UserJob* newJob;
        cModuleType *modType;
        std::ostringstream appPath;
        int i, numParameters, size;

    // Init ..
        appPath << app->getNedTypeName();

    // Create the app module
        modType = cModuleType::get (appPath.str().c_str());

    // Create the app into the user module
        cloneApp = modType->create(appPath.str().c_str(), userMod);

    // Configure the main parameters
        numParameters = app->getNumParams();
        for (i = 0; i < numParameters ; i++){
            cloneApp->par(i) = app->par(i);
        }

        cloneApp->setName("app");

    // Finalize and build the module
        cloneApp->finalizeParameters();
        cloneApp->buildInside();

        // Call initiialize
        cloneApp->callInitialize();

        newJob = check_and_cast<UserJob*> (cloneApp);
        newJob->setAppType(appName.c_str());
        newJob->setOriginalName(appName.c_str());
        newJob->setAppType(app->getAppType());

        user =  check_and_cast <AbstractUser*>  (userMod);
        newJob->setUpUser(user);

        size = app->getPreloadSize();
        for (i = 0; i < size; i++){
            newJob->setPreloadFile(app->getPreloadFile(i));
        }

        size = app->getFSSize();
        for (i = 0; i < size; i++){
            newJob->setFSElement(app->getFSElement(i));
        }

        return newJob;
}


} // namespace icancloud
} // namespace inet
