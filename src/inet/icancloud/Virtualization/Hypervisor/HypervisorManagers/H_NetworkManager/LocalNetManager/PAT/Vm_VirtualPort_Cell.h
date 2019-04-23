//
// This class defines the set of virtual ports open by a virtual machine
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

#ifndef VM_VIRTUALPORT_CELL_H_
#define VM_VIRTUALPORT_CELL_H_

#include <sstream>
#include <vector>
#include <string.h>
#include <map>
#include <stdio.h>

/** Root port */
#define ROOT_PORT 0
/** Initial Registered port number */
#define REGISTERED_INITIAL_PORT 1024
/** Initial dynamic port number */
#define DYNAMIC_INITIAL_PORT 49152
/** Last dynamic port number */
#define LAST_PORT 65535

namespace inet {

namespace icancloud {


using namespace std;

class Vm_VirtualPort_Cell {

protected:

	struct port_structure{
		int rPort;								/** Real Port (uniq) */
		int vPort;								/** Virtual Port */
		int connectionID;
	};

	string vmIP;
	int vmID;								/** Identifier Source, where source = node_name \/ vm_name + user*/
	vector<port_structure*> ports;

public:

	Vm_VirtualPort_Cell();

	virtual ~Vm_VirtualPort_Cell();

	void setVMID (int id);

	void setVMIP (string vmIP);

	void setPort(int realPort, int virtualPort);

	void setConnectionID (int realPort, int connID);

	int getVMID ();

	string getVMIP();

	int getVirtualPort (int realPort);

	int getRealPort (int virtualPort);

	int getConnectionID (int virtualPort);

	vector<int> getAllRPorts ();

	vector<int> getAllConnectionIDs ();

	void deletePort_byConnectionID (int connID);

	void deletePort_byRPort (int realPort);

	void deletePort_byVPort (int virtualPort);
};

} // namespace icancloud
} // namespace inet

#endif /* VM_VIRTUALPORT_CELL_H_ */
