/*
	*** SQ Module
	*** src/modules/sq/sq.h
	Copyright T. Youngs 2012-2018

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

#ifndef DUQ_SQMODULE_H
#define DUQ_SQMODULE_H

#include "module/module.h"
#include "classes/partialset.h"
#include "classes/isotopologuereference.h"
#include "classes/braggpeak.h"

// Forward Declarations
class PartialSet;
class Weights;

// SQ Module
class SQModule : public Module
{
	public:
	// Constructor
	SQModule();
	// Destructor
	~SQModule();


	/*
	 * Instances
	 */
	protected:
	// List of all instances of this Module type
	static List<Module> instances_;

	public:
	// Return list of all created instances of this Module
	List<Module>& instances();
	// Create instance of this module
	Module* createInstance();


	/*
	 * Definition
	 */
	public:
	// Return name of module
	const char* name();
	// Return brief description of module
	const char* brief();
	// Return instance type for module
	InstanceType instanceType();
	// Return the maximum number of Configurations the Module can target (or -1 for any number)
	int nTargetableConfigurations();
	// Modules upon which this Module depends to have run first
	const char* dependentModules();
	// Set up supplied dependent module (only if it has been auto-added)
	bool setUpDependentModule(Module* depMod);


	/*
	 * Options
	 */
	protected:
	// Set up keywords for Module
	void setUpKeywords();
	// Parse complex keyword line, returning true (1) on success, false (0) for recognised but failed, and -1 for not recognised
	int parseComplexKeyword(ModuleKeywordBase* keyword, LineParser& parser, DUQ* duq, GenericList& targetList, const char* prefix);


	/*
	 * Processing
	 */
	private:
	// Run main processing
	bool process(DUQ& duq, ProcessPool& procPool);

	public:
	// Whether the Module has a processing stage
	bool hasProcessing();


	/*
	 * Members / Functions
	 */
	public:
	// Calculate unweighted S(Q) from unweighted g(r)
	static bool calculateUnweightedSQ(ProcessPool& procPool, Configuration* cfg, PartialSet& unweightedgr, PartialSet& unweightedsq, double qMin, double qDelta, double qMax, double rho, const WindowFunction& windowFunction, const BroadeningFunction& broadening);
	// Sum unweighted S(Q) over supplied target Configurations
	static bool sumUnweightedSQ(ProcessPool& procPool, Module* module, GenericList& moduleData, PartialSet& summedsq);


	/*
	 * GUI Widget
	 */
	public:
	// Return a new widget controlling this Module
	ModuleWidget* createWidget(QWidget* parent, DUQ& dUQ);
};

#endif
