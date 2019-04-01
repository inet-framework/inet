//
// Module that represents the quantity of storage nodes and how they will be named at data center structure..
// 
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11

package inet.icancloud.Management.Topology;

simple StorageNode
{
    parameters:
        @class(icancloud::StorageNode);
		int quantity;
		string id;
}
