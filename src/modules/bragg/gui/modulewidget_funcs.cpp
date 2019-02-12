/*
	*** Bragg Module Widget - Functions
	*** src/modules/bragg/gui/modulewidget_funcs.cpp
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

#include "modules/bragg/gui/modulewidget.h"
#include "gui/viewer/dataviewer.hui"
#include "gui/widgets/mimetreewidgetitem.h"
#include "main/dissolve.h"
#include "classes/atomtype.h"
#include "templates/variantpointer.h"
#include "templates/genericlisthelper.h"

// Constructor
BraggModuleWidget::BraggModuleWidget(QWidget* parent, Module* module, Dissolve& dissolve) : ModuleWidget(parent), module_((BraggModule*) module), dissolve_(dissolve)
{
	// Set up user interface
	ui.setupUi(this);

	// Grab our DataViewer widget
	dataView_ = ui.PlotWidget->dataViewer();

	// Start a new, empty session
	dataView_->startNewSession(true);

	// Set up the view pane
	View& view = dataView_->view();
	view.setViewType(View::FlatXYView);
	view.axes().setTitle(0, "\\it{r}, \\sym{angstrom}");
	view.axes().setMax(0, 10.0);
	view.axes().setTitle(1, "g(r) / S(Q)");
	view.axes().setMin(1, -1.0);
	view.axes().setMax(1, 1.0);

	refreshing_ = false;
}

BraggModuleWidget::~BraggModuleWidget()
{
}

// Update controls within widget
void BraggModuleWidget::updateControls()
{
	// Ensure that any displayed data are up-to-date
	dataView_->refreshReferencedDataSets();

	dataView_->postRedisplay();
}

// Disable sensitive controls within widget, ready for main code to run
void BraggModuleWidget::disableSensitiveControls()
{
}

// Enable sensitive controls within widget, ready for main code to run
void BraggModuleWidget::enableSensitiveControls()
{
}

/*
 * ModuleWidget Implementations
 */

// Write widget state through specified LineParser
bool BraggModuleWidget::writeState(LineParser& parser)
{
	// Write DataViewer session
	if (!dataView_->writeSession(parser)) return false;

	return true;
}

// Read widget state through specified LineParser
bool BraggModuleWidget::readState(LineParser& parser)
{
	// Read DataViewer session
	if (!dataView_->readSession(parser)) return false;
	
	return true;
}

/*
 * Widgets / Functions
 */

