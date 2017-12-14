/*
	*** Module Widget
	*** src/modules/energy/gui/modulewidget.h
	Copyright T. Youngs 2007-2017

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

#ifndef DUQ_ENERGYMODULEWIDGET_H
#define DUQ_ENERGYMODULEWIDGET_H

#include "modules/energy/gui/ui_modulewidget.h"
#include "gui/modulewidget.h"

// Forward Declarations
class DUQ;
class Module;
class PartialSet;
class EnergyModule;
class UChromaViewWidget;

// Module Widget
class EnergyModuleWidget : public ModuleWidget
{
	// All Qt declarations derived from QObject must include this macro
	Q_OBJECT

	private:
	// Associated Module
	EnergyModule* module_;
	// UChromaViews contained within this widget
	UChromaViewWidget* energyGraph_;
	// Reference to DUQ
	DUQ& dUQ_;

	public:
	// Constructor / Destructor
	EnergyModuleWidget(QWidget* parent, Module* module, DUQ& dUQ);
	~EnergyModuleWidget();
	// Main form declaration
	Ui::EnergyModuleWidget ui;
	// Update controls within widget
	void updateControls();
	// Initialise controls
	void initialiseControls(EnergyModule* module);

	/*
	 * ModuleWidget Implementations
	 */
	public:
	// Write widget state through specified LineParser
	bool writeState(LineParser& parser);
	// Read widget state through specified LineParser
	bool readState(LineParser& parser);


	/*
	 * Widgets / Functions
	 */
	private:

	private slots:
	void on_TargetCombo_currentIndexChanged(int index);
};

#endif
