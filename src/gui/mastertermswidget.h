/*
	*** MasterTerms Widget
	*** src/gui/mastertermswidget.h
	Copyright T. Youngs 2012-2018

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

#ifndef DUQ_MASTERTERMSWIDGET_H
#define DUQ_MASTERTERMSWIDGET_H

#include "gui/ui_mastertermswidget.h"
#include "gui/subwidget.h"

// Forward Declarations
class DUQ;
class MasterIntra;

// MasterTerms Widget
class MasterTermsWidget : public SubWidget
{
	// All Qt declarations derived from QObject must include this macro
	Q_OBJECT

	private:
	// Reference to dUQ
	DUQ& duq_;


	public:
	// Constructor / Destructor
	MasterTermsWidget(QWidget* parent, DUQ& dUQ);
	~MasterTermsWidget();
	// Main form declaration
	Ui::MasterTermsWidget ui;


	/*
	 * SubWidget Reimplementations / Definitions
	 */
	protected:
	void closeEvent(QCloseEvent* event);

	public:
	// Update controls within widget
	void updateControls();
	// Disable sensitive controls within widget, ready for main code to run
	void disableSensitiveControls();
	// Enable sensitive controls within widget, ready for main code to run
	void enableSensitiveControls();
	// Return string specifying widget type
	const char* widgetType();
	// Write widget state through specified LineParser
	bool writeState(LineParser& parser);
	// Read widget state through specified LineParser
	bool readState(LineParser& parser);


	/*
	 * Widget Functions, Signals, and Slots
	 */
	private:
	// Row update function for BondsTable
	void updateBondsTableRow(int row, MasterIntra* masterBond, bool createItems);
	// Row update function for AnglesTable
	void updateAnglesTableRow(int row, MasterIntra* masterAngle, bool createItems);
	// Row update function for TorsionsTable
	void updateTorsionsTableRow(int row, MasterIntra* masterTorsion, bool createItems);

	signals:
	void windowClosed(void*);

	private slots:
        void bondFunctionChanged(int index);
        void on_BondsTable_itemChanged(QTableWidgetItem* w);
	void on_NewBondTermButton_clicked(bool checked);
	void on_RemoveBondTermButton_clicked(bool checked);
        void angleFunctionChanged(int index);
        void on_AnglesTable_itemChanged(QTableWidgetItem* w);
	void on_NewAngleTermButton_clicked(bool checked);
	void on_RemoveAngleTermButton_clicked(bool checked);
        void torsionFunctionChanged(int index);
        void on_TorsionsTable_itemChanged(QTableWidgetItem* w);
	void on_NewTorsionTermButton_clicked(bool checked);
	void on_RemoveTorsionTermButton_clicked(bool checked);
};

#endif
