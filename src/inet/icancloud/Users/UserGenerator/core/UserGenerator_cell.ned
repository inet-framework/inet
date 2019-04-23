//
// Module that defines a cell that generates users.
//
// It is composed by four groups:
//	-VmDefinition: The parameters of a virtual machine at system that a user wants to rent.
//	-DistributionDefinition: Statistical features that the users appearement has to follow
//	-AppDefinition: The definition of the set of apps that each user will launch
//	-IUSerGenerator: Module interface to create users
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

package inet.icancloud.Users.UserGenerator.core;

import inet.icancloud.Users.UserGenerator.VmDefinition.VmDefinition;
import inet.icancloud.Users.UserGenerator.DistributionDefinition.DistributionDefinition;
import inet.icancloud.Users.UserGenerator.AppDefinition.AppDefinition;
import inet.icancloud.Users.UserGenerator.core.CustomGeneratorCells.IUserGenerator;

module UserGenerator_cell
{
    parameters:
        // User parameters
        string name;
        string behavior;
        int quantity_user_preloaded;							// Users preloaded at the system before the distribution begins
        int total_users;										// Total users to be created per total time creation.
        double time_creation @unit(h);									// Time creation in hours of users in the system in hours
        int repetitions; 										// # Repetitions of the behavior. -1 infinite.        
        string remoteServers = default("LOCAL");

        int vmsToRentTypesQuantity;

        bool printResults = default(true);

        string cell_type;

        @display("i=abstract/person;bgl=2");

    submodules:
        vmDefinition: VmDefinition {
            parameters:
                vmsToRentTypesQuantity = vmsToRentTypesQuantity;
                @display("p=57,171;is=vl");
        }

        distribution: DistributionDefinition {
            parameters:
                name = default("no_distribution");
                params = default("");
                @display("p=57,74");
        }

        appDefinition: AppDefinition {
            parameters:
                @display("p=226,74");
        }

        core: <cell_type> like IUserGenerator {
            parameters:
                name = name;
                behavior = behavior;
                quantity_user_preloaded = quantity_user_preloaded;							// Users preloaded at the system before the distribution begins
                total_users = total_users;										// Total users to be created per total time creation.
                time_creation = time_creation;									// Time creation in hours of users in the system in hours
                remoteServers = remoteServers;
                vmsToRentTypesQuantity = vmsToRentTypesQuantity;
                printResults = printResults;
                repetitions = repetitions;
                @display("p=226,171;is=vl");
        }

}
