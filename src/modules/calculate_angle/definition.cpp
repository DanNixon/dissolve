/*
	*** Calculate Angle Module - Definition
	*** src/modules/calculate_angle/definition.cpp
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

#include "modules/calculate_angle/angle.h"

// Return type of module
const char* CalculateAngleModule::type() const
{
	return "CalculateAngle";
}

// Return category for module
const char* CalculateAngleModule::category() const
{
	return "Analysis";
}

// Return brief description of module
const char* CalculateAngleModule::brief() const
{
	return "Calculate distance/angle maps";
}

// Return the number of Configuration targets this Module requires
int CalculateAngleModule::nRequiredTargets() const
{
	return Module::ExactlyOneTarget;
}
