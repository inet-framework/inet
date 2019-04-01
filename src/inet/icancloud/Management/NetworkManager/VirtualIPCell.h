/**
 *
 * @class VirtualIPCell VirtualIPCell.cc VirtualIPCell.h
 *
 * This class represent a virtual machine and its virtual ip and the ip of the node where it is linked to.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-12-11
 */

#ifndef VIRTUALIPCELL_H_
#define VIRTUALIPCELL_H_

#include "inet/icancloud/Management/NetworkManager/VirtualIPs.h"

namespace inet {

namespace icancloud {


class VirtualIPCell {

protected:

	VirtualIPs* virtualIP;
	string ipNode;
	int vmID;

public:
	VirtualIPCell();
	virtual ~VirtualIPCell();

	void setVirtualIP(string newVirtualIP);
	string getVirtualIP();

	void setIPNode(string id);
	string getIPNode();

	void setVMID(int vm_id);
	int getVMID();

	string getHole(string ipBasis);


};

} // namespace icancloud
} // namespace inet

#endif /* VIRTUALIPCELL_H_ */
