// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.



//
// Simulates processor usage of an IPNode.
//
// Relies on direct send in and out. 
//
// Set numOfProcessors=1 for single processor node, and 
// numOfProcessors=0 to disable processor check.
//
// author: Jochen Reber
// date: 16.6.00
//
simple ProcessorManager
    parameters:
        numOfProcessors: const;
    gates:
        in: ClaimProcessorIn, ClaimKernelIn,
            ReleaseProcessorIn, ReleaseKernelIn;
endsimple

