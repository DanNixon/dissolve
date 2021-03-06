/*
	*** ExponentialSpin Functions
	*** src/gui/widgets/exponentialspin_funcs.cpp
	Copyright T. Youngs 2016-2020

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

#include "gui/widgets/exponentialspin.hui"
#include "math/doubleexp.h"
#include <QLineEdit>

// Constructor
ExponentialSpin::ExponentialSpin(QWidget* parent) : QDoubleSpinBox(parent)
{
	// Set up validator
	validator_.setNotation(QDoubleValidator::ScientificNotation);
	lineEdit()->setValidator(&validator_);

	// Set local values
	limitMinValue_ = false;
	limitMaxValue_ = false;

	// Set QDoubleSpinBox limits to be present but effectively 'off', and number of decimals
	setMinimum(-1.0e99);
	setMaximum(1.0e99);
	setDecimals(3);
}

/*
 * Data
 */

// Set minimum limit
void ExponentialSpin::setMinimumLimit(double value)
{
	limitMinValue_ = true;

	setMinimum(value);
}

// Set minimum limit
void ExponentialSpin::setMaximumLimit(double value)
{
	limitMaxValue_ = true;

	setMaximum(value);
}

// Remove range limits
void ExponentialSpin::removeLimits()
{
	limitMinValue_ = false;
	limitMaxValue_ = false;
}

/*
 * Reimplementations
 */

// Convert supplied value to text
QString ExponentialSpin::textFromValue(double value) const
{
	const int exponentFormatThreshold = 3;
// 	printf("Here we are in updateText, setting [%s].\n", qPrintable(value_.text(precision)));
	DoubleExp de(value);
	return de.asString(exponentFormatThreshold, decimals()).get();
}

// Validate supplied text
QValidator::State ExponentialSpin::validate(QString& text, int& pos) const
{
	// Set validator
	static QRegExp regExp("[-+]?[0-9]*[.]?[0-9]+([eE][-+]?[0-9]+)?");
	return (regExp.exactMatch(text) ? QValidator::Acceptable : QValidator::Invalid);
}

// Interpret text into value
double ExponentialSpin::valueFromText(const QString& text) const
{
	return text.toDouble();
}

// Step value by specified amount
// void ExponentialSpin::stepBy(int nSteps)
// {
// 	value_ = value_.value() + valueStep_*nSteps;
// 
// 	// Check new value and update text
// 	clamp();
// 	updateTextFromValue();
// 	emit(valueChanged(value_.value()));
// 	textChanged_ = false;
// }

// // Return which steps should be enabled
// QAbstractSpinBox::StepEnabled ExponentialSpin::stepEnabled() const
// {
// 	bool up = (value_.value() < valueMax_) || (!limitMaxValue_);
// 	bool down = (value_.value() > valueMin_) || (!limitMinValue_);
// 
// 	if (up && down) return (QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled);
// 	else if (up) return QAbstractSpinBox::StepUpEnabled;
// 	else if (down) return QAbstractSpinBox::StepDownEnabled;
// 	else return QAbstractSpinBox::StepNone;
// }
