/*
	*** Keywords Widget - Functions
	*** src/gui/keywordwidgets/widget_funcs.cpp
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

#include "gui/keywordwidgets/widget.hui"
#include "gui/keywordwidgets/widgets.h"
#include "module/module.h"
#include "main/dissolve.h"
#include "base/lineparser.h"
#include <QToolBox>
#include <QFormLayout>
#include <QLabel>

// Constructor
KeywordsWidget::KeywordsWidget(QWidget* parent) : QToolBox(parent)
{
	refreshing_ = false;
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

KeywordsWidget::~KeywordsWidget()
{
}

/*
 * Controls
 */

// Create widget for specified keyword
QWidget* KeywordsWidget::createKeywordWidget(RefList<KeywordWidgetBase>& keywordWidgets, KeywordBase* keyword, const CoreData& coreData)
{
	QWidget* widget = NULL;
	KeywordWidgetBase* base = NULL;

	// Ensure that we have the base keyword pointer, and its datatype
	KeywordBase* keywordBase = keyword->base();
	KeywordBase::KeywordDataType type = keywordBase->type();

	// Try to create a suitable widget	
	if (type == KeywordBase::AtomTypeSelectionData)
	{
		AtomTypeSelectionKeywordWidget* atomTypeSelectionWidget = new AtomTypeSelectionKeywordWidget(NULL, keywordBase, coreData);
		connect(atomTypeSelectionWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = atomTypeSelectionWidget;
		base = atomTypeSelectionWidget;
	}
	else if (type == KeywordBase::BoolData)
	{
		BoolKeywordWidget* boolWidget = new BoolKeywordWidget(NULL, keywordBase, coreData);
		connect(boolWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = boolWidget;
		base = boolWidget;
	}
	else if (type == KeywordBase::BroadeningFunctionData)
	{
		BroadeningFunctionKeywordWidget* broadeningFunctionWidget = new BroadeningFunctionKeywordWidget(NULL, keywordBase, coreData);
		connect(broadeningFunctionWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = broadeningFunctionWidget;
		base = broadeningFunctionWidget;
	}
	else if (type == KeywordBase::CharStringData)
	{
		CharStringKeywordWidget* charWidget = new CharStringKeywordWidget(NULL, keywordBase, coreData);
		connect(charWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = charWidget;
		base = charWidget;
	}
	else if (type == KeywordBase::DoubleData)
	{
		DoubleKeywordWidget* doubleWidget = new DoubleKeywordWidget(NULL, keywordBase, coreData);
		connect(doubleWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = doubleWidget;
		base = doubleWidget;
	}
	else if (type == KeywordBase::EnumOptionsData)
	{
		EnumOptionsKeywordWidget* enumOptionsWidget = new EnumOptionsKeywordWidget(NULL, keywordBase, coreData);
		connect(enumOptionsWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = enumOptionsWidget;
		base = enumOptionsWidget;
	}
	else if (type == KeywordBase::ExpressionVariableListData)
	{
		ExpressionVariableListKeywordWidget* expressionVariableListWidget = new ExpressionVariableListKeywordWidget(NULL, keywordBase, coreData);
		connect(expressionVariableListWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = expressionVariableListWidget;
		base = expressionVariableListWidget;
	}
	else if (type == KeywordBase::FileAndFormatData)
	{
		FileAndFormatKeywordWidget* fileAndFormatWidget = new FileAndFormatKeywordWidget(NULL, keywordBase, coreData);
		connect(fileAndFormatWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = fileAndFormatWidget;
		base = fileAndFormatWidget;
	}
	else if (type == KeywordBase::IntegerData)
	{
		IntegerKeywordWidget* intWidget = new IntegerKeywordWidget(NULL, keywordBase, coreData);
		connect(intWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = intWidget;
		base = intWidget;
	}
	else if (type == KeywordBase::IsotopologueCollectionData)
	{
		IsotopologueCollectionKeywordWidget* isotopologueCollectionWidget = new IsotopologueCollectionKeywordWidget(NULL, keywordBase, coreData);
		connect(isotopologueCollectionWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = isotopologueCollectionWidget;
		base = isotopologueCollectionWidget;
	}
	else if (type == KeywordBase::ModuleData)
	{
		ModuleKeywordWidget* moduleWidget = new ModuleKeywordWidget(NULL, keywordBase, coreData);
		connect(moduleWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = moduleWidget;
		base = moduleWidget;
	}
	else if (type == KeywordBase::ModuleGroupsData)
	{
		ModuleGroupsKeywordWidget* moduleGroupsWidget = new ModuleGroupsKeywordWidget(NULL, keywordBase, coreData);
		connect(moduleGroupsWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = moduleGroupsWidget;
		base = moduleGroupsWidget;
	}
	else if (type == KeywordBase::ModuleRefListData)
	{
		ModuleRefListKeywordWidget* moduleRefListWidget = new ModuleRefListKeywordWidget(NULL, keywordBase, coreData);
		connect(moduleRefListWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = moduleRefListWidget;
		base = moduleRefListWidget;
	}
	else if (type == KeywordBase::NodeData)
	{
		NodeKeywordWidget* nodeWidget = new NodeKeywordWidget(NULL, keywordBase, coreData);
		connect(nodeWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = nodeWidget;
		base = nodeWidget;
	}
	else if (type == KeywordBase::NodeAndIntegerData)
	{
		NodeAndIntegerKeywordWidget* nodeAndIntegerWidget = new NodeAndIntegerKeywordWidget(NULL, keywordBase, coreData);
		connect(nodeAndIntegerWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = nodeAndIntegerWidget;
		base = nodeAndIntegerWidget;
	}
	else if (type == KeywordBase::NodeRefListData)
	{
		NodeRefListKeywordWidget* nodeRefListWidget = new NodeRefListKeywordWidget(NULL, keywordBase, coreData);
		connect(nodeRefListWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = nodeRefListWidget;
		base = nodeRefListWidget;
	}
	else if (type == KeywordBase::NodeValueData)
	{
		NodeValueKeywordWidget* nodeValueWidget = new NodeValueKeywordWidget(NULL, keywordBase, coreData);
		connect(nodeValueWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = nodeValueWidget;
		base = nodeValueWidget;
	}
	else if (type == KeywordBase::NodeValueEnumOptionsData)
	{
		NodeValueEnumOptionsKeywordWidget* nodeValueEnumOptionsWidget = new NodeValueEnumOptionsKeywordWidget(NULL, keywordBase, coreData);
		connect(nodeValueEnumOptionsWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = nodeValueEnumOptionsWidget;
		base = nodeValueEnumOptionsWidget;
	}
	else if (type == KeywordBase::PairBroadeningFunctionData)
	{
		PairBroadeningFunctionKeywordWidget* pairBroadeningFunctionWidget = new PairBroadeningFunctionKeywordWidget(NULL, keywordBase, coreData);
		connect(pairBroadeningFunctionWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = pairBroadeningFunctionWidget;
		base = pairBroadeningFunctionWidget;
	}
	else if (type == KeywordBase::RangeData)
	{
		RangeKeywordWidget* rangeWidget = new RangeKeywordWidget(NULL, keywordBase, coreData);
		connect(rangeWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = rangeWidget;
		base = rangeWidget;
	}
	else if (type == KeywordBase::SpeciesData)
	{
		SpeciesKeywordWidget* speciesWidget = new SpeciesKeywordWidget(NULL, keywordBase, coreData);
		connect(speciesWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = speciesWidget;
		base = speciesWidget;
	}
	else if (type == KeywordBase::SpeciesRefListData)
	{
		SpeciesRefListKeywordWidget* speciesRefListWidget = new SpeciesRefListKeywordWidget(NULL, keywordBase, coreData);
		connect(speciesRefListWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = speciesRefListWidget;
		base = speciesRefListWidget;
	}
	else if (type == KeywordBase::SpeciesSiteData)
	{
		SpeciesSiteKeywordWidget* speciesSiteWidget = new SpeciesSiteKeywordWidget(NULL, keywordBase, coreData);
		connect(speciesSiteWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = speciesSiteWidget;
		base = speciesSiteWidget;
	}
	else if (type == KeywordBase::SpeciesSiteRefListData)
	{
		SpeciesSiteRefListKeywordWidget* speciesSiteRefListWidget = new SpeciesSiteRefListKeywordWidget(NULL, keywordBase, coreData);
		connect(speciesSiteRefListWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = speciesSiteRefListWidget;
		base = speciesSiteRefListWidget;
	}
	else if (type == KeywordBase::WindowFunctionData)
	{
		WindowFunctionKeywordWidget* windowFunctionWidget = new WindowFunctionKeywordWidget(NULL, keywordBase, coreData);
		connect(windowFunctionWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = windowFunctionWidget;
		base = windowFunctionWidget;
	}
	else if (type == KeywordBase::Vec3DoubleData)
	{
		Vec3DoubleKeywordWidget* vec3DoubleWidget = new Vec3DoubleKeywordWidget(NULL, keywordBase, coreData);
		connect(vec3DoubleWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = vec3DoubleWidget;
		base = vec3DoubleWidget;
	}
	else if (type == KeywordBase::Vec3IntegerData)
	{
		Vec3IntegerKeywordWidget* vec3IntWidget = new Vec3IntegerKeywordWidget(NULL, keywordBase, coreData);
		connect(vec3IntWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = vec3IntWidget;
		base = vec3IntWidget;
	}
	else if (type == KeywordBase::Vec3NodeValueData)
	{
		Vec3NodeValueKeywordWidget* vec3NodeValueWidget = new Vec3NodeValueKeywordWidget(NULL, keywordBase, coreData);
		connect(vec3NodeValueWidget, SIGNAL(keywordValueChanged(int)), this, SLOT(keywordDataChanged(int)));
		widget = vec3NodeValueWidget;
		base = vec3NodeValueWidget;
	}

	// Set tooltip on widget (using the description from the keyword pointer passed, rather than its base) and add to the list of widgets
	if (widget)
	{
		widget->setToolTip(keyword->description());
		keywordWidgets.append(base);
	}

	return widget;
}

// Set up keyword controls for specified keyword list
void KeywordsWidget::setUp(const KeywordList& keywords, const CoreData& coreData)
{
	// Clear existing item groups....
	while (count() > 0) removeItem(0);
	keywordWidgets_.clear();

	// Loop over keyword groups first - we'll keep track of which keywords are not part of a group, and these in an 'Other' tab at the end
	RefList<KeywordBase> remainingKeywords;
	ListIterator<KeywordBase> keywordIterator(keywords.keywords());
	while (KeywordBase* keyword = keywordIterator.iterate()) remainingKeywords.append(keyword);

	ListIterator<KeywordGroup> groupIterator(keywords.groups());
	while (KeywordGroup* group = groupIterator.iterate())
	{
		// If this is the 'HIDDEN' group, don't display any of its widgets
		if (DissolveSys::sameString(group->name(), "HIDDEN"))
		{
			// Remove all keywords in this group from the remainingKeywords list
			RefListIterator<KeywordBase> groupKeywordIterator(group->keywords());
			while (KeywordBase* keyword = groupKeywordIterator.iterate()) remainingKeywords.remove(keyword);

			continue;
		}

		// Create a new QWidget and layout for our widgets
		QWidget* groupWidget = new QWidget;
		QFormLayout* groupLayout = new QFormLayout(groupWidget);

		// Loop over keywords in the group and add them to our groupbox
		RefListIterator<KeywordBase> groupKeywordIterator(group->keywords());
		while (KeywordBase* keyword = groupKeywordIterator.iterate())
		{
			// Create / setup the keyword widget
			QWidget* widget = createKeywordWidget(keywordWidgets_, keyword, coreData);

			// Can now remove this keyword from our reference list
			remainingKeywords.remove(keyword);

			if (!widget)
			{
				// WORKAROUND - Don't raise errors for datastore types (issue #36)
				if ((keyword->type() == KeywordBase::Data1DStoreData) || (keyword->type() == KeywordBase::Data2DStoreData) || (keyword->type() == KeywordBase::Data3DStoreData)) continue;

				Messenger::error("Can't create widget for keyword '%s' (%s).\n", keyword->name(), KeywordBase::keywordDataType(keyword->type()));
				continue;
			}

			// Create a label and add it and the widget to our layout
			QLabel* nameLabel = new QLabel(keyword->name());
			nameLabel->setToolTip(keyword->description());
			groupLayout->addRow(nameLabel, widget);
		}

		// Group is finished, so add the widget as a new tab in our QToolBox
		addItem(groupWidget, group->name());
	}

	// If there are any 'group-orphaned' keywords, add these at the bottom
	if (remainingKeywords.nItems() > 0)
	{
		// Need a widget with a QFormLayout...
		QWidget* groupWidget = new QWidget;
		QFormLayout* groupLayout = new QFormLayout(groupWidget);

		RefListIterator<KeywordBase> remainingKeywordsIterator(remainingKeywords);
		while (KeywordBase* keyword = remainingKeywordsIterator.iterate())
		{
			// Create / setup the keyword widget
			QWidget* widget = createKeywordWidget(keywordWidgets_, keyword, coreData);

			if (!widget)
			{
				Messenger::error("Can't create widget for keyword '%s'.\n", keyword->name());
				continue;
			}

			// Create a label and add it and the widget to our layout
			QLabel* nameLabel = new QLabel(keyword->name());
			nameLabel->setToolTip(keyword->description());
			groupLayout->addRow(nameLabel, widget);
		}

		// Group is finished, so add the widget as a new tab in our QToolBox
		addItem(groupWidget, "Other");
	}
}

// Update controls within widget
void KeywordsWidget::updateControls()
{
	refreshing_ = true;

	// Update all our keyword widgets
	RefListIterator<KeywordWidgetBase> keywordIterator(keywordWidgets_);
	while (KeywordWidgetBase* keywordWidget = keywordIterator.iterate()) keywordWidget->updateValue();

	refreshing_ = false;
}

/*
 * Signals / Slots
 */

// Keyword data changed
void KeywordsWidget::keywordDataChanged(int flags)
{
	// Always emit the 'dataModified' signal
	emit(dataModified());

	// Set-up of encompassing class required?
	if (flags&KeywordBase::ModificationRequiresSetUpOption) emit(setUpRequired());
}
