/*
	*** NETA Connection Node
	*** src/neta/connection.cpp
	Copyright T. Youngs 2019

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

#include "neta/connection.h"
#include "data/ffatomtype.h"
#include "classes/speciesatom.h"
#include "templates/dynamicarray.h"
#include "templates/refdatalist.h"

// Constructor
NETAConnectionNode::NETAConnectionNode(NETADefinition* parent, PointerArray<Element> targetElements, PointerArray<ForcefieldAtomType> targetAtomTypes, SpeciesBond::BondType bt) : NETANode(parent, NETANode::ConnectionNode)
{
	allowedElements_ = targetElements;
	allowedAtomTypes_ = targetAtomTypes;
	bondType_ = bt;
}

// Destructor
NETAConnectionNode::~NETAConnectionNode()
{
}

/*
 * Scoring
 */

// Evaluate the node and return its score
int NETAConnectionNode::score(const SpeciesAtom* i, RefList<const SpeciesAtom>& matchPath) const
{
// 	printf("I AM THE CONNECTION - matchPath size = %i:\n", matchPath.nItems());
// 	RefListIterator<const SpeciesAtom> matchIterator(matchPath);
// 	while (const SpeciesAtom* iii = matchIterator.iterate()) printf("   -- %p %i %s\n", iii, iii->userIndex(), iii->element()->symbol());
// 	printf("SITTING ON SPECIESATOM %i (%s)\n", i->userIndex(), i->element()->symbol());

	// Get directly connected atoms about 'i', excluding any that have already been matched
	RefDataList<const SpeciesAtom, int> neighbours;
	const PointerArray<SpeciesBond>& bonds = i->bonds();
	for (int n=0; n<bonds.nItems(); ++n)
	{
		const SpeciesAtom* j = bonds.at(n)->partner(i);
		if (matchPath.contains(j)) continue;
		neighbours.append(j, NETANode::NoMatch);
	}

	// Loop over neighbour atoms
	int nMatches = 0;
	RefDataListIterator<const SpeciesAtom,int> neighbourIterator(neighbours);
	while (const SpeciesAtom* j = neighbourIterator.iterate())
	{
		// Evaluate the neighbour against our elements
		int atomScore = NETANode::NoMatch;
		for (int n=0; n<allowedElements_.nItems(); ++n)
		{
			if (j->element() != allowedElements_.at(n)) continue;

			// Process branch definition via the base class, using a copy of the current match path
			RefList<const SpeciesAtom> branchMatchPath = matchPath;
			int branchScore = NETANode::score(j, branchMatchPath);
			if (branchScore == NETANode::NoMatch) continue;

			// Create total score (element match plus branch score)
			atomScore = 1 + branchScore;

			// Now have a match, so break out of the loop
			break;
		}
		if (atomScore == NETANode::NoMatch) for (int n=0; n<allowedAtomTypes_.nItems(); ++n)
		{
			// Evaluate the neighbour against the atom type
			int typeScore = allowedAtomTypes_.at(n)->neta().score(j);
			if (typeScore == NETANode::NoMatch) continue;

			// Process branch definition via the base class, using a copy of the current match path
			RefList<const SpeciesAtom> branchMatchPath = matchPath;
			int branchScore = NETANode::score(j, branchMatchPath);
			if (branchScore == NETANode::NoMatch) continue;

			// Create total score
			atomScore = typeScore + branchScore;

			// Now have a match, so break out of the loop
			break;
		}

		// Did we match the atom?
		if (atomScore == NETANode::NoMatch) continue;

		// Found a match, so increase the match count and store the score
		++nMatches;
		neighbourIterator.currentData() = atomScore;

		// Have we matched enough? If so break out early.
		if (compareValues(nMatches, repeatCountOperator_, repeatCount_)) break;
	}

	// Did we find the required number of matches in the neighbour list?
	if (!compareValues(nMatches, repeatCountOperator_, repeatCount_)) return NETANode::NoMatch;

	// Generate total score and add matched atoms to path
	int totalScore = 0;
	neighbourIterator.restart();
	while (const SpeciesAtom* j = neighbourIterator.iterate())
	{
		if (neighbourIterator.currentData() == NETANode::NoMatch) continue;

		totalScore += neighbourIterator.currentData();
		matchPath.append(j);
	}

	return totalScore;
}