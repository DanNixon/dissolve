/*
	*** Keyword Widget - Vec3NodeValue
	*** src/gui/keywordwidgets/vec3nodevalue.h
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

#ifndef DISSOLVE_KEYWORDWIDGET_VEC3NODEVALUE_H
#define DISSOLVE_KEYWORDWIDGET_VEC3NODEVALUE_H

#include "gui/keywordwidgets/ui_vec3nodevalue.h"
#include "gui/keywordwidgets/base.h"
#include "keywords/vec3nodevalue.h"
#include <QWidget>

// Forward Declarations
/* none */

class Vec3NodeValueKeywordWidget : public QWidget, public KeywordWidgetBase
{
	// All Qt declarations must include this macro
	Q_OBJECT

	public:
	// Constructor
	Vec3NodeValueKeywordWidget(QWidget* parent, KeywordBase* keyword, const CoreData& coreData);


	/*
	 * Keyword
	 */
	private:
	// Associated keyword
	Vec3NodeValueKeyword* keyword_;


	/*
	 * Widgets
	 */
	private:
	// Main form declaration
	Ui::Vec3NodeValueWidget ui_;

	private slots:
	// Values edited
	void on_ValueAEdit_editingFinished();
	void on_ValueAEdit_returnPressed();
	void on_ValueBEdit_editingFinished();
	void on_ValueBEdit_returnPressed();
	void on_ValueCEdit_editingFinished();
	void on_ValueCEdit_returnPressed();

	signals:
	// Keyword value changed
	void keywordValueChanged(int flags);


	/*
	 * Update
	 */
	public:
	// Update value displayed in widget
	void updateValue();
};

#endif
