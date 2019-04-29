/*
	*** Filters
	*** src/math/filters.h
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

#ifndef DISSOLVE_FILTERS_H
#define DISSOLVE_FILTERS_H

#include "math/broadeningfunction.h"

// Forward Declarations
class Data1D;

// Filters
class Filters
{
	public:
	// Perform point-wise convolution of data with the supplied BroadeningFunction
	static void convolve(Data1D& data, const BroadeningFunction& function);
	// Perform point-wise convolution of data with the supplied BroadeningFunction, normalising to the original integral of the function
	static void convolveNormalised(Data1D& data, const BroadeningFunction& function);
	// Apply Kolmogorov–Zurbenko filter to data
	static void kolmogorovZurbenko(Data1D& data, int k, int m);
	// Apply median filter to data
	static void median(Data1D& data, int length);
	// Perform moving average smoothing on data
	static void movingAverage(Data1D& data, int avgSize);
	// Subtract average level (starting at supplied x value) from data
	static double subtractAverage(Data1D& data, double xStart);
	// Trim supplied data to specified range
	static void trim(Data1D& data, double xMin, double xMax, bool interpolateEnds = false, double interpolationThreshold = 0.01);
};

#endif
