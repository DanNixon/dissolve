/*
	*** Calculate Axis Angle Module
	*** src/modules/calculate_axisangle/axisangle.h
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

#ifndef DISSOLVE_MODULE_CALCULATEAXISANGLE_H
#define DISSOLVE_MODULE_CALCULATEAXISANGLE_H

#include "module/module.h"
#include "procedure/procedure.h"

// Forward Declarations
class Collect1DProcedureNode;
class Collect2DProcedureNode;
class Process1DProcedureNode;
class Process2DProcedureNode;
class SelectProcedureNode;
class SpeciesSite;

// Calculate Axis Angle Module
class CalculateAxisAngleModule : public Module
{
	public:
	// Constructor
	CalculateAxisAngleModule();
	// Destructor
	~CalculateAxisAngleModule();


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
	 * Processing
	 */
	private:
	// Run set-up stage
	bool setUp(Dissolve& dissolve, ProcessPool& procPool);
	// Run main processing
	bool process(Dissolve& dissolve, ProcessPool& procPool);


	/*
	 * Functions / Data
	 */
	private:
	// Analysis procedure to be run
	Procedure analyser_;
	// SelectNode for site A
	SelectProcedureNode* selectA_;
	// SelectNode for site B
	SelectProcedureNode* selectB_;
	// Collect1DNode for A-B RDF
	Collect1DProcedureNode* collectDistance_;
	// Collect1DNode for A-B angle histogram
	Collect1DProcedureNode* collectAngle_;
	// Collect2DNode for distance-angle data
	Collect2DProcedureNode* collectDAngle_;
	// Process1DNode for A-B RDF
	Process1DProcedureNode* processDistance_;
	// Process1DNode for A-B-C angle histogram
	Process1DProcedureNode* processAngle_;
	// Process2DNode for distance-angle data
	Process2DProcedureNode* processDAngle_;


	/*
	 * GUI Widget
	 */
	public:
	// Return a new widget controlling this Module
	ModuleWidget* createWidget(QWidget* parent, Dissolve& dissolve);
};

#endif

