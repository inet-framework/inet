//
// This class defines the set of virtual ports used by a tenant
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

#ifndef USER_VIRTUALPORT_CELL_H_
#define USER_VIRTUALPORT_CELL_H_

#include "Vm_VirtualPort_Cell.h"

namespace inet {

namespace icancloud {


class User_VirtualPort_Cell {

protected:

	int user;
	vector<Vm_VirtualPort_Cell*> vm_ports;

public:

	User_VirtualPort_Cell();
	virtual ~User_VirtualPort_Cell();

	void setUserID (int userID);
	int getUserID ();
	Vm_VirtualPort_Cell* searchVM(int vmID);
	Vm_VirtualPort_Cell* newVM(int vmID);
	void eraseVM(int vmID);
	int getVM_Size();
};

} // namespace icancloud
} // namespace inet

#endif /* USER_VIRTUALPORT_CELL_H_ */
