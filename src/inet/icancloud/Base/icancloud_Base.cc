#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <direct.h>
#elif defined __GNUC__
#include <sys/types.h>
#include <sys/stat.h>
#endif


#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


const string icancloud_Base::STARTED_MODULES_FILE="Started_Modules.txt";
const string icancloud_Base::LOG_MESSAGES_FILE="Log_Messages.txt";
const string icancloud_Base::DEBUG_MESSAGES_FILE="Debug_Messages.txt";
const string icancloud_Base::ERROR_MESSAGES_FILE="Error_Messages.txt";
const string icancloud_Base::RESULTS_MESSAGES_FILE="Results.txt";
const string icancloud_Base::OUTPUT_DIRECTORY="output";


void createDir(string dir) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
    _mkdir(dir.data());
#else
    mkdir(dir.data(), 0777);
#endif
}


icancloud_Base::~icancloud_Base(){
	
     // delete(latencyMessage);
    //    latencyMessage = nullptr;

//	struct stat info;

		// Write messages to file?
		if (WRITE_MESSAGES_TO_FILE){
	
			// Close all files!
			close (fdStartedModules);
			close (fdLog);
			close (fdDebug);
			close (fdError);			
		}		

//		// Check the size of result file!
//		stat (resultPath, &info);
//
//		if (info.st_size == 0)
//			unlink (resultPath);
//		else
//			close (fdResults);

}


void icancloud_Base::initialize(int stage) {

    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        char module_path[NAME_SIZE];
        char currentHostName[NAME_SIZE];
        string currentRunPath;

        // Init common attributes
        currentRequest = 0;
        latencyMessage = nullptr;
        queue.clear();

//            // Check if output folder exists...
//            checkDir (OUTPUT_DIRECTORY.c_str());
//
//            // Check the current run directory
//            currentRunPath = checkCurrentRunDir();
//
//            // Results messages!
//            sprintf (resultPath, "%s/Results.txt", currentRunPath.c_str());

        // Create log files...
        if (WRITE_MESSAGES_TO_FILE) {

            // Creates current module folder!
#ifdef WITH_PARSIM
            memset(currentHostName, 0, NAME_SIZE);
            gethostname (currentHostName, NAME_SIZE);
            sprintf (module_path, "%s/%s-%d", currentRunPath.c_str(), currentHostName, getId());

#else
            sprintf(module_path, "%s/%d", currentRunPath.c_str(), getId());
#endif

            checkDir(module_path);

            // Creates Started module file!
            if (SHOW_STARTED_MODULE_MESSAGES) {
                sprintf(startedModulesPath, "%s/%s", currentRunPath.c_str(),
                        STARTED_MODULES_FILE.c_str());
            }

            // Creates log messages file!
            if (SHOW_LOG_MESSAGES) {

                sprintf(logPath, "%s/%s", module_path,
                        LOG_MESSAGES_FILE.c_str());
                fdLog = open(logPath, O_CREAT | O_TRUNC | O_RDWR, 00777);

                // Error opening log file
                if (fdLog < 0) {
                    perror("Error opening log File");
                    throw cRuntimeError("Error opening log File");
                } else
                    close(fdLog);
            }

            // Creates debug messages file!
            if (SHOW_DEBUG_MESSAGES) {

                sprintf(debugPath, "%s/%s", module_path,
                        DEBUG_MESSAGES_FILE.c_str());
                fdDebug = open(debugPath, O_CREAT | O_TRUNC | O_RDWR, 00777);

                // Error opening debug file
                if (fdDebug < 0) {
                    perror("Error opening debug File");
                    throw cRuntimeError("Error opening debug File");
                } else
                    close(fdDebug);
            }

            // Creates error messages file!
            if (SHOW_ERROR_MESSAGES) {

                sprintf(errorPath, "%s/%s", module_path,
                        ERROR_MESSAGES_FILE.c_str());
                fdError = open(errorPath, O_CREAT | O_TRUNC | O_RDWR, 00777);

                // Error opening error file
                if (fdError < 0) {
                    perror("Error opening error File");
                    throw cRuntimeError("Error opening error File");
                } else
                    close(fdError);
            }
        }

        // Log that this module starts
        showStartedModule(" ");
    }

}


void icancloud_Base::handleMessageWhenUp(cMessage *msg) {

    // icancloud_Message *sm;

    // If msg is a Self Message...
    if (msg->isSelfMessage())
        processSelfMessage(msg);

    // Not a self message...
    else {
        auto pkt = check_and_cast<Packet *>(msg);

        const auto &sm = pkt->peekAtFront<icancloud_Message>();

        // Casting to icancloud_Message
        if (sm) {
            // Request message, upload message trace and send to destination module!
            if (!sm->getIsResponse()) {

                // Update message trace
                updateMessageTrace(pkt);
                // Insert into queue
                queue.insert(pkt);
            }
            // Response message!
            else {
                processResponseMessage(pkt);
            }
        }
        // msg is not a icancloud Message!ºº
        else
            showErrorMessage("Unknown received message [%s]", msg->getName());
    }
    // If not processing any request...
    processCurrentRequestMessage();
}

void icancloud_Base::finish() {
    cSimpleModule::finish();
    //    cancelEvent(latencyMessage);
    //    latencyMessage = nullptr;
}

void icancloud_Base::sendRequestMessage(Packet * pkt, cGate* gate) {

    pkt->trim();
    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    // If trace is empty, add current hostName, module and request number
    if (sm->isTraceEmpty()) {
        pkt->trimFront();
        auto sm = pkt->removeAtFront<icancloud_Message>();
        sm->addNodeToTrace(getHostName());
        pkt->insertAtFront(sm);
        updateMessageTrace(pkt);
    }

    // Send the message!
    send(pkt, gate);

    // Process next request!
    processCurrentRequestMessage();
}


void icancloud_Base::sendResponseMessage (Packet * pkt)
{

    pkt->trim();
    auto sm = pkt->removeAtFront<icancloud_Message>();

    int gateId;
    // Get the gateId to send back the message
    gateId = sm->getLastGateId ();
    // Removes the current module from trace...
    sm->removeLastModuleFromTrace ();
    // Send back the message
    pkt->insertAtFront(sm);
    send (pkt, gateId);
    // Process next request!
    processCurrentRequestMessage ();
}


void icancloud_Base::showStartedModule(const char *args, ...) {

    char currentHostName[NAME_SIZE];
    va_list ap;
    bool isEnd;
    char aux[2 * NAME_SIZE];
    char aux2[2 * NAME_SIZE];
    char aux3[2 * NAME_SIZE];
    char type;
    int ret;
    std::ostringstream msgLine;

    char *s;
    int c;
    int d;
    double f;
    unsigned int u;
    unsigned long int U;
    unsigned long long int w;

    if (SHOW_STARTED_MODULE_MESSAGES) {

        // Init
        isEnd = 0;
        va_start(ap, args);
        strcpy(aux, args);
        msgLine.str("");

#ifdef WITH_PARSIM
        memset (currentHostName, 0, NAME_SIZE);
        gethostname (currentHostName, NAME_SIZE);
        msgLine << "ModuleID:" << currentHostName << "-" << getId() << " -> " << getFullPath() << endl;

#else
        msgLine << "ModuleID:" << getId() << " -> " << getFullPath() << endl;
#endif

        while (!isEnd) {

            // Parse current line!
            ret = sscanf(aux, "%[^%\n]%%%c%[^\n]", aux2, &type, aux3);

            // End of args!
            if (ret == 0)
                isEnd = 1;

            // One parameter...
            else if (ret == 1) {
                msgLine << aux2;
                isEnd = 1;
            }

            // Two parameters...
            else if ((ret == 2) || (ret == 3)) {

                msgLine << aux2;

                // Type?
                switch (type) {

                // String!
                case 's':
                    s = va_arg(ap, char *);
                    msgLine << s;
                    break;

                    // Char!
                case 'c':
                    c = va_arg(ap, int);
                    msgLine << (char) c;
                    break;

                    // Int!
                case 'd':
                    d = va_arg(ap, int);
                    msgLine << d;
                    break;

                    // Double!
                case 'f':
                    f = va_arg(ap, double);
                    msgLine << f;
                    break;

                    // Unsigned int!
                case 'u':
                    u = va_arg(ap, unsigned int);
                    msgLine << u;
                    break;

                    // Unsigned long int!
                case 'U':
                    U = va_arg(ap, unsigned long int);
                    msgLine << U;
                    break;

                    // Unsigned long long int!
                case 'w':
                    w = va_arg(ap, unsigned long long int);
                    msgLine << w;
                    break;
                }

                if (ret == 3)
                    strcpy(aux, aux3);
                else
                    isEnd = 1;
            }
        }

        // End of parsing!
        msgLine << endl;
        va_end(ap);

        // Show message in GUI
        if (SHOW_GUI_MESSAGES)
            EV << msgLine.str();

        // Show message in console
        if (SHOW_CONSOLE_MESSAGES)
            printf("%s", msgLine.str().c_str());

        // Write message to file
        if (WRITE_MESSAGES_TO_FILE) {

            // If file exists, open it, else, create it!
            if ((fdStartedModules = open(startedModulesPath, O_APPEND | O_RDWR,
                    00777)) == -1)
                fdStartedModules = open(startedModulesPath,
                        O_CREAT | O_APPEND | O_RDWR, 00777);

            // Error opening started modules file
            if (fdStartedModules < 0) {
                perror("Error opening Started Modules File");
                throw cRuntimeError("Error opening Started Modules File");
                exit(0);
            } else {
                write(fdStartedModules, msgLine.str().c_str(),
                        msgLine.str().length());
                close(fdStartedModules);
            }
        }
    }
}


void icancloud_Base::showLogMessage (string logMsg){

	std::ostringstream msgLine;

		if (SHOW_LOG_MESSAGES){

			// Prepare the message line!
			msgLine.str("");
			msgLine << "[" << simTime() << "]" << "(" << getId() << ") LOG: " << logMsg << endl;

			// Show message in GUI
			if (SHOW_GUI_MESSAGES)
				EV 	<< msgLine.str();

			// Show message in console
			if (SHOW_CONSOLE_MESSAGES)
				printf ("%s", msgLine.str().c_str());
				
			// Write message to file
			if (WRITE_MESSAGES_TO_FILE){

				fdLog = open (logPath, O_APPEND | O_RDWR, 00777);

				if (fdLog<0){
					perror ("Error opening Log File (on write).");
					throw cRuntimeError ("Error opening Log File (on write)");
					exit(0);
				}
				else{
					write (fdLog, msgLine.str().c_str(), msgLine.str().length());
					close (fdLog);
				}
			}
		}
}


void icancloud_Base::showDebugMessage (const char *args, ...){

	va_list ap;
	bool isEnd;
	char aux [2*NAME_SIZE];
	char aux2 [2*NAME_SIZE];
	char aux3 [2*NAME_SIZE];
	char type;
	int ret;
	std::ostringstream msgLine;

	char *s;
	int c;
	int d;
	double f;
	unsigned int u;
	unsigned long int U;
	unsigned long long int w;


		if (SHOW_DEBUG_MESSAGES){

			// Init
			isEnd = 0;
			va_start(ap, args);
			strcpy (aux, args);
			msgLine.str("");
			msgLine << "[" << simTime() << "]" << "(" << getHostName() << "." << moduleIdName << ") DEBUG: ";

				while (!isEnd){

                   	// Parse current line!
					ret = sscanf (aux, "%[^%\n]%%%c%[^\n]", aux2, &type, aux3);

						// End of args!
						if (ret==0)
							isEnd=1;

						// One parameter...
						else if (ret==1){
							msgLine << aux2;
							isEnd=1;
						}

						// Two parameters...
						else if ((ret==2) || (ret==3)){

							msgLine << aux2;

							// Type?
							switch (type){

								// String!
								case 's':
									s = va_arg(ap, char *);
									msgLine << s;
									break;

								// Char!
								case 'c':
									c = va_arg(ap, int);
									msgLine << (char)c;
									break;

								// Int!
								case 'd':
                             		d = va_arg(ap, int);
                             		msgLine << d;
                             		break;

                             	// Double!
								case 'f':
                             		f = va_arg(ap, double);
                             		msgLine << f;
                             		break;

                             	// Unsigned int!
								case 'u':
                             		u = va_arg(ap, unsigned int);
                             		msgLine << u;
                             		break;

                             	// Unsigned long int!
								case 'U':
                             		U = va_arg(ap, unsigned long int);
                             		msgLine << U;
                             		break;

                             	// Unsigned long long int!
								case 'w':
                             		w = va_arg(ap, unsigned long long int);
                             		msgLine << w;
                             		break;
							}


							if (ret==3)
								strcpy (aux, aux3);
							else
								isEnd=1;
						}
				}

				// End of parsing!
				msgLine << endl;
				va_end(ap);

				// Show message in GUI
				if (SHOW_GUI_MESSAGES)
					EV 	<< msgLine.str();

				// Show message in console
				if (SHOW_CONSOLE_MESSAGES)
					printf ("%s", msgLine.str().c_str());					

				// Write message to file
				if (WRITE_MESSAGES_TO_FILE){

					fdDebug = open (debugPath, O_APPEND | O_RDWR, 00777);

					if (fdDebug<0){
						perror ("Error opening Debug File (on write).");
						throw cRuntimeError ("Error opening Debug File (on write)");
						exit(0);
					}
					else{
						write (fdDebug, msgLine.str().c_str(), msgLine.str().length());
						close (fdDebug);
					}
				}
		}
}


void icancloud_Base::showBigDebugMessage (string message){

//	// TODO This function does not work well.
//	// Simply, splits it into a const slices and call showDebugMessage function!
//
//	size_t init_ptr;
//	size_t last_ptr;
//	size_t last_found;
//	size_t current_found;
//
//
//		// Init
//		init_ptr = last_found = current_found =0;
//		last_ptr = NAME_SIZE;
//
//		while (init_ptr<message.size()){
//
//			current_found = message.find_last_of ("\n", last_ptr);
//
//			// Found!
//			if (current_found!=string::npos){
//
//				//printf ("Salto encontrado -> ");
//
//				// Previous block...
//				if (current_found==last_found){
//					showDebugMessage (message.substr(init_ptr, NAME_SIZE).c_str());
//					//printf ("%s\n", message.substr(init_ptr, NAME_SIZE).c_str());
//					init_ptr += NAME_SIZE+1;
//					last_ptr = init_ptr + NAME_SIZE;
//				}
//
//				// New block...
//				else{
//					showDebugMessage (message.substr(init_ptr, current_found-init_ptr).c_str());
//					//printf ("%s\n", message.substr(init_ptr, current_found-init_ptr).c_str());
//					last_found = current_found;
//					init_ptr = current_found+1;
//					last_ptr = init_ptr + NAME_SIZE;
//				}
//			}
//
//			// not found!
//			else{
//				showDebugMessage (message.substr(init_ptr, NAME_SIZE).c_str());
//				//printf ("Santo no encontrado -> %s\n", message.substr(init_ptr, NAME_SIZE).c_str());
//				init_ptr += NAME_SIZE +1;
//				last_ptr = init_ptr + NAME_SIZE;
//			}
//		}
}


void icancloud_Base::showErrorMessage (const char *args, ...){

	va_list ap;
	bool isEnd;
	char aux [NAME_SIZE];
	char aux2 [NAME_SIZE];
	char aux3 [NAME_SIZE];
	char type;
	int ret;
	std::ostringstream msgLine;

	char *s;
	int c;
	int d;
	double f;
	unsigned int u;
	unsigned long int U;
	unsigned long long int w;


		if (SHOW_ERROR_MESSAGES){

			// Init
			isEnd = 0;
			va_start(ap, args);
			strcpy (aux, args);
			msgLine.str("");
			msgLine << "[" << simTime() << "]" << "(" << getFullPath() << ") ERROR: ";

				while (!isEnd){

                   	// Parse current line!
					ret = sscanf (aux, "%[^%\n]%%%c%[^\n]", aux2, &type, aux3);

						// End of args!
						if (ret==0)
							isEnd=1;

						// One parameter...
						else if (ret==1){
							msgLine << aux2;
							isEnd=1;
						}

						// Two parameters...
						else if ((ret==2) || (ret==3)){

							msgLine << aux2;

							// Type?
							switch (type){

								// String!
								case 's':
									s = va_arg(ap, char *);
									msgLine << s;
									break;

								// Char!
								case 'c':
									c = va_arg(ap, int);
									msgLine << (char)c;
									break;

								// Int!
								case 'd':
                             		d = va_arg(ap, int);
                             		msgLine << d;
                             		break;

                             	// Double!
								case 'f':
                             		f = va_arg(ap, double);
                             		msgLine << f;
                             		break;

                             	// Unsigned int!
								case 'u':
                             		u = va_arg(ap, unsigned int);
                             		msgLine << u;
                             		break;

                             	// Unsigned long int!
								case 'U':
                             		U = va_arg(ap, unsigned long int);
                             		msgLine << U;
                             		break;

                             	// Unsigned long long int!
								case 'w':
                             		w = va_arg(ap, unsigned long long int);
                             		msgLine << w;
                             		break;
							}


							if (ret==3)
								strcpy (aux, aux3);
							else
								isEnd=1;
						}
				}

				// End of parsing!
				msgLine << endl;
				va_end(ap);

				// Show message in GUI
				if (SHOW_GUI_MESSAGES)
					EV 	<< msgLine.str();

				// Show message in console
				if (SHOW_CONSOLE_MESSAGES)
					printf ("%s", msgLine.str().c_str());
				
				// Write message to file
				if (WRITE_MESSAGES_TO_FILE){

					fdError = open (errorPath, O_APPEND | O_RDWR, 00777);

					if (fdError<0){
						perror ("Error opening Error File (on write).");
						throw cRuntimeError ("Error opening Error File (on write)");
						exit(0);
					}
					else{
						write (fdError, msgLine.str().c_str(), msgLine.str().length());
						close (fdError);
					}
				}
		}

	// Throws an exception to stop the execution!
	throw cRuntimeError(msgLine.str().c_str());
}


void icancloud_Base::showResultMessage (const char *args, ...){

	va_list ap;
	bool isEnd;
	char aux [NAME_SIZE];
	char aux2 [NAME_SIZE];
	char aux3 [NAME_SIZE];
	char type;
	int ret;
	std::ostringstream msgLine;

	char *s;
	int c;
	int d;
	double f;
	unsigned int u;
	unsigned long int U;
	unsigned long long int w;


		// Init
		isEnd = 0;
		va_start(ap, args);
		strcpy (aux, args);
		msgLine.str("");
		msgLine << "[" << simTime() << "]" << "(" << getFullPath() << ") ";

			while (!isEnd){

               	// Parse current line!
				ret = sscanf (aux, "%[^%\n]%%%c%[^\n]", aux2, &type, aux3);

					// End of args!
					if (ret==0)
						isEnd=1;

					// One parameter...
					else if (ret==1){
						msgLine << aux2;
						isEnd=1;
					}

					// Two parameters...
					else if ((ret==2) || (ret==3)){

						msgLine << aux2;

						// Type?
						switch (type){

							// String!
							case 's':
								s = va_arg(ap, char *);
								msgLine << s;
								break;

							// Char!
							case 'c':
								c = va_arg(ap, int);
								msgLine << (char)c;
								break;

							// Int!
							case 'd':
                         		d = va_arg(ap, int);
                         		msgLine << d;
                         		break;

                         	// Double!
							case 'f':
                         		f = va_arg(ap, double);
                         		msgLine << f;
                         		break;

                         	// Unsigned int!
							case 'u':
                         		u = va_arg(ap, unsigned int);
                         		msgLine << u;
                         		break;

                         	// Unsigned long int!
							case 'U':
                         		U = va_arg(ap, unsigned long int);
                         		msgLine << U;
                         		break;

                         	// Unsigned long long int!
							case 'w':
                         		w = va_arg(ap, unsigned long long int);
                         		msgLine << w;
                         		break;
						}


						if (ret==3)
							strcpy (aux, aux3);
						else
							isEnd=1;
					}
			}

			// End of parsing!
			msgLine << endl;
			va_end(ap);

			// Show message in GUI
			if (SHOW_GUI_MESSAGES)
				EV 	<< msgLine.str();

			// Show message in console
			if (SHOW_CONSOLE_MESSAGES)
				printf ("%s", msgLine.str().c_str());				

			// Open or create results file!
			if ((fdResults = open (resultPath, O_APPEND | O_RDWR, 00777)) == -1)
				fdResults = open (resultPath, O_CREAT | O_APPEND | O_RDWR, 00777);

			// Error opening results file?
			if (fdResults<0){
				perror ("Error opening Result File (on write)");
				throw cRuntimeError ("Error opening Result File (on write)");
				exit(0);
			}
			else{
				write (fdResults, msgLine.str().c_str(), msgLine.str().length());
				close (fdResults);
			}
}

vector<string> icancloud_Base::divide(const char* inputString, char separator){
    vector<string> parts;

    parts.clear();

    std::istringstream f(inputString);

    std::string s;

    while (std::getline(f, s, separator)) {
        parts.push_back(s);
    }

    return parts;

}


void icancloud_Base::updateMessageTrace (Packet *pkt){

	int gateId;

    // If gate==nullptr, first module, there is no previoud gate!
    if ((getOutGate (pkt))==nullptr)
        gateId = NO_GATE_ID;
    else
        gateId = getOutGate (pkt)->getId();
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    sm->addModuleToTrace (getId(), gateId, currentRequest);
    pkt->insertAtFront(sm);
	// Add the current module to trace...
    // Set the update trace
    currentRequest++;
}


string icancloud_Base::getHostName() {

    cModule *current_Module;
    cModule *parent_Module;
    string hostName;
    bool found;
    bool end;

    // Init!
    found = end = false;
    current_Module = this;

    // While not found and not raise the top of the hierarchy...
    while ((!found) && (!end)) {

        // Get the current module's parent module!
        parent_Module = current_Module->getParentModule();

        // End of the hierarchy! or get the hostName!
        if (parent_Module == nullptr)
            end = true;
        else {
            //const char *newHostName = parent_Module->par ("hostName");
            const char *newHostName = parent_Module->getAncestorPar("hostName");

            if (newHostName != nullptr) {
                hostName = newHostName;
                found = true;
            }
        }
    }

    // If hostName not found!
    if (end)
        showErrorMessage("Host name not found!");

    return hostName;
}

void icancloud_Base::processCurrentRequestMessage() {

    //icancloud_Message *sm;
    inet::Packet *unqueuedMessage;

    // There is no pending request!
    //if (!isPendingRequest()) {

    // While exists enqueued requests
    if (!queue.isEmpty() && !isPendingRequest()) {

        // Pop!
        unqueuedMessage = check_and_cast<Packet *>(queue.pop());

        // Dynamic cast!

        const auto &sm = CHK(unqueuedMessage->peekAtFront<icancloud_Message>());
        if (sm == nullptr)
            throw cRuntimeError("");
        //sm = check_and_cast<icancloud_Message *>(unqueuedMessage);
        // Process
        processRequestMessage(unqueuedMessage);
    }
}
//}


bool icancloud_Base::isPendingRequest() {
    // Check if latencyMsg is not nullptr!!!
    if (latencyMessage != nullptr) {
        if (latencyMessage->isScheduled()) {
            return true;
        }
    }
    return false;
}


void icancloud_Base::checkDir (const char *path){

	DIR *logDir;

		// Check if log Directory exists
		logDir = opendir (path);

		// If dir do not exists, then create it!
		if (logDir==nullptr){
			//mkdir (path, 00777);
		    createDir (path);
			//closedir (logDir);
		}
		else{
			closedir (logDir);
		}
}


string icancloud_Base::checkCurrentRunDir (){

	struct tm * timeinfo;
	time_t rawtime;
	char run_path [NAME_SIZE];
	string runPath;


		// Check if current run directory exists...
		time (&rawtime);
		timeinfo = localtime (&rawtime);

		sprintf (run_path, "%s/run.%04d-%02d-%02d_%02d-%02d-%02d", OUTPUT_DIRECTORY.c_str(),
											timeinfo->tm_year+1900,
											timeinfo->tm_mon+1,
											timeinfo->tm_mday,
											timeinfo->tm_hour,
											timeinfo->tm_min,
											timeinfo->tm_sec);



		checkDir (run_path);

		runPath = run_path;

	return runPath;
}


string icancloud_Base::boolToString (bool value){

	string result;

		if (value)
			result = "true";
		else
			result = "false";

	return result;
}


} // namespace icancloud
} // namespace inet
