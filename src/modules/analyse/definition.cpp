/*
	*** Analyse Module - Definition
	*** src/modules/analyse/definition.cpp
	Copyright T. Youngs 2012-2018

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "modules/analyse/analyse.h"

// Return name of module
const char* AnalyseModule::name()
{
	return "Analyse";
}

// Return brief description of module
const char* AnalyseModule::brief()
{
	return "Perform analysis of one or more Configurations";
}

// Return instance type for module
Module::InstanceType AnalyseModule::instanceType()
{
	return Module::MultipleInstance;
}

// Return the maximum number of Configurations the Module can target (or -1 for any number)
int AnalyseModule::nTargetableConfigurations()
{
	return -1;
}

// Modules upon which this Module depends to have run first
const char* AnalyseModule::dependentModules()
{
	return "";
}

// Set up supplied dependent module (only if it has been auto-added)
bool AnalyseModule::setUpDependentModule(Module* depMod)
{
	return true;
}