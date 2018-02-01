/*
	*** IntraShake Module - Processing
	*** src/modules/intrashake/process.cpp
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

#include "modules/intrashake/intrashake.h"
#include "main/duq.h"
#include "classes/changestore.h"
#include "classes/energykernel.h"
#include "classes/box.h"
#include "base/sysfunc.h"
#include "templates/genericlisthelper.h"

// Run pre-processing stage
bool IntraShakeModule::preProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}

// Run main processing
bool IntraShakeModule::process(DUQ& duq, ProcessPool& procPool)
{
	/*
	 * Perform intramolecular shakes, modifying individual bound terms on molecules.
	 * 
	 * This is a parallel routine, with processes operating as one pool
	 */

	// Check for zero Configuration targets
	if (targetConfigurations_.nItems() == 0)
	{
		Messenger::warn("No Configuration targets for Module.\n");
		return true;
	}

	// Loop over target Configurations
	for (RefListItem<Configuration,bool>* ri = targetConfigurations_.first(); ri != NULL; ri = ri->next)
	{
		// Grab Configuration pointer
		Configuration* cfg = ri->item;

		// Set up process pool - must do this to ensure we are using all available processes
		procPool.assignProcessesToGroups(cfg->processPool());

		// Get reference to relevant module data
		GenericList& moduleData = configurationLocal_ ? cfg->moduleData() : duq.processingModuleData();

		// Retrieve control parameters from Configuration
		bool adjustAngles = keywords_.asBool("AdjustAngles");
		bool adjustBonds = keywords_.asBool("AdjustBonds");
		bool adjustTorsions = keywords_.asBool("AdjustTorsions");
		double angleStepSize = GenericListHelper<double>::retrieve(moduleData, "AngleStepSize", uniqueName(), keywords_.asDouble("AngleStepSize"));
		const double angleStepSizeMax = keywords_.asDouble("AngleStepSizeMax");
		const double angleStepSizeMin = keywords_.asDouble("AngleStepSizeMin");
		double bondStepSize = GenericListHelper<double>::retrieve(moduleData, "BondStepSize", uniqueName(), keywords_.asDouble("BondStepSize"));
		const double bondStepSizeMax = keywords_.asDouble("BondStepSizeMax");
		const double bondStepSizeMin = keywords_.asDouble("BondStepSizeMin");
		double cutoffDistance = keywords_.asDouble("CutoffDistance");
		if (cutoffDistance < 0.0) cutoffDistance = duq.pairPotentialRange();
		const int nShakesPerTerm = keywords_.asInt("ShakesPerTerm");
		const double targetAcceptanceRate = keywords_.asDouble("TargetAcceptanceRate");
		const bool termEnergyOnly = keywords_.asBool("TermEnergyOnly");
		double torsionStepSize = GenericListHelper<double>::retrieve(moduleData, "TorsionStepSize", uniqueName(), keywords_.asDouble("TorsionStepSize"));
		const double torsionStepSizeMax = keywords_.asDouble("TorsionStepSizeMax");
		const double torsionStepSizeMin = keywords_.asDouble("TorsionStepSizeMin");
		const double rRT = 1.0/(.008314472*cfg->temperature());

		// Print argument/parameter summary
		Messenger::print("IntraShake: Cutoff distance is %f\n", cutoffDistance);
		Messenger::print("IntraShake: Performing %i shake(s) per term\n", nShakesPerTerm);
		if (adjustBonds) Messenger::print("IntraShake: Distance step size for bond adjustments is %f Angstroms (allowed range is %f <= delta <= %f).\n", bondStepSize, bondStepSizeMin, bondStepSizeMax);
		if (adjustAngles) Messenger::print("IntraShake: Angle step size for angle adjustments is %f degrees (allowed range is %f <= delta <= %f).\n", angleStepSize, angleStepSizeMin, angleStepSizeMax);
		if (adjustTorsions) Messenger::print("IntraShake: Rotation step size for torsion adjustments is %f degrees (allowed range is %f <= delta <= %f).\n", torsionStepSize, torsionStepSizeMin, torsionStepSizeMax);
		Messenger::print("IntraShake: Target acceptance rate is %f.\n", targetAcceptanceRate);
		if (termEnergyOnly) Messenger::print("IntraShake: Only term energy will be considered (interatomic contributions with the system will be excluded).\n");

		// Create a local ChangeStore and EnergyKernel
		ChangeStore changeStore(procPool);
		EnergyKernel kernel(procPool, cfg, duq.potentialMap(), cutoffDistance);

		// Initialise the random number buffer
		procPool.initialiseRandomBuffer(ProcessPool::Pool);

		int shake, nBondAttempts = 0, nAngleAttempts = 0, nTorsionAttempts = 0, nBondAccepted = 0, nAngleAccepted = 0, nTorsionAccepted = 0;
		int terminus;
		bool accept;
		double ppEnergy, newPPEnergy, intraEnergy, newIntraEnergy, delta, totalDelta = 0.0;
		Vec3<double> vji, vjk, v;
		Matrix3 transform;
		const Box* box = cfg->box();

		Timer timer;
		procPool.resetAccumulatedTime();
		for (int m = 0; m<cfg->nMolecules(); ++m)
		{
			/*
			 * Calculation Begins
			 */

			// Get Molecule pointer
			Molecule* mol = cfg->molecule(m);

			// Set current atom targets in ChangeStore (entire cell contents)
			changeStore.add(mol);

			// Calculate reference pairpotential energy for Molecule
			ppEnergy = termEnergyOnly ? 0.0 : kernel.energy(mol, ProcessPool::OverPoolProcesses);

			// Loop over defined Bonds
			if (adjustBonds) for (int n=0; n<mol->nBonds(); ++n)
			{
				// Grab Bond pointer
				Bond* b = mol->bond(n);

				// Store current energy of this intramolecular term
				intraEnergy = kernel.energy(b);

				// Select random terminus
				terminus = procPool.random() > 0.5 ? 1 : 0;

				// Loop over number of shakes per term
				for (shake=0; shake<nShakesPerTerm; ++shake)
				{
					// Get translation vector, normalise, and apply random delta
					vji = box->minimumVector(b->i(), b->j());
					vji.normalise();
					vji *= procPool.randomPlusMinusOne() * bondStepSize;

					// Adjust the Atoms attached to the selected terminus
					mol->translate(vji, b->nAttached(terminus), b->attached(terminus));

					// Calculate new energy
					newPPEnergy = termEnergyOnly ? 0.0 : kernel.energy(mol, ProcessPool::OverPoolProcesses);
					newIntraEnergy = kernel.energy(b);
					
					// Trial the transformed Molecule
					delta = (newPPEnergy + newIntraEnergy) - (ppEnergy + intraEnergy);
					accept = delta < 0 ? true : (procPool.random() < exp(-delta*rRT));

					// Accept new (current) positions of the Molecule's Atoms?
					if (accept)
					{
						changeStore.updateAll();
						ppEnergy = newPPEnergy;
						intraEnergy = newIntraEnergy;
						totalDelta += delta;
						++nBondAccepted;
					}
					else changeStore.revertAll();

					++nBondAttempts;
				}
			}

			// Loop over defined Angles
			if (adjustAngles) for (int n=0; n<mol->nAngles(); ++n)
			{
				// Grab Angle pointer
				Angle* a = mol->angle(n);

				// Store current energy of this intramolecular term
				intraEnergy = kernel.energy(a);

				// Select random terminus
				terminus = procPool.random() > 0.5 ? 1 : 0;

				// Loop over number of shakes per term
				for (shake=0; shake<nShakesPerTerm; ++shake)
				{
					// Get bond vectors and calculate cross product to get rotation axis
					vji = box->minimumVector(a->j(), a->i());
					vjk = box->minimumVector(a->j(), a->k());
					v = vji * vjk;

					// Create suitable transformation matrix
					transform.createRotationAxis(v.x, v.y, v.z, procPool.randomPlusMinusOne()*angleStepSize, true);

					// Adjust the Atoms attached to the selected terminus
					mol->transform(box, transform, a->j()->r(), a->nAttached(terminus), a->attached(terminus));

					// Calculate new energy
					newPPEnergy = termEnergyOnly ? 0.0 : kernel.energy(mol, ProcessPool::OverPoolProcesses);
					newIntraEnergy = kernel.energy(a);
					
					// Trial the transformed Molecule
					delta = (newPPEnergy + newIntraEnergy) - (ppEnergy + intraEnergy);
					accept = delta < 0 ? true : (procPool.random() < exp(-delta*rRT));

					// Accept new (current) positions of the Molecule's Atoms?
					if (accept)
					{
						changeStore.updateAll();
						ppEnergy = newPPEnergy;
						intraEnergy = newIntraEnergy;
						totalDelta += delta;
						++nAngleAccepted;
					}
					else changeStore.revertAll();

					++nAngleAttempts;
				}
			}

			// Loop over defined Torsions
			if (adjustTorsions) for (int n=0; n<mol->nTorsions(); ++n)
			{
				// Grab Torsion pointer
				Torsion* t = mol->torsion(n);

				// Store current energy of this intramolecular term
				intraEnergy = kernel.energy(t);

				// Select random terminus
				terminus = procPool.random() > 0.5 ? 1 : 0;

				// Loop over number of shakes per term
				for (shake=0; shake<nShakesPerTerm; ++shake)
				{
					// Get bond vectors j-k to get rotation axis
					vjk = box->minimumVector(t->j(), t->k());

					// Create suitable transformation matrix
					transform.createRotationAxis(vjk.x, vjk.y, vjk.z, procPool.randomPlusMinusOne()*torsionStepSize, true);

					// Adjust the Atoms attached to the selected terminus
					mol->transform(box, transform, terminus == 0 ? t->j()->r() : t->k()->r(), t->nAttached(terminus), t->attached(terminus));

					// Calculate new energy
					newPPEnergy = termEnergyOnly ? 0.0 : kernel.energy(mol, ProcessPool::OverPoolProcesses);
					newIntraEnergy = kernel.energy(t);
					
					// Trial the transformed Molecule
					delta = (newPPEnergy + newIntraEnergy) - (ppEnergy + intraEnergy);
					accept = delta < 0 ? true : (procPool.random() < exp(-delta*rRT));

					// Accept new (current) positions of the Molecule's Atoms?
					if (accept)
					{
						changeStore.updateAll();
						ppEnergy = newPPEnergy;
						intraEnergy = newIntraEnergy;
						totalDelta += delta;
						++nTorsionAccepted;
					}
					else changeStore.revertAll();

					++nTorsionAttempts;
				}
			}

			// Store modifications to Atom positions ready for broadcast
			changeStore.storeAndReset();

			/*
			* Calculation End
			*/

			// Distribute coordinate changes to all processes
			changeStore.distributeAndApply(cfg);
			changeStore.reset();
		}
		timer.stop();

		Messenger::print("IntraShake: Total energy delta was %10.4e kJ/mol.\n", totalDelta);
		Messenger::print("IntraShake: Total number of attempted moves was %i (%s work, %s comms).\n", nBondAttempts+nAngleAttempts+nTorsionAttempts, timer.totalTimeString(), procPool.accumulatedTimeString());

		// Calculate and report acceptance rates and adjust step sizes - if no moves were accepted, just decrease the current stepSize by a constant factor
		if (adjustBonds && (nBondAttempts > 0))
		{
			double bondRate = double(nBondAccepted) / nBondAttempts;
			Messenger::print("IntraShake: Overall bond shake acceptance rate was %4.2f% (%i of %i attempted moves).\n", 100.0*bondRate, nBondAccepted, nBondAttempts);

			bondStepSize *= (nBondAccepted == 0) ? 0.8 : bondRate/targetAcceptanceRate;
			if (bondStepSize < bondStepSizeMin) bondStepSize = bondStepSizeMin;
			else if (bondStepSize > bondStepSizeMax) bondStepSize = bondStepSizeMax;

			Messenger::print("IntraShake: Updated distance step size for bond adjustments is %f Angstroms.\n", bondStepSize); 
			GenericListHelper<double>::realise(moduleData, "BondStepSize", uniqueName(), GenericItem::InRestartFileFlag) = bondStepSize;
		}

		if (adjustAngles && (nAngleAttempts > 0))
		{
			double angleRate = double(nAngleAccepted) / nAngleAttempts;
			Messenger::print("IntraShake: Overall angle shake acceptance rate was %4.2f% (%i of %i attempted moves).\n", 100.0*angleRate, nAngleAccepted, nAngleAttempts);

			angleStepSize *= (nAngleAccepted == 0) ? 0.8 : angleRate/targetAcceptanceRate;
			if (angleStepSize < angleStepSizeMin) angleStepSize = angleStepSizeMin;
			else if (angleStepSize > angleStepSizeMax) angleStepSize = angleStepSizeMax;

			Messenger::print("IntraShake: Updated rotation step size for angle adjustments is %f degrees.\n", angleStepSize); 
			GenericListHelper<double>::realise(moduleData, "AngleStepSize", uniqueName(), GenericItem::InRestartFileFlag) = angleStepSize;
		}

		if (adjustTorsions && (nTorsionAttempts > 0))
		{
			double torsionRate = double(nTorsionAccepted) / nTorsionAttempts;
			Messenger::print("IntraShake: Overall torsion shake acceptance rate was %4.2f% (%i of %i attempted moves).\n", 100.0*torsionRate, nTorsionAccepted, nTorsionAttempts);

			torsionStepSize *= (nTorsionAccepted == 0) ? 0.8 : torsionRate/targetAcceptanceRate;
			if (torsionStepSize < torsionStepSizeMin) torsionStepSize = torsionStepSizeMin;
			else if (torsionStepSize > torsionStepSizeMax) torsionStepSize = torsionStepSizeMax;

			Messenger::print("IntraShake: Updated rotation step size for torsion adjustments is %f degrees.\n", torsionStepSize); 
			GenericListHelper<double>::realise(moduleData, "TorsionStepSize", uniqueName(), GenericItem::InRestartFileFlag) = torsionStepSize;
		}

		// Increment configuration changeCount_
		if ((nBondAccepted > 0) || (nAngleAccepted > 0) || (nTorsionAccepted > 0)) cfg->incrementCoordinateIndex();
	}

	return true;
}

// Run post-processing stage
bool IntraShakeModule::postProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}