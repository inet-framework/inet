/**
 *
 * @class VirtualIPCell VirtualIPCell.cc VirtualIPCell.h
 *
 * This class manage the ports from a virtualized environment to a real environment. It is responsible for assign a real port to a
 * virtual one, to free and translate them.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-12-11
 */


#ifndef PORT_TABLE_H_
#define PORT_TABLE_H_

#include <sstream>
#include <vector>
#include "stdio.h"
#include "string.h"
#include "inet/icancloud/Base/include/Constants.h"

namespace inet {

namespace icancloud {


using std::string;
using std::pair;
using std::vector;

// Ports status
static const int PORT_OPEN = 0;
static const int PORT_NOT_FOUND = -1;

static const int LISTEN = 1;
static const int CONNECT = 2;

class PortTable {

protected:

	struct realPorts{
		int vmID;
		int user;
		int rPort;
		int operation;
	};

	struct ports{
		int vPort;
		vector<realPorts*> rPorts;
	};

	string ipNode;
	vector <ports*> virtualPorts;

public:
	PortTable();
	virtual ~PortTable();

	void initialize(string ip_node);
	string getIPNode();

	void registerPort(int rPort, int vPort, int user, int vmID, int operation);
	int getRealPort (int vPort, int vmID, int user);
	int getOpenRealPort  (int vPort, int user, int vmID);

	void freeVMPorts (int vmID, int user);
	int freePort (int vmID, int user, int virtualPort);


};

} // namespace icancloud
} // namespace inet

#endif /* VIRTUALIPS_H_ */
