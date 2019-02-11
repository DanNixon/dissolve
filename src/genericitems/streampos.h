/*
	*** Generic Item Container - StreamPos
	*** src/genericitems/streampos.h
	Copyright T. Youngs 2012-2019

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

#ifndef DISSOLVE_GENERICITEMCONTAINER_STREAMPOS_H
#define DISSOLVE_GENERICITEMCONTAINER_STREAMPOS_H

#include "genericitems/container.h"
#include <ios>

// GenericItemContainer<streampos>
template <> class GenericItemContainer<streampos> : public GenericItem
{
	public:
	// Constructor
	GenericItemContainer<streampos>(const char* name, int flags = 0) : GenericItem(name, flags)
	{
	}
	// Data item
	streampos data;


	/*
	 * Item Class
	 */
	protected:
	// Create a new GenericItem containing same class as current type
	GenericItem* createItem(const char* className, const char* name, int flags = 0)
	{
		if (DissolveSys::sameString(className, itemClassName())) return new GenericItemContainer<streampos>(name, flags);
		return NULL;
	}

	public:
	// Return class name contained in item
	const char* itemClassName()
	{
		return "streampos";
	}


	/*
	 * I/O
	 */
	public:
	// Write data through specified parser
	bool write(LineParser& parser)
	{
		return parser.writeLineF("%li\n", data);
	}
	// Read data through specified parser
	bool read(LineParser& parser, const CoreData& coreData)
	{
		if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success) return false;
		data = parser.argli(0);
		return true;
	}


	/*
	 * Parallel Comms
	 */
	public:
	// Broadcast item contents
	bool broadcast(ProcessPool& procPool, int root)
	{
		long int pos = (long int) data;
		return procPool.broadcast(pos, root);
		data = (streampos) pos;
	}
	// Check item equality
	bool equality(ProcessPool& procPool)
	{
		return procPool.equality(data);
	}
};

#endif