/*
	*** AtomShake Module - Definition
	*** src/modules/atomshake/definition.cpp
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

#include "modules/atomshake/atomshake.h"

// Return name of module
const char* AtomShakeModule::name()
{
	return "AtomShake";
}

// Return brief description of module
const char* AtomShakeModule::brief()
{
	return "Perform atomic Monte Carlo on all atoms";
}

// Return instance type for module
Module::InstanceType AtomShakeModule::instanceType()
{
	return Module::MultipleInstance;
}

// Return the maximum number of Configurations the Module can target (or -1 for any number)
int AtomShakeModule::nTargetableConfigurations()
{
	return 1;
}

// Whether the Module has a pre-processing stage
bool AtomShakeModule::hasPreProcessing()
{
	return false;
}

// Whether the Module has a processing stage
bool AtomShakeModule::hasProcessing()
{
	return true;
}

// Whether the Module has a post-processing stage
bool AtomShakeModule::hasPostProcessing()
{
	return false;
}

// Modules upon which this Module depends to have run first
const char* AtomShakeModule::dependentModules()
{
	return "";
}

// Set up supplied dependent module (only if it has been auto-added)
bool AtomShakeModule::setUpDependentModule(Module* depMod)
{
	return true;
}
