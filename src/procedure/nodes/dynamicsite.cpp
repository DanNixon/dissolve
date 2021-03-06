/*
	*** Procedure Node - Dynamic Site
	*** src/procedure/nodes/dynamicsite.cpp
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

#include "procedure/nodes/dynamicsite.h"
#include "procedure/nodes/select.h"
#include "keywords/types.h"
#include "classes/atom.h"
#include "classes/configuration.h"
#include "classes/coredata.h"
#include "classes/molecule.h"
#include "classes/site.h"
#include "classes/species.h"
#include "data/elements.h"
#include "classes/atomtype.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "templates/dynamicarray.h"

// Constructor
DynamicSiteProcedureNode::DynamicSiteProcedureNode(SelectProcedureNode* parent) : ProcedureNode(ProcedureNode::DynamicSiteNode)
{
	parent_ = parent;

	keywords_.add("Definition", new AtomTypeRefListKeyword(atomTypes_), "AtomType", "Define one or more AtomTypes to include in this site");
	keywords_.add("Definition", new ElementRefListKeyword(elements_), "Element", "Define one or more Elements to include in this site");
}

// Destructor
DynamicSiteProcedureNode::~DynamicSiteProcedureNode()
{
}

/*
 * Identity
 */

// Return whether specified context is relevant for this node type
bool DynamicSiteProcedureNode::isContextRelevant(ProcedureNode::NodeContext context)
{
	return (context == ProcedureNode::AnalysisContext);
}

// Return whether a name for the node must be provided
bool DynamicSiteProcedureNode::mustBeNamed() const
{
	return false;
}

/*
 * Site Generation
 */

// Generate sites from the specified Molecule
void DynamicSiteProcedureNode::generateSites(const Molecule* molecule)
{
	// Loop over Atoms in the Molecule
	for (int n=0; n<molecule->nAtoms(); ++n)
	{
		// If the element is listed in our target elements list, add this atom as a site
		if (elements_.contains(molecule->atom(n)->speciesAtom()->element()))
		{
			generatedSites_.add(Site(molecule, molecule->atom(n)->r()));
			continue;
		}

		// If the Atom's AtomType is listed in our target AtomType list, add this atom as a site
		if (atomTypes_.contains(molecule->atom(n)->speciesAtom()->atomType()))
		{
			generatedSites_.add(Site(molecule, molecule->atom(n)->r()));
			continue;
		}
	}
}

// Return Array of generated sites
const Array<Site>& DynamicSiteProcedureNode::generatedSites() const
{
	return generatedSites_;
}

/*
 * Execute
 */

// Execute node, targetting the supplied Configuration
ProcedureNode::NodeExecutionResult DynamicSiteProcedureNode::execute(ProcessPool& procPool, Configuration* cfg, const char* prefix, GenericList& targetList)
{
	// Clear our current list of sites
	generatedSites_.clear();

	// Grab exclusion lists and any specific Molecule parent
	const RefList<const Molecule>& excludedMolecules = parent_->excludedMolecules();
	const Molecule* moleculeParent = parent_->sameMoleculeMolecule();

	/*
	 * We'll loop over all Molecules in the Configuration, rather than all Atoms, since there are useful exclusions we can make based on the parent Molecule.
	 * If, however, a sameMolecule_ is defined then we can simply grab this Molecule and do the checks within it, rather than looping.
	 * Both use the local function generateSites(Molecule*) in order to extract site information.
	 */

	if (moleculeParent) generateSites(moleculeParent);
	else
	{
		// Loop over Molecules in the target Configuration
		DynamicArray<Molecule>& molecules = cfg->molecules();
		for (int n=0; n<molecules.nItems(); ++n)
		{
			// Check Molecule exclusions
			if (excludedMolecules.contains(molecules[n])) continue;

			// All OK, so generate sites
			generateSites(molecules[n]);
		}
	}

	return ProcedureNode::Success;
}
