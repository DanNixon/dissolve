/*
	*** AtomTypeList Definition
	*** src/classes/atomtypelist.h
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

#ifndef DUQ_ATOMTYPELIST_H
#define DUQ_ATOMTYPELIST_H

#include "classes/atomtypedata.h"
#include "templates/list.h"
#include "templates/array.h"

// Forward Declarations
class AtomType;
class Isotope;

/*
 * AtomTypeList Definition
 */
class AtomTypeList
{
	public:
	// Constructor
	AtomTypeList();
	// Destructor
	~AtomTypeList();
	// Copy Constructor
	AtomTypeList(const AtomTypeList& source);
	// Assignment Operator
	void operator=(const AtomTypeList& source);


	/*
	 * Type List
	 */
	private:
	// List of AtomTypeData
	List<AtomTypeData> types_;

	public:
	// Clear all data
	void clear();
	// Add/increase this AtomType/Isotope pair, returning the index of the AtomType in the list
	int add(AtomType* atomType, Isotope* tope, int popAdd = 0);
	// Add the AtomTypes in the supplied list into this one, increasing populations etc.
	void add(const AtomTypeList& source);
	// Check for presence of AtomType/Isotope pair in list
	bool contains(AtomType* atomType, Isotope* tope);
	// Zero populations of all types in the list
	void zero();
	// Return number of AtomType/Isotopes in list
	int nItems() const;
	// Return first item in list
	AtomTypeData* first() const;
	// Print AtomType populations
	void print() const;


	/*
	 * Access
	 */
	public:
	// Return index of AtomType/Isotope in list
	int indexOf(AtomType* atomtype) const;
	// Return total population of all types in list
	int totalPopulation() const;
	// Finalise list, calculating fractional populations etc.
	void finalise();
	// Return nth referenced AtomType
	AtomType* atomType(int n);
	// Array access operator
	AtomTypeData* operator[](int n);
};

#endif
