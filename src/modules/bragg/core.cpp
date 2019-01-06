/*
	*** Bragg Module - Core Functions
	*** src/modules/bragg/core.cpp
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

#include "modules/bragg/bragg.h"

/*
 * Constructor / Destructor
 */

// Constructor
BraggModule::BraggModule() : Module()
{
	// Set unique name for this instance of the Module
	static int instanceId = 0;
	uniqueName_.sprintf("%s%02i", type(), instanceId++);

	// Set up variables / control parameters
	setUpKeywords();

	// Set representative colour
	colour_[0] = 0;
	colour_[1] = 200;
	colour_[2] = 0;
}

// Destructor
BraggModule::~BraggModule()
{
}

/*
 * Instances
 */

// Create instance of this module
Module* BraggModule::createInstance() const
{
	return new BraggModule;
}

