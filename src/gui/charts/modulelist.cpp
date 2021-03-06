/*
	*** ModuleList Chart
	*** src/gui/charts/modulelist.cpp
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

#include "gui/charts/modulelist.h"
#include "gui/charts/modulelistmetrics.h"
#include "gui/charts/moduleblock.h"
#include "gui/charts/moduleinsertionblock.h"
#include "gui/widgets/mimestrings.h"
#include "main/dissolve.h"
#include "module/list.h"
#include "module/module.h"
#include <QApplication>
#include <QDrag>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QWidget>

// Constructor
ModuleListChart::ModuleListChart(ModuleList* moduleList, Dissolve& dissolve, Configuration* localConfiguration) : ChartBase(), dissolve_(dissolve)
{
	refreshing_ = false;

	// Target ModuleLayer
	moduleList_ = moduleList;
	localConfiguration_ = localConfiguration;

	// Create the insertion widget if we don't already have one
	insertionBlock_ = new ModuleInsertionBlock(this);
	insertionBlock_->setVisible(false);

	updateContentBlocks();

	recalculateLayout();

	updateControls();
}

ModuleListChart::~ModuleListChart()
{
}

/*
 * QWidget Reimplementations
 */

// Paint event
void ModuleListChart::paintEvent(QPaintEvent* event)
{
	// Draw suitable connecting lines between widgets, illustrating the execution path of the code
	QPainter painter(this);

	// Draw the background before we do anything else
	QBrush backgroundBrush = QBrush(Qt::black, QPixmap(":/images/images/squares.jpg"));
	painter.fillRect(QRect(0, 0, width(), height()), backgroundBrush);

	// Set up some QPens
	QPen solidPen(Qt::black);
	solidPen.setWidth(metrics_.blockBorderWidth());

	// Create a metrics object
	ModuleListChartMetrics metrics;

	// Draw lines between module widgets
	painter.setPen(solidPen);
	QPoint p1, p2;
	int top = 0;
	ModuleBlock* lastBlock = NULL;
	RefListIterator<ModuleBlock> blockIterator(moduleBlockWidgets_);
	while (ModuleBlock* block = blockIterator.iterate())
	{
		// If this block is not visible, continue
		if (!block->isVisible()) continue;

		// Draw connecting line between blocks
		p1 = QPoint(width() / 2, top);
		p2 = QPoint(width() / 2, block->geometry().top());
		painter.drawLine(p1, p2);
		painter.setBrush(Qt::black);
		painter.drawEllipse(p2, metrics.blockDentRadius()-metrics.blockBorderWidth()-1, metrics.blockDentRadius()-metrics.blockBorderWidth()-1);

		top = block->geometry().bottom();
	}

	// Is there a current selected block?
	if (selectedBlock_)
	{
		// Cast up the selectedBlock_ to a ModuleBlock
		ModuleBlock* selectedModule = dynamic_cast<ModuleBlock*>(selectedBlock_);
		if ((selectedModule) && (moduleBlockWidgets_.contains(selectedModule)))
		{
			QRect rect = selectedModule->geometry();
			rect.adjust(-metrics.chartMargin(), -15, metrics.chartMargin(), 15);
			painter.fillRect(rect, QColor(49,0,73,80));
		}
		else selectedBlock_ = NULL;
	}

	// Highlight all hotspots
	if (false)
	{
		ListIterator<ChartHotSpot> hotSpotIterator(hotSpots_);
		while (ChartHotSpot* hotSpot = hotSpotIterator.iterate()) painter.fillRect(hotSpot->geometry(), QBrush(QColor(200,200,0,50)));
	}
}

/*
 * Chart Blocks
 */

// Find ModuleBlock displaying specified Module
ModuleBlock* ModuleListChart::moduleBlock(Module* module)
{
	RefListIterator<ModuleBlock> moduleBlockIterator(moduleBlockWidgets_);
	while (ModuleBlock* block = moduleBlockIterator.iterate()) if (block->module() == module) return block;

	return NULL;
}

// Update the content block widgets against the current target data
void ModuleListChart::updateContentBlocks()
{
	if (!moduleList_) return;

	// Create a temporary list that will store our widgets to be 'reused'
	RefList<ModuleBlock> newWidgets;

	// Iterate through the nodes in this sequence, searching for their widgets in the oldWidgetsList
	ListIterator<Module> moduleIterator(moduleList_->modules());
	while (Module* module = moduleIterator.iterate())
	{
		// Does this Module have an existing widget?
		ModuleBlock* block = moduleBlock(module);
		if (block)
		{
			// Widget already exists, so remove the reference from nodeWidgets_ and add it to the new list
			newWidgets.append(block);
			moduleBlockWidgets_.remove(block);
			Messenger::printVerbose("Using existing ModuleBlock %p for Module %p (%s).\n", block, module, module->uniqueName());
		}
		else
		{
			// No current widget, so must create one
			block = new ModuleBlock(this, module, dissolve_);
			connect(block, SIGNAL(dataModified()), this, SLOT(chartDataModified()));
			connect(block, SIGNAL(remove(const QString&)), this, SLOT(blockRemovalRequested(const QString&)));
			newWidgets.append(block);
			chartBlocks_.append(block);
			Messenger::printVerbose("Creating new ModuleBlock %p for Module %p (%s).\n", block, module, module->uniqueName());
		}
	}

	// Any widgets remaining in moduleBlockWidgets_ are no longer used, and can thus be deleted
	RefListIterator<ModuleBlock> widgetRemover(moduleBlockWidgets_);
	while (ModuleBlock* block = widgetRemover.iterate())
	{
		chartBlocks_.remove(block);
		delete block;
	}

	// Copy the new list
	moduleBlockWidgets_ = newWidgets;

	// Set the correct number of hotspots (number of block widgets + 1)
	int nHotSpots = moduleBlockWidgets_.nItems() + 1;
	while (nHotSpots < hotSpots_.nItems()) hotSpots_.removeLast();
	while (nHotSpots > hotSpots_.nItems()) hotSpots_.add();
}

// Set the currently-selected Module
void ModuleListChart::setCurrentModule(Module* module)
{
	if (!module) selectedBlock_ = NULL;
	else selectedBlock_ = moduleBlock(module);

	repaint();

	blockSelectionChanged(selectedBlock_);
}

// Return the currently-selected molecule
Module* ModuleListChart::currentModule() const
{
	if (!selectedBlock_) return NULL;

	// Cast selectedBlock_ up to a ModuleBlock
	ModuleBlock* moduleBlock = dynamic_cast<ModuleBlock*>(selectedBlock_);
	if (!moduleBlock) return NULL;

	return moduleBlock->module();
}

/*
 * Block Interaction
 */

// Return whether to accept the dragged object (described by its mime info)
bool ModuleListChart::acceptDraggedObject(const MimeStrings* strings)
{
	// Check the content of the strings
	if (strings->hasData(MimeString::ModuleType))
	{
		// We accept the drop of an object specifying a Module type - we'll create a new instance of this type in the list
		return true;
	}
	else if (strings->hasData(MimeString::LocalType))
	{
		// We accept the drop of a local block (a Module, referenced by its unique name) - we'll move it into the list
		return true;
	}

	return false;
}

// Handle hover over specified hotspot, returning whether layout update is required
bool ModuleListChart::handleHotSpotHover(const ChartHotSpot* hotSpot)
{
	// Need to recalculate widgets in order to display hotspot drop area
	return true;
}

// Handle the drop of an object (described by its mime info)
void ModuleListChart::handleDroppedObject(const MimeStrings* strings)
{
	// Check - if there is no current hotspot, then we have nothing to do
	if (!currentHotSpot_) return;

	if (strings->hasData(MimeString::LocalType))
	{
		// Local data - i.e. the dragged block which originated from this chart
		if (!draggedBlock_)
		{
			Messenger::error("Local data dropped, but no dragged block is set.\n");
			return;
		}

		// Cast the dragged block up to a ModuleBlock
		ModuleBlock* draggedModuleBlock = dynamic_cast<ModuleBlock*>(draggedBlock_);
		if (!draggedModuleBlock) return;

		// Get the Module associated to the dragged block
		Module* draggedModule = draggedModuleBlock->module();

		// Cast the blocks either side of the current hotspot up to ModuleBlocks, and get their Modules
		ModuleBlock* moduleBlockBefore = dynamic_cast<ModuleBlock*>(currentHotSpot_->blockBefore());
		Module* moduleBeforeHotSpot = (moduleBlockBefore ? moduleBlockBefore->module() : NULL);
		ModuleBlock* moduleBlockAfter = dynamic_cast<ModuleBlock*>(currentHotSpot_->blockAfter());
		Module* moduleAfterHotSpot = (moduleBlockAfter ? moduleBlockAfter->module() : NULL);

		// Check the blocks either side of the hotspot to see where our Module needs to be (or has been returned to)
		if ((draggedModule->prev() == moduleBeforeHotSpot) && (draggedModule->next() == moduleAfterHotSpot))
		{
			// Dragged block has not moved. Nothing to do.
			return;
		}
		else
		{
			// Block has been dragged to a new location...
			if (moduleBeforeHotSpot) moduleList_->modules().moveAfter(draggedModule, moduleBeforeHotSpot);
			else moduleList_->modules().moveBefore(draggedModule, moduleAfterHotSpot);

			// Flag that the current data has changed
			emit(dataModified());
		}
	}
	else if (strings->hasData(MimeString::ModuleType))
	{
		// Create a new instance of the specified module type
		Module* newModule = dissolve_.createModuleInstance(qPrintable(strings->data(MimeString::ModuleType)));

		// Cast the blocks either side of the current hotspot up to ModuleBlocks, and get their Modules
		ModuleBlock* moduleBlockBefore = dynamic_cast<ModuleBlock*>(currentHotSpot_->blockBefore());
		Module* moduleBeforeHotSpot = (moduleBlockBefore ? moduleBlockBefore->module() : NULL);
		ModuleBlock* moduleBlockAfter = dynamic_cast<ModuleBlock*>(currentHotSpot_->blockAfter());
		Module* moduleAfterHotSpot = (moduleBlockAfter ? moduleBlockAfter->module() : NULL);

		// Add the new modele 
		if (moduleAfterHotSpot) moduleList_->modules().ownBefore(newModule, moduleAfterHotSpot);
		else moduleList_->modules().own(newModule);

		newModule->setConfigurationLocal(localConfiguration_ != NULL);

		// Set Configuration targets as appropriate
		if (newModule->nRequiredTargets() != Module::ZeroTargets)
		{
			if (localConfiguration_) newModule->addTargetConfiguration(localConfiguration_);
			else newModule->addTargetConfigurations(dissolve_.configurations());
		}

		// Flag that the current data has changed
		emit(dataModified());
	}
}

// Return mime info for specified block (owned by this chart)
MimeStrings ModuleListChart::mimeInfo(ChartBlock* block)
{
	// Try to cast the block into a ModuleBlock
	ModuleBlock* moduleBlock = dynamic_cast<ModuleBlock*>(block);
	if (!moduleBlock) return MimeStrings();

	MimeStrings mimeStrings;
	mimeStrings.add(MimeString::LocalType, moduleBlock->module()->uniqueName());

	return mimeStrings;
}

// Specified block has been double clicked
void ModuleListChart::blockDoubleClicked(ChartBlock* block)
{
	// Cast block to a ModuleBlock
	ModuleBlock* moduleBlock = dynamic_cast<ModuleBlock*>(block);
	if (!moduleBlock) return;

	// Emit the relevant signal
	emit(ChartBase::blockDoubleClicked(moduleBlock->module()->uniqueName()));
}

// The chart has requested removal of one of its blocks
void ModuleListChart::blockRemovalRequested(const QString& blockIdentifier)
{
	// Get the reference to the Module list
	List<Module>& modules = moduleList_->modules();

	// Find the named Module in our list
	Module* module = moduleList_->find(qPrintable(blockIdentifier));
	if (!module)
	{
		Messenger::error("Can't find module to remove (%s) in our target list!\n", qPrintable(blockIdentifier));
		return;
	}

	// Are we sure that we want to delete the Module?
	QMessageBox queryBox;
	queryBox.setText(QString("This will delete the Module '%1'.").arg(blockIdentifier));
	queryBox.setInformativeText("This cannot be undone. Proceed?");
	queryBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	queryBox.setDefaultButton(QMessageBox::No);
	int ret = queryBox.exec();

	if (ret == QMessageBox::Yes)
	{
		modules.cut(module);

		// If the Module is currently displayed in its own ModuleTab, remove that first
		emit(blockRemoved(blockIdentifier));

		// If the module to delete is the currently-displayed one, unset it now
		if (module == currentModule()) setCurrentModule(NULL);

		// Remove the Module instance
		dissolve_.deleteModuleInstance(module);

		emit(dataModified());

		updateControls();
	}
}

// Block selection has changed
void ModuleListChart::blockSelectionChanged(ChartBlock* block)
{
	// If a NULL block pointer was provided there is no current selection
	if (!block)
	{
		emit(ChartBase::blockSelectionChanged(QString()));
		return;
	}

	// Cast block to a ModuleBlock
	ModuleBlock* moduleBlock = dynamic_cast<ModuleBlock*>(block);
	if (!moduleBlock) return;

	// Emit the relevant signal
	emit(ChartBase::blockSelectionChanged(moduleBlock->module()->uniqueName()));
}

/*
 * Widget Layout
*/

// Calculate new widget geometry according to the layout requirements
QSize ModuleListChart::calculateNewWidgetGeometry(QSize currentSize)
{
	/*
	 * ModuleList layout is a single vertical column.
	 */

	// Create a metrics object
	ModuleListChartMetrics metrics;

	// Left edge of next widget, and maximum height
	int top = metrics.chartMargin();
	int hotSpotTop = 0;
	int maxWidth = 0;

	// Get the first hot spot in the list (the list should have been made the correct size in updateContentBlocks()).
	ChartHotSpot* hotSpot = hotSpots_.first();

	// Loop over widgets
	ModuleBlock* lastVisibleBlock = NULL;
	RefListIterator<ModuleBlock> blockIterator(moduleBlockWidgets_);
	while (ModuleBlock* block = blockIterator.iterate())
	{
		// Set default visibility of the block
		block->setVisible(true);

		// If this block is currently being dragged, hide it and continue with the next one
		if (draggedBlock_ == block)
		{
			block->setVisible(false);
			continue;
		}

		// If our hotspot is the current one, increase the size.
		if (hotSpot == currentHotSpot_) top += metrics.verticalInsertionSpacing();

		// Set top edge of this widget
		block->setNewPosition(metrics.chartMargin(), top);

		// Try to give our block its preferred (minimum) size
		QSize minSize = block->widget()->minimumSizeHint();
		block->setNewSize(minSize.width(), minSize.height());

		// Set the hotspot to end at the left edge of the current block
		hotSpot->setGeometry(QRect(0, hotSpotTop, width(), top - hotSpotTop));

		// Set surrounding blocks for the hotspot
		hotSpot->setSurroundingBlocks(lastVisibleBlock, block);

		// Set new hotspot top edge
		hotSpotTop = top + minSize.height();

		// Add on height of widget and spacing to top edge
		top += minSize.height();
		if (!blockIterator.isLast()) top += metrics.verticalModuleSpacing();

		// Check maximal width
		if (minSize.width() > maxWidth) maxWidth = minSize.width();

		// Set the last visible block
		lastVisibleBlock = block;

		// Move to the next hotspot
		hotSpot = hotSpot->next();
	}

	// Finalise required size
	QSize requiredSize = QSize(maxWidth + 2*metrics.chartMargin(), top + metrics.chartMargin());

	// Set final hotspot geometry
	hotSpot->setGeometry(QRect(0, hotSpotTop, width(), height() - hotSpotTop));
	hotSpot->setSurroundingBlocks(lastVisibleBlock, NULL);
	hotSpot = hotSpot->next();

	// Set the correct heights for all hotspots up to the current one - any after that are not required and will have zero height
// 	for (ChartHotSpot* spot = hotSpots_.first(); spot != hotSpot; spot = spot->next()) spot->setWidth(maxWidth);
	for (ChartHotSpot* spot = hotSpot; spot != NULL; spot = spot->next()) spot->setHeight(0);

	// If there is a current hotspot, set the insertion widget to be visible and set its geometry
	if (currentHotSpot_)
	{
		insertionBlock_->setVisible(true);

		insertionBlock_->setGeometry(currentHotSpot_->geometry());
	}
	else insertionBlock_->setVisible(false);

	// Return required size
	return requiredSize;
}

/*
 * State I/O
 */

// Write widget state through specified LineParser
bool ModuleListChart::writeState(LineParser& parser) const
{
	return true;
}

// Read widget state through specified LineParser
bool ModuleListChart::readState(LineParser& parser)
{
	return true;
}
