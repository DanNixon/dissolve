/*
	*** Calculate Average Molecule Module - Definition
	*** src/modules/calculate_avgmol/definition.cpp
	Copyright T. Youngs 2012-2020

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

#include "modules/calculate_avgmol/avgmol.h"

// Return type of module
const char* CalculateAvgMolModule::type() const
{
	return "CalculateAvgMol";
}

// Return category for module
const char* CalculateAvgMolModule::category() const
{
	return "Analysis / Geometry";
}

// Return brief description of module
const char* CalculateAvgMolModule::brief() const
{
	return "Calculate average atomic positions of a species around an oriented site";
}

// Return the number of Configuration targets this Module requires
int CalculateAvgMolModule::nRequiredTargets() const
{
	return Module::ExactlyOneTarget;
}
