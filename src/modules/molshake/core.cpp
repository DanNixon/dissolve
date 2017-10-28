/*
	*** MolShake Module - Core
	*** src/modules/molshake/core.cpp
	Copyright T. Youngs 2012-2017

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "modules/molshake/molshake.h"

// Static Members
List<Module> MolShakeModule::instances_;

/*
 * Constructor / Destructor
 */

// Constructor
MolShakeModule::MolShakeModule() : Module()
{
	// Add to instances list and set unique name for this instance
	uniqueName_.sprintf("%s%02i", name(), instances_.nItems());
	instances_.own(this);

	// Set up variables / control parameters
	setUpKeywords();
}

// Destructor
MolShakeModule::~MolShakeModule()
{
}

/*
 * Instances
 */

// Create instance of this module
List<Module>& MolShakeModule::instances()
{
	return instances_;
}

// Create instance of this module
Module* MolShakeModule::createInstance()
{
	return new MolShakeModule;
}
