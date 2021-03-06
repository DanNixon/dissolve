/*
	*** Bragg Module - Processing
	*** src/modules/bragg/process.cpp
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

#include "modules/bragg/bragg.h"
#include "main/dissolve.h"
#include "math/averaging.h"
#include "classes/box.h"
#include "classes/configuration.h"
#include "classes/species.h"
#include "classes/weights.h"
#include "genericitems/listhelper.h"

// Run main processing
bool BraggModule::process(Dissolve& dissolve, ProcessPool& procPool)
{
	/*
	 * Calculate Bragg contributions.
	 * 
	 * This is a serial routine, with each process constructing its own copy of the data.
	 * Partial calculation routines called by this routine are parallel.
	 */

	// Check for zero Configuration targets
	if (targetConfigurations_.nItems() == 0) return Messenger::error("No configuration targets set for module '%s'.\n", uniqueName());

	const int averaging = keywords_.asInt("Averaging");
	if (!Averaging::averagingSchemes().isValid(keywords_.asString("AveragingScheme"))) return Averaging::averagingSchemes().errorAndPrintValid(keywords_.asString("AveragingScheme"));
	Averaging::AveragingScheme averagingScheme = Averaging::averagingSchemes().enumeration(keywords_.asString("AveragingScheme"));
	const double qDelta = keywords_.asDouble("QDelta");
	const double qMax = keywords_.asDouble("QMax");
	const double qMin = keywords_.asDouble("QMin");
	const Vec3<int> multiplicity = keywords_.asVec3Int("Multiplicity");
	// TODO Could look for this value in the Configuration's module data (could be set/stored if a known crystal repeat was used in the generation of the Configuration).
	const bool saveReflections = keywords_.asBool("SaveReflections");

	// Print argument/parameter summary
	Messenger::print("Bragg: Calculating Bragg S(Q) over %f < Q < %f Angstroms**-1 using bin size of %f Angstroms**-1.\n", qMin, qMax, qDelta);
	Messenger::print("Bragg: Multiplicity is (%i %i %i).\n", multiplicity.x, multiplicity.y, multiplicity.z);
	if (averaging <= 1) Messenger::print("Bragg: No averaging of reflections will be performed.\n");
	else Messenger::print("Bragg: Reflections will be averaged over %i sets (scheme = %s).\n", averaging, Averaging::averagingSchemes().keyword(averagingScheme));
	Messenger::print("\n");

	/*
	 * Regardless of whether we are a main processing task (summing some combination of Configuration's partials) or multiple independent Configurations,
	 * we must loop over the specified targetConfigurations_ and calculate the partials for each.
	 */
	RefListIterator<Configuration> configIterator(targetConfigurations_);
	while (Configuration* cfg = configIterator.iterate())
	{
		// Set up process pool - must do this to ensure we are using all available processes
		procPool.assignProcessesToGroups(cfg->processPool());

		// Calculate Bragg vectors and intensities for the current Configuration
		bool alreadyUpToDate;
		if (!calculateBraggTerms(procPool, cfg, qMin, qDelta, qMax, multiplicity, alreadyUpToDate)) return false;

		// If we are already up-to-date, then theres nothing more to do for this Configuration
		if (alreadyUpToDate)
		{
			Messenger::print("Bragg data is up-to-date for Configuration '%s'.\n", cfg->name());
			continue;
		}

		// Perform averaging of reflections data if requested
		if (averaging > 1)
		{
			Averaging::arrayAverage< Array<BraggReflection> >(cfg->moduleData(), "BraggReflections", "", averaging, averagingScheme);
		}

		// Form partial and total reflection functions
		formReflectionFunctions(procPool, cfg, qMin, qDelta, qMax);

		// Save reflection data?
		if (saveReflections)
		{
			// Retrieve BraggReflection data from the Configuration's module data
			bool found = false;
			const Array<BraggReflection>& braggReflections = GenericListHelper< Array<BraggReflection> >::value(cfg->moduleData(), "BraggReflections", "", Array<BraggReflection>(), &found);
			if (!found) return Messenger::error("Failed to find BraggReflection array in module data for Configuration '%s'.\n", cfg->name());

			// Open a file and save the basic reflection data
			LineParser braggParser(&procPool);
			if (!braggParser.openOutput(CharString("%s-Bragg.txt", cfg->niceName()))) return false;
			braggParser.writeLineF("#   ID      Q       nKVecs    Intensity(0,0)\n");
			for (int n = 0; n<braggReflections.nItems(); ++n)
			{
				if (!braggParser.writeLineF("%6i  %10.6f  %8i  %10.6e\n", n, braggReflections.constAt(n).q(), braggReflections.constAt(n).nKVectors(), braggReflections.constAt(n).intensity(0, 0))) return false;
			}
			braggParser.closeFiles();

			// Save intensity data
			int i = 0;
			for (AtomTypeData* atd1 = cfg->usedAtomTypes(); atd1 != NULL; atd1 = atd1->next(), ++i)
			{
				int j = i;
				for (AtomTypeData* atd2 = atd1; atd2 != NULL; atd2 = atd2->next(), ++j)
				{
					LineParser intensityParser(&procPool);
					if (!intensityParser.openOutput(CharString("%s-Bragg-%s-%s.txt", cfg->niceName(), atd1->atomTypeName(), atd2->atomTypeName()))) return false;
					intensityParser.writeLineF("#     Q      Intensity(%s,%s)\n", atd1->atomTypeName(), atd2->atomTypeName());
					for (int n = 0; n<braggReflections.nItems(); ++n)
					{
						if (!intensityParser.writeLineF("%10.6f  %10.6e\n", braggReflections.constAt(n).q(), braggReflections.constAt(n).intensity(i, j))) return false;
					}
					intensityParser.closeFiles();
				}
			}
		}
	}

	return true;
}
