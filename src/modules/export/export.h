/*
	*** Export Module
	*** src/modules/export/export.h
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

#ifndef DISSOLVE_MODULE_EXPORT_H
#define DISSOLVE_MODULE_EXPORT_H

#include "module/module.h"
#include "io/export/coordinates.h"
#include "io/export/pairpotential.h"
#include "io/export/trajectory.h"

// Forward Declarations
/* none */

// Export Module
class ExportModule : public Module
{
	/*
	 * Export data in various formats
	 */

	public:
	// Constructor
	ExportModule();
	// Destructor
	~ExportModule();


	/*
	 * Instances
	 */
	public:
	// Create instance of this module
	Module* createInstance() const;


	/*
	 * Definition
	 */
	public:
	// Return type of module
	const char* type() const;
	// Return category for module
	const char* category() const;
	// Return brief description of module
	const char* brief() const;
	// Return the number of Configuration targets this Module requires
	int nRequiredTargets() const;


	/*
	 * Initialisation
	 */
	protected:
	// Perform any necessary initialisation for the Module
	void initialise();


	/*
	 * Data
	 */
	private:
	// Filename and format for coordinate export
	CoordinateExportFileFormat coordinatesFormat_;
	// Basename and format for PairPotential export
	PairPotentialExportFileFormat pairPotentialFormat_;
	// Filename and format for trajectory export
	TrajectoryExportFileFormat trajectoryFormat_;


	/*
	 * Processing
	 */
	private:
	// Run main processing
	bool process(Dissolve& dissolve, ProcessPool& procPool);
};

#endif

