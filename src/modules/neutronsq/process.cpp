/*
	*** Neutron SQ Module - Processing
	*** src/modules/neutronsq/process.cpp
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

#include "modules/neutronsq/neutronsq.h"
#include "main/dissolve.h"
#include "classes/atomtype.h"
#include "classes/box.h"
#include "classes/configuration.h"
#include "classes/species.h"
#include "classes/weights.h"
#include "math/filters.h"
#include "math/ft.h"
#include "io/export/data1d.h"
#include "modules/import/import.h"
#include "modules/bragg/bragg.h"
#include "modules/rdf/rdf.h"
#include "modules/sq/sq.h"
#include "genericitems/listhelper.h"

// Run set-up stage
bool NeutronSQModule::setUp(Dissolve& dissolve, ProcessPool& procPool)
{
	/*
	 * Load and set up reference data (if a file/format was given)
	 */
	if (referenceFQ_.hasValidFileAndFormat())
	{
		// Load the data
		Data1D referenceData;
		if (!referenceFQ_.importData(referenceData, &procPool))
		{
			Messenger::error("Failed to load reference data '%s'.\n", referenceFQ_.filename());
			return false;
		}

		// Truncate data beyond QMax
		const double qMax = keywords_.asDouble("QMax") < 0.0 ? 30.0 : keywords_.asDouble("QMax");
		if (referenceData.constXAxis().lastValue() < qMax) Messenger::warn("Qmax limit of %e Angstroms**-1 for calculated NeutronSQ (%s) is beyond limit of reference data (Qmax = %e Angstroms**-1).\n", qMax, uniqueName(), referenceData.constXAxis().lastValue());
		else while (referenceData.constXAxis().lastValue() > qMax) referenceData.removeLastPoint();

		// Remove first point?
		if (keywords_.asBool("ReferenceIgnoreFirst"))
		{
			referenceData.removeFirstPoint();
			Messenger::print("Removed first point from supplied reference data - new Qmin = %e Angstroms**-1.\n", referenceData.constXAxis().firstValue());
		}

		// Remove normalisation factor from data
		NeutronSQModule::NormalisationType normType = keywords_.enumeration<NeutronSQModule::NormalisationType>("ReferenceNormalisation");
		if (normType != NeutronSQModule::NoNormalisation)
		{
			// We need the summed Weights in order to do the normalisation
			Weights summedWeights;
			if (!calculateSummedWeights(summedWeights))
			{
				Messenger::error("Couldn't get summed Weights for reference data in NeutronSQ module '%s', and so can't perform requested normalisation.\n", uniqueName());
				return false;
			}

			// Remove normalisation of data
			if (normType == NeutronSQModule::AverageOfSquaresNormalisation)
			{
				referenceData.values() *= summedWeights.boundCoherentAverageOfSquares();
				Messenger::print("NeutronSQ: Removed <b>**2 normalisation from reference data ('%s'), factor = %f.\n", uniqueName(), summedWeights.boundCoherentAverageOfSquares());
			}
			else if (normType == NeutronSQModule::SquareOfAverageNormalisation)
			{
				referenceData.values() *= summedWeights.boundCoherentSquareOfAverage();
				Messenger::print("NeutronSQ: Removed <b**2> normalisation from reference data ('%s'), factor = %f.\n", uniqueName(), summedWeights.boundCoherentSquareOfAverage());
			}
		}

		// Get window function to use for transformation of S(Q) to g(r)
		const WindowFunction& referenceWindowFunction = keywords_.retrieve<WindowFunction>("ReferenceWindowFunction", WindowFunction());
		if (referenceWindowFunction.function() == WindowFunction::NoWindow) Messenger::print("No window function will be applied in Fourier transform of S(Q) to g(r).");
		else Messenger::print("Window function to be applied in Fourier transform of reference data is %s (%s).", WindowFunction::functionType(referenceWindowFunction.function()), referenceWindowFunction.parameterSummary().get());

		// Store the reference data in processing
		referenceData.setName(uniqueName());
		Data1D& storedData = GenericListHelper<Data1D>::realise(dissolve.processingModuleData(), "ReferenceData", uniqueName(), GenericItem::ProtectedFlag);
		storedData.setObjectTag(CharString("%s//ReferenceData", uniqueName()));
		storedData = referenceData;

		// Calculate and store the FT of the reference data in processing
		referenceData.setName(uniqueName());
		Data1D& storedDataFT = GenericListHelper<Data1D>::realise(dissolve.processingModuleData(), "ReferenceDataFT", uniqueName(), GenericItem::ProtectedFlag);
		storedDataFT.setObjectTag(CharString("%s//ReferenceDataFT", uniqueName()));
		storedDataFT = referenceData;
		double rho = nTargetConfigurations() == 0 ? 0.1 : RDFModule::summedRho(this, dissolve.processingModuleData());
		Fourier::sineFT(storedDataFT, 1.0 / (2.0 * PI * PI * rho), 0.0, 0.05, 30.0, referenceWindowFunction);
		if (nTargetConfigurations() == 0) Messenger::warn("No configurations associated to module, so Fourier transform of reference data will use assumed atomic density of 0.1.\n");

		// Save data?
		if (keywords_.asBool("SaveReference"))
		{
			if (procPool.isMaster())
			{
				Data1DExportFileFormat exportFormat(CharString("%s-ReferenceData.q", uniqueName()));
				if (!exportFormat.exportData(storedData)) return procPool.decideFalse();
				Data1DExportFileFormat exportFormatFT(CharString("%s-ReferenceData.r", uniqueName()));
				if (!exportFormatFT.exportData(storedDataFT)) return procPool.decideFalse();
				procPool.decideTrue();
			}
			else if (!procPool.decision()) return false;
		}
	}

	return true;
}

// Run main processing
bool NeutronSQModule::process(Dissolve& dissolve, ProcessPool& procPool)
{
	/*
	 * Calculate weighted or unweighted partials and total g(r)/G(r) or S(Q)/F(Q)
	 * 
	 * This is a serial routine, with each process constructing its own copy of the data.
	 * Partial calculation routines called by this routine are parallel.
	 */

	// Check for zero Configuration targets
	if (targetConfigurations_.nItems() == 0) return Messenger::error("No configuration targets set for module '%s'.\n", uniqueName());

	CharString varName;

	const bool includeBragg = keywords_.asBool("IncludeBragg");
	const BroadeningFunction& braggQBroadening = keywords_.retrieve<BroadeningFunction>("BraggQBroadening", BroadeningFunction());
	NeutronSQModule::NormalisationType normalisation = keywords_.enumeration<NeutronSQModule::NormalisationType>("Normalisation");
	const BroadeningFunction& qBroadening = keywords_.retrieve<BroadeningFunction>("QBroadening", BroadeningFunction());
	const double qDelta = keywords_.asDouble("QDelta");
	const double qMin = keywords_.asDouble("QMin");
	double qMax = keywords_.asDouble("QMax");
	if (qMax < 0.0) qMax = 30.0;
	const bool saveUnweighted = keywords_.asBool("SaveUnweighted");
	const bool saveWeighted = keywords_.asBool("SaveWeighted");
	const WindowFunction& windowFunction = keywords_.retrieve<WindowFunction>("WindowFunction", WindowFunction());

	// Print argument/parameter summary
	Messenger::print("NeutronSQ: Calculating S(Q)/F(Q) over %f < Q < %f Angstroms**-1 using step size of %f Angstroms**-1.\n", qMin, qMax, qDelta);
	if (windowFunction.function() == WindowFunction::NoWindow) Messenger::print("NeutronSQ: No window function will be applied in Fourier transforms of g(r) to S(Q).");
	else Messenger::print("NeutronSQ: Window function to be applied in Fourier transforms is %s (%s).", WindowFunction::functionType(windowFunction.function()), windowFunction.parameterSummary().get());
	const WindowFunction& referenceWindowFunction = keywords_.retrieve<WindowFunction>("ReferenceWindowFunction", WindowFunction());
	if (referenceWindowFunction.function() == WindowFunction::NoWindow) Messenger::print("No window function will be applied when calculating representative g(r) from S(Q).");
	else Messenger::print("Window function to be applied when calculating representative g(r) from S(Q) is %s (%s).", WindowFunction::functionType(referenceWindowFunction.function()), referenceWindowFunction.parameterSummary().get());
	if (normalisation == NeutronSQModule::NoNormalisation) Messenger::print("NeutronSQ: No normalisation will be applied to total F(Q).\n");
	else if (normalisation == NeutronSQModule::AverageOfSquaresNormalisation) Messenger::print("NeutronSQ: Total F(Q) will be normalised to <b>**2");
	else if (normalisation == NeutronSQModule::SquareOfAverageNormalisation) Messenger::print("NeutronSQ: Total F(Q) will be normalised to <b**2>");
	if (qBroadening.function() == BroadeningFunction::NoFunction) Messenger::print("NeutronSQ: No broadening will be applied to calculated S(Q).");
	else Messenger::print("NeutronSQ: Broadening to be applied in calculated S(Q) is %s (%s).", BroadeningFunction::functionType(qBroadening.function()), qBroadening.parameterSummary().get());
	if (saveUnweighted) Messenger::print("NeutronSQ: Unweighted partials and totals will be saved.\n");
	if (saveWeighted) Messenger::print("NeutronSQ: Weighted partials and totals will be saved.\n");
	if (includeBragg)
	{
		Messenger::print("NeutronSQ: Bragg scattering will be calculated from reflection data in target Configurations, if present.\n");
		if (braggQBroadening.function() == BroadeningFunction::NoFunction) Messenger::print("NeutronSQ: No additional broadening will be applied to calculated Bragg S(Q).");
		else Messenger::print("NeutronSQ: Additional broadening to be applied in calculated Bragg S(Q) is %s (%s).", BroadeningFunction::functionType(braggQBroadening.function()), braggQBroadening.parameterSummary().get());
	}
	Messenger::print("\n");


	/*
	 * Loop over target Configurations and Fourier transform their UnweightedGR into the corresponding UnweightedSQ.
	 */

	bool created;

	RefListIterator<Configuration> configIterator(targetConfigurations_);
	while (Configuration* cfg = configIterator.iterate())
	{
		// Set up process pool - must do this to ensure we are using all available processes
		procPool.assignProcessesToGroups(cfg->processPool());

		// Get unweighted g(r) for this Configuration - we don't supply a specific Module prefix, since the unweighted g(r) may come from one of many RDF-type modules
		if (!cfg->moduleData().contains("UnweightedGR")) return Messenger::error("Couldn't locate UnweightedGR for Configuration '%s'.\n", cfg->name());
		const PartialSet& unweightedgr = GenericListHelper<PartialSet>::value(cfg->moduleData(), "UnweightedGR");

		// Does a PartialSet for the unweighted S(Q) already exist for this Configuration?
		PartialSet& unweightedsq = GenericListHelper<PartialSet>::realise(cfg->moduleData(), "UnweightedSQ", NULL, GenericItem::InRestartFileFlag, &created);
		if (created) unweightedsq.setUpPartials(unweightedgr.atomTypes(), CharString("%s-%s", cfg->niceName(), uniqueName()), "unweighted", "sq", "Q, 1/Angstroms");

		// Is the PartialSet already up-to-date? Do we force its calculation anyway?
		bool& forceCalculation = GenericListHelper<bool>::retrieve(cfg->moduleData(), "_ForceNeutronSQ", NULL, false);
		const bool sqUpToDate = DissolveSys::sameString(unweightedsq.fingerprint(), CharString("%i/%i", cfg->moduleData().version("UnweightedGR"), includeBragg ? cfg->moduleData().version("BraggReflections") : -1));
		if ((!forceCalculation) && sqUpToDate)
		{
			Messenger::print("Unweighted partial S(Q) are up-to-date for Configuration '%s'.\n", cfg->name());
			continue;
		}
		forceCalculation = false;

		// Transform g(r) into S(Q)
		if (!SQModule::calculateUnweightedSQ(procPool, cfg, unweightedgr, unweightedsq, qMin, qDelta, qMax, cfg->atomicDensity(), windowFunction, qBroadening)) return false;

		// Include Bragg scattering?
		if (includeBragg)
		{
			// Check if reflection data is present
			if (!cfg->moduleData().contains("BraggReflections")) return Messenger::error("Bragg scattering requested to be included, but Configuration '%s' contains no reflection data.\n", cfg->name());
			const Array<BraggReflection>& braggReflections = GenericListHelper< Array<BraggReflection> >::value(cfg->moduleData(), "BraggReflections", "", Array<BraggReflection>());
			const int nReflections = braggReflections.nItems();
			const double braggQMax = braggReflections.constAt(nReflections-1).q();
			Messenger::print("Found BraggReflections data for Configuration '%s' (nReflections = %i, QMax = %f Angstroms**-1).\n", cfg->name(), nReflections, braggQMax);

			// Create a temporary array into which our broadened Bragg partials will be placed
			Array2D< Data1D >& braggPartials = GenericListHelper< Array2D< Data1D > >::realise(cfg->moduleData(), "BraggPartials", uniqueName(), GenericItem::NoFlag, &created);
			if (created)
			{
				// Initialise the array
				braggPartials.initialise(unweightedsq.nAtomTypes(), unweightedsq.nAtomTypes(), true);

				for (int i=0; i<unweightedsq.nAtomTypes(); ++i)
				{
					for (int j=i; j<unweightedsq.nAtomTypes(); ++j) braggPartials.at(i,j) = unweightedsq.constPartial(0,0);
				}
			}
			for (int i=0; i<unweightedsq.nAtomTypes(); ++i)
			{
				for (int j=i; j<unweightedsq.nAtomTypes(); ++j) braggPartials.at(i,j).values() = 0.0;
			}

			// First, re-bin the reflection data into the arrays we have just set up
			if (!BraggModule::reBinReflections(procPool, cfg, braggPartials)) return false;

			// Apply necessary broadening
			for (int i=0; i<unweightedsq.nAtomTypes(); ++i)
			{
				for (int j=i; j<unweightedsq.nAtomTypes(); ++j)
				{
					// Bragg-specific broadening
					Filters::convolve(braggPartials.at(i,j), braggQBroadening, true);

					// Local 'QBroadening' term
					Filters::convolve(braggPartials.at(i,j), qBroadening, true);
				}
			}

			// Remove self-scattering level from partials between the same atom type and remove normalisation from atomic fractions
			for (int i=0; i<unweightedsq.nAtomTypes(); ++i)
			{
				for (int j=i; j<unweightedsq.nAtomTypes(); ++j)
				{
					// Subtract self-scattering level if types are equivalent
					if (i == j) braggPartials.at(i,i) -= cfg->usedAtomTypeData(i)->fraction();

					// Remove atomic fraction normalisation
					braggPartials.at(i,j) /= cfg->usedAtomTypeData(i)->fraction()*cfg->usedAtomTypeData(j)->fraction();
				}
			}

			// Blend the bound/unbound and Bragg partials at the higher Q limit
			for (int i=0; i<unweightedsq.nAtomTypes(); ++i)
			{
				for (int j=i; j<unweightedsq.nAtomTypes(); ++j)
				{
					// Note: Intramolecular broadening will not be applied to bound terms within the calculated Bragg scattering
					Data1D& bound = unweightedsq.boundPartial(i,j);
					Data1D& unbound = unweightedsq.unboundPartial(i,j);
					Data1D& partial = unweightedsq.partial(i,j);
					Data1D& bragg = braggPartials.at(i,j);

					for (int n=0; n<bound.nValues(); ++n)
					{
						const double q = bound.xAxis(n);
						if (q <= braggQMax)
						{
							bound.value(n) = 0.0;
							unbound.value(n) = bragg.value(n);
							partial.value(n) = bragg.value(n);
						}
					}
				}
			}

			// Re-form the total function
			unweightedsq.formTotal(true);
		}

		// Set names of resources (Data1D) within the PartialSet, and tag it with the fingerprint from the source unweighted g(r)
		unweightedsq.setObjectTags(CharString("%s//%s//%s", cfg->niceName(), "NeutronSQ", "UnweightedSQ"));
		unweightedsq.setFingerprint(CharString("%i/%i", cfg->moduleData().version("UnweightedGR"), includeBragg ? cfg->moduleData().version("BraggReflections") : -1));

		// Save data if requested
		if (saveUnweighted && (!MPIRunMaster(procPool, unweightedsq.save()))) return false;

		// Construct weights matrix based on Isotopologue specifications and the populations of AtomTypes in the Configuration
		Weights weights;
		if (isotopologues_.contains(cfg))
		{
			// Get the set...
			const IsotopologueSet* topeSet = isotopologues_.isotopologueSet(cfg);

			// Iterate over Species present in the set
			ListIterator<Isotopologues> topeIterator(topeSet->isotopologues());
			while (Isotopologues* topes = topeIterator.iterate())
			{
				// Find the referenced Species in our SpeciesInfo list
				SpeciesInfo* spInfo = cfg->usedSpeciesInfo(topes->species());
				if (!spInfo) return Messenger::error("Couldn't locate SpeciesInfo for '%s' in the Configuration '%s'.\n", topes->species()->name(), cfg->niceName());

				// Add defined isotopologues, in the relative isotopic proportions defined, to the weights.
				ListIterator<IsotopologueWeight> weightIterator(topes->mix());
				while (IsotopologueWeight* isoWeight = weightIterator.iterate()) weights.addIsotopologue(spInfo->species(), spInfo->population(), isoWeight->isotopologue(), isoWeight->weight());
			}
		}

		// We will automatically use the natural isotopologue for a species if it is not explicitly covered by at least one Isotopologue definition
		ListIterator<SpeciesInfo> speciesInfoIterator(cfg->usedSpecies());
		while (SpeciesInfo* spInfo = speciesInfoIterator.iterate()) if (!weights.hasIsotopologues(spInfo->species()))
		{
			Messenger::print("Isotopologue specification for Species '%s' in Configuration '%s' is missing - natural isotopologue will be used.\n", spInfo->species()->name(), cfg->name());

			Species* sp = spInfo->species();
			weights.addIsotopologue(sp, spInfo->population(), sp->naturalIsotopologue(), 1.0);
		}

		// Create, print, and store weights
		Messenger::print("Isotopologue and isotope composition for Configuration '%s':\n\n", cfg->name());
		weights.createFromIsotopologues(exchangeableTypes_);
		weights.print();

		// Does a PartialSet for the unweighted S(Q) already exist for this Configuration?
		PartialSet& weightedsq = GenericListHelper<PartialSet>::realise(cfg->moduleData(), "WeightedSQ", NULL, GenericItem::InRestartFileFlag, &created);
		if (created) weightedsq.setUpPartials(unweightedsq.atomTypes(), CharString("%s-%s", cfg->niceName(), uniqueName()), "weighted", "sq", "Q, 1/Angstroms");

		// Calculate weighted S(Q)
		calculateWeightedSQ(unweightedsq, weightedsq, weights, normalisation);

		// Set names of resources (Data1D) within the PartialSet
		weightedsq.setObjectTags(CharString("%s//%s//%s", cfg->niceName(), uniqueName_.get(), "WeightedSQ"));

		// Save data if requested
		if (saveWeighted && (!MPIRunMaster(procPool, weightedsq.save()))) return false;
	}

	/*
	 * Construct the weighted sum of Configuration partials and store in the processing module data list
	 */

	// Create/retrieve PartialSet for summed partial S(Q)
	PartialSet& summedUnweightedSQ = GenericListHelper<PartialSet>::realise(dissolve.processingModuleData(), "UnweightedSQ", uniqueName_, GenericItem::InRestartFileFlag);

	// Sum the partials from the associated Configurations
	if (!SQModule::sumUnweightedSQ(procPool, this, dissolve.processingModuleData(), summedUnweightedSQ)) return false;

	// Save data if requested
	if (saveUnweighted && (!MPIRunMaster(procPool, summedUnweightedSQ.save()))) return false;

	// Create/retrieve PartialSet for summed partial S(Q)
	PartialSet& summedWeightedSQ = GenericListHelper<PartialSet>::realise(dissolve.processingModuleData(), "WeightedSQ", uniqueName_, GenericItem::InRestartFileFlag);
	summedWeightedSQ = summedUnweightedSQ;
	summedWeightedSQ.setFileNames(uniqueName_, "weighted", "sq");
	summedWeightedSQ.setObjectTags(CharString("%s//%s", uniqueName_.get(), "WeightedSQ"));

	// Calculate weighted S(Q)
	Messenger::print("Isotopologue and isotope composition over all Configurations used in '%s':\n\n", uniqueName_.get());
	Weights summedWeights;
	if (!calculateSummedWeights(summedWeights)) return false;
	summedWeights.print();
	GenericListHelper<Weights>::realise(dissolve.processingModuleData(), "FullWeights", uniqueName_, GenericItem::InRestartFileFlag) = summedWeights;
	calculateWeightedSQ(summedUnweightedSQ, summedWeightedSQ, summedWeights, normalisation);

	// Create/retrieve PartialSet for summed unweighted g(r)
	PartialSet& summedUnweightedGR = GenericListHelper<PartialSet>::realise(dissolve.processingModuleData(), "UnweightedGR", uniqueName_, GenericItem::InRestartFileFlag);

	// Sum the partials from the associated Configurations
	if (!RDFModule::sumUnweightedGR(procPool, this, dissolve.processingModuleData(), summedUnweightedGR)) return false;

	// Create/retrieve PartialSet for summed weighted g(r)
	PartialSet& summedWeightedGR = GenericListHelper<PartialSet>::realise(dissolve.processingModuleData(), "WeightedGR", uniqueName_, GenericItem::InRestartFileFlag, &created);
	if (created) summedWeightedGR.setUpPartials(summedUnweightedSQ.atomTypes(), uniqueName_, "weighted", "gr", "r, Angstroms");
	summedWeightedGR.setObjectTags(CharString("%s//%s", uniqueName_.get(), "WeightedGR"));

	// Calculate weighted g(r)
	calculateWeightedGR(summedUnweightedGR, summedWeightedGR, summedWeights, normalisation);

	// Calculate representative total g(r) from FT of calculated S(Q)
	Data1D& repGR = GenericListHelper<Data1D>::realise(dissolve.processingModuleData(), "RepresentativeTotalGR", uniqueName_, GenericItem::InRestartFileFlag);
	repGR = summedWeightedSQ.total();
	double rho = nTargetConfigurations() == 0 ? 0.1 : RDFModule::summedRho(this, dissolve.processingModuleData());
	Fourier::sineFT(repGR, 1.0 / (2.0 * PI * PI * rho), qMin, qDelta, qMax, referenceWindowFunction);
	repGR.setObjectTag(CharString("%s//RepresentativeTotalGR", uniqueName_.get()));

	// Save data if requested
	if (saveWeighted && (!MPIRunMaster(procPool, summedWeightedSQ.save()))) return false;

	return true;
}
