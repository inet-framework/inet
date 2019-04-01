//
// Module interface that defines a cell that makes users appear at system.
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

package inet.icancloud.Users.UserGenerator.core.CustomGeneratorCells;

moduleinterface IUserGenerator
{
    parameters:
        // User parameters
     	string name;											// Name of the user
	 	string behavior;										// Behaviour (profile) name 
        int quantity_user_preloaded;							// Users preloaded at the system before the distribution begins
        int total_users;										// Total users to be created per total time creation.
        int repetitions; 										// # Repetitions of the behavior. -1 infinite.        
        string remoteServers;           						// The name of the remote servers that user will choose
        int vmsToRentTypesQuantity;								// the quantity of different vms that are selected by user	
        double time_creation @unit(s);									// The amount of time that this module will continue creating tenants.
        bool printResults;										// Flag to print results to a file.
}