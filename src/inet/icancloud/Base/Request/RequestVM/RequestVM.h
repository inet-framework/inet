/*
 * @class RequestVM RequestVM.h "Base/Request/RequestVM/RequestVM.h"
 *
 * This class define and manages a virtual machine request by a tenant
 * it offers methods for:
 *      managing single vm request
 *      analyzing the resources requested
 * @author Gabriel Gonzalez Castane
 * @date 2013-05-04
 */

#ifndef REQUESTVM_H_
#define REQUESTVM_H_

#include "inet/icancloud/Virtualization/VirtualMachines/VM.h"
#include "inet/icancloud/Base/Request/Request.h"

namespace inet {

namespace icancloud {


class VM;

class RequestVM : public AbstractRequest{

private:

    // To return the started VMs
	vector<VM*> vmSet;
    vector<connection_T*> connections;

    // To request vms
    struct userVmType_t{
        elementType* type;
        int quantity;
    };

    typedef userVmType_t userVmType;

    vector<userVmType*> vmsToBeSelected;

public:

	RequestVM();

	RequestVM(int userID, int op, vector<VM*> newVMSet);

	virtual ~RequestVM();

	int getVMQuantity(){return vmSet.size();};
	VM* getVM(int position);
	void eraseVM(int position);
	int getNumberVM (){return vmSet.size();};
	void setVectorVM(vector<VM*> newVMSet){vmSet = newVMSet;};
    vector<VM*> getVectorVM(){return vmSet;};

	void setConnection(connection_T* c){connections.push_back(c);};
	connection_T* getConnection(int index){return (*(connections.begin()+index));};
	int getConnectionSize(){return connections.size();};
	void eraseConnection(int i){connections.erase(connections.begin()+i);};
	void eraseConnection(){connections.clear();};

	// Methods for analyzing the resources requested
	void setNewSelection(string typeName, int quantity);
	int getDifferentTypesQuantity(){return vmsToBeSelected.size();};
	string getSelectionType(int indexAtSelection);
	int getSelectionQuantity(int indexAtSelection);
	void eraseSelectionType(int indexAtSelection);
	void decreaseSelectionQuantity(int indexAtSelection);

	// Methods for managing single vm request
	void cleanSelectionType(){vmsToBeSelected.clear();}
	void setForSingleRequest(elementType* el);
	elementType* getSingleRequestType(){return (*(vmsToBeSelected.begin()))->type;};

	RequestVM* dup ();

	bool compareReq(AbstractRequest* req);

};

} // namespace icancloud
} // namespace inet

#endif /* REQUESTVM_H_ */
