/*
	*** Isotope Data
	*** src/classes/isotopedata.h
	Copyright T. Youngs 2012-2017

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

#ifndef DUQ_ISOTOPEDATA_H
#define DUQ_ISOTOPEDATA_H

#include "templates/mpilistitem.h"

// Forward Declarations
class Isotope;

/*
 * IsotopeData Definition
 */
class IsotopeData : public MPIListItem<IsotopeData>
{
	public:
	// Constructor
	IsotopeData();
	// Copy Constructor
	IsotopeData(const IsotopeData& source);
	// Assignment Operator
	void operator=(const IsotopeData& source);


	/*
	 * Properties
	 */
	private:
	// Reference Isotope
	Isotope* isotope_;
	// Integer population of Isotope
	int population_;
	// Local fractional population (e.g. within an AtomTypeData)
	double fraction_;

	public:
	// Initialise
	bool initialise(Isotope* isotope);
	// Add to population of Isotope
	void add(int nAdd);
	// Finalise, calculating local fractional population (e.g. within an AtomTypeData)
	void finalise(int totalAtoms);
	// Zero population and fraction
	void zeroPopulation();
	// Return reference Isotope
	Isotope* isotope() const;
	// Return population
	int population() const;
	// Return local fractional population (e.g. within an AtomTypeData)
	double fraction() const;


	/*
	 * Parallel Comms
	 */
	public:
	// Broadcast data from Master to all Slaves
	bool broadcast(ProcessPool& procPool, int root = 0);
};

#endif
