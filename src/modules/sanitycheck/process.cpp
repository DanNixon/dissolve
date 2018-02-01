/*
	*** SanityCheck Module - Processing
	*** src/modules/sanitycheck/process.cpp
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

#include "modules/sanitycheck/sanitycheck.h"
#include "main/duq.h"
#include "base/sysfunc.h"

// Run pre-processing stage
bool SanityCheckModule::preProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}

// Run main processing
bool SanityCheckModule::process(DUQ& duq, ProcessPool& procPool)
{
	/*
	 * This is a parallel routine.
	 * As much data as possible is checked over all processes, checking that the "everybody knows everything" ideal is true.
	 */

	// Basic checks on data from dUQ
	int i = 0;
	for (AtomType* at1 = duq.atomTypeList().first(); at1 != NULL; at1 = at1->next, ++i)
	{
		int j = i;
		for (AtomType* at2 = at1; at2 != NULL; at2 = at2->next, ++j)
		{
			PairPotential* pp = duq.pairPotential(at1, at2);
			if (!pp)
			{
				Messenger::error("Failed to find PairPotential for AtomTypes '%s' and '%s'.\n", at1->name(), at2->name());
				return false;
			}

			// Check for equality
			if (!pp->equality(procPool)) return Messenger::error("Sanity check failed at PairPotential %s-%s.\n", at1->name(), at2->name());
		}
	}

	// Loop over all Configurations
	ListIterator<Configuration> configIterator(duq.configurations());
	while (Configuration* cfg = configIterator.iterate())
	{
		// TODO This is the most basic sanity checking we can do. Need to extend to bonds, angles, molecules etc.
		// Number of Atoms and atomic positions
		if (!procPool.equality(cfg->nAtoms())) return Messenger::error("Failed sanity check for Configuration '%s' nAtoms (%i).\n", cfg->name(), cfg->nAtoms());
		for (int n=0; n<cfg->nAtoms(); ++n)
		{
			Vec3<double> r = cfg->atom(n)->r();
			if (!procPool.equality(cfg->atom(n)->r())) return Messenger::error("Failed sanity check for Configuration '%s' atom position %i (%f %f %f).\n", cfg->name(), n, r.x, r.y, r.z);
		}

		// Module data within the Configuration
		if (!cfg->moduleData().equality(procPool)) return Messenger::error("Failed sanity check for Configuration '%s' module data.\n", cfg->name());
	}

	return true;
}

// Run post-processing stage
bool SanityCheckModule::postProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}