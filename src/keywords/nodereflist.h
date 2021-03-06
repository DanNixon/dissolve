/*
	*** Keyword - Node Reference List
	*** src/keywords/nodereflist.h
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

#ifndef DISSOLVE_KEYWORD_NODEREFLIST_H
#define DISSOLVE_KEYWORD_NODEREFLIST_H

#include "keywords/data.h"
#include "procedure/nodes/node.h"
#include "base/lineparser.h"

// Forward Declarations
class NodeValue;
class ProcedureNode;

// Keyword with ProcedureNode RefList
class NodeRefListKeywordBase
{
	public:
	// Constructor
	NodeRefListKeywordBase(ProcedureNode* parentNode, ProcedureNode::NodeType nodeType, bool onlyInScope);
	// Destructor
	virtual ~NodeRefListKeywordBase();


	/*
	 * Parent Node
	 */
	private:
	// Parent ProcedureNode
	ProcedureNode* parentNode_;

	public:
	// Return parent ProcedureNode
	ProcedureNode* parentNode() const;


	/*
	 * Target Node
	 */
	private:
	// Target node type to allow
	ProcedureNode::NodeType nodeType_;
	// Whether to accept nodes within scope only
	bool onlyInScope_;

	public:
	// Return target node type to allow
	ProcedureNode::NodeType nodeType() const;
	// Return whether to accept nodes within scope only
	bool onlyInScope() const;
	// Add the specified node to the list
	virtual bool addNode(ProcedureNode* node) = 0;
	// Return the current list (as const ProcedureNodes)
	virtual RefList<const ProcedureNode> nodes() const = 0;
	// Return if the specified node is in the current list
	virtual bool hasNode(ProcedureNode* node) = 0;
	// Remove the specified node from the list
	virtual bool removeNode(ProcedureNode* node) = 0;
};

// Keyword with ProcedureNode RefList
template <class N> class NodeRefListKeyword : public NodeRefListKeywordBase, public KeywordData< RefList<N>& >
{
	public:
	// Constructor
	NodeRefListKeyword(ProcedureNode* parentNode, ProcedureNode::NodeType nodeType, bool onlyInScope, RefList<N>& nodeRefList) : NodeRefListKeywordBase(parentNode, nodeType, onlyInScope), KeywordData< RefList<N>& >(KeywordBase::NodeRefListData, nodeRefList)
	{
	}
	// Destructor
	~NodeRefListKeyword()
	{
	}


	/*
	 * Arguments
	 */
	public:
	// Return minimum number of arguments accepted
	int minArguments() const
	{
		return 1;
	}
	// Return maximum number of arguments accepted
	int maxArguments() const
	{
		return 99;
	}
	// Parse arguments from supplied LineParser, starting at given argument offset
	bool read(LineParser& parser, int startArg, const CoreData& coreData)
	{
		if (!parentNode()) return Messenger::error("Can't read keyword %s since the parent ProcedureNode has not been set.\n", KeywordBase::name());

		// Loop over arguments
		for (int n=startArg; n<parser.nArgs(); ++n)
		{
			// Locate the named node - don't prune by type yet (we'll check that in setNode())
			ProcedureNode* node = onlyInScope() ? parentNode()->nodeInScope(parser.argc(n)) : parentNode()->nodeExists(parser.argc(n));
			if (!node) return Messenger::error("Node '%s' given to keyword %s doesn't exist.\n", parser.argc(n), KeywordBase::name());

			if (!addNode(node)) return false;
		}

		return true;
	}
	// Write keyword data to specified LineParser
	bool write(LineParser& parser, const char* keywordName, const char* prefix)
	{
		if (KeywordData< RefList<N>& >::data_.nItems() == 0) return true;

		CharString nodes;
		RefListIterator<N> nodeIterator(KeywordData< RefList<N>& >::data_);
		while (N* node = nodeIterator.iterate()) nodes.strcatf("  '%s'", node->name());

		if (!parser.writeLineF("%s%s  %s\n", prefix, KeywordBase::name(), nodes.get())) return false;

		return true;
	}


	/*
	 * Target Node
	 */
	public:
	// Add the specified node to the list
	bool addNode(ProcedureNode* node)
	{
		if (!node) return false;

		if (node->type() != nodeType()) return Messenger::error("Node '%s' is of type %s, but the %s keyword requires a node of type %s.\n", node->name(), ProcedureNode::nodeTypes().keyword(node->type()), KeywordBase::name(), ProcedureNode::nodeTypes().keyword(nodeType()));

		// Cast up the node
		N* castNode = dynamic_cast<N*>(node);
		if (!castNode) return false;

		// If this node is already in the list, complain
		if (KeywordData< RefList<N>& >::data_.contains(castNode)) return Messenger::error("Node '%s' is already present in this list for keyword %s.\n", castNode->name(), KeywordBase::name());

		// Add the node to the list
		KeywordData< RefList<N>& >::data_.append(castNode);

		KeywordData< RefList<N>& >::set_ = true;

		return true;
	}
	// Return the current list (as ProcedureNodes)
	RefList<const ProcedureNode> nodes() const
	{
		RefList<const ProcedureNode> nodes;
		
		RefListIterator<N> nodeIterator(KeywordData< RefList<N>& >::data_);
		while (N* node = nodeIterator.iterate()) nodes.append(node);
	
		return nodes;
	}
	// Return if the specified node is in the current list
	bool hasNode(ProcedureNode* node)
	{
		RefListIterator<N> nodeIterator(KeywordData< RefList<N>& >::data_);
		while (N* existingNode = nodeIterator.iterate()) if (existingNode == node) return true;

		return false;
	}
	// Remove the specified node from the list
	bool removeNode(ProcedureNode* node)
	{
		if (!node) return false;

		if (node->type() != nodeType()) return Messenger::error("Node '%s' is of type %s, but the %s keyword stores nodes of type %s, so not attempting to remove it.\n", node->name(), ProcedureNode::nodeTypes().keyword(node->type()), KeywordBase::name(), ProcedureNode::nodeTypes().keyword(nodeType()));

		// Cast up the node
		N* castNode = dynamic_cast<N*>(node);
		if (!castNode) return false;

		// Check that the list actually contains the specified node
		if (!KeywordData< RefList<N>& >::data_.contains(castNode)) return Messenger::error("Node '%s' is not in this keyword's (%s) list of nodes, so can't remove it.\n", castNode->name(), KeywordBase::name());

		KeywordData< RefList<N>& >::data_.remove(castNode);

		return true;
	}


	/*
	 * Object Management
	 */
	protected:
	// Prune any references to the supplied ProcedureNode in the contained data
	void removeReferencesTo(ProcedureNode* node)
	{
		// Check the node type
		if (node->type() != nodeType()) return;

		// Cast up the node
		N* castNode = dynamic_cast<N*>(node);
		if (!castNode) return;

		if (KeywordData< RefList<N>& >::data_.contains(castNode)) KeywordData< RefList<N>& >::data_.remove(castNode);
	}
};

#endif

