/*
	*** Pair Broadening Function
	*** src/math/pairbroadeningfunction.h
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

#ifndef DISSOLVE_PAIRBROADENINGFUNCTION_H
#define DISSOLVE_PAIRBROADENINGFUNCTION_H

#include "base/charstring.h"
#include "genericitems/base.h"
#include "math/broadeningfunction.h"
#include "templates/array.h"
#include "templates/array2d.h"

// Forward Declarations
class AtomType;
class LineParser;
class ProcessPool;
class SpeciesIntra;

// Pair Broadening Function
class PairBroadeningFunction : public GenericItemBase
{
	public:
	// Function Types
	enum FunctionType { NoFunction, GaussianFunction, GaussianElementPairFunction, FrequencyFunction, nFunctionTypes };
	// Return FunctionType from supplied string
	static FunctionType functionType(const char* s);
	// Return FunctionType name
	static const char* functionType(FunctionType func);
	// Return number of parameters needed to define FunctionType
	static int nFunctionParameters(FunctionType func);

	public:
	// Constructor
	PairBroadeningFunction(FunctionType function = NoFunction);
	// Destructor
	~PairBroadeningFunction();
	// Copy Constructor
	PairBroadeningFunction(const PairBroadeningFunction& source);
	// Assignment Operator
	void operator=(const PairBroadeningFunction& source);


	/*
	 * Function Data
	 */
	private:
	// Function Type
	FunctionType function_;
	// Gaussian FWHM parameter
	double gaussianFWHM_;
	// Elemental pair Gaussian FWHM parameters
	Array2D<double> elementPairGaussianFWHM_;
	// Elemental pair flags (whether a valid value exists)
	Array2D<bool> elementPairGaussianFlags_;
	// Frequency-based bond broadening constant
	double frequencyBondConstant_;
	// Frequency-based angle broadening constant
	double frequencyAngleConstant_;

	public:
	// Read function data from LineParser source
	bool readAsKeyword(LineParser& parser, int startArg, const CoreData& coreData);
	// Write function data to LineParser source
	bool writeAsKeyword(LineParser& parser, const char* prefix, bool writeBlockMarker = true);
	// Set function type
	void setFunction(FunctionType function);
	// Return function type
	FunctionType function() const;
	// Set Gaussian FWHM parameters
	void setGaussianFWHM(double fwhm);
	// Return Gaussian FWHM parameter
	double gaussianFWHM() const;
	// Set frequency bond constant
	void setFrequencyBondConstant(double k);
	// Return frequency bond constant
	double frequencyBondConstant() const;
	// Set frequency angle constant
	void setFrequencyAngleConstant(double k);
	// Return frequency angle constant
	double frequencyAngleConstant() const;
	// Return array of pointers to all adjustable parameters
	Array<double*> parameters();
	// Return short summary of function and its parameters
	CharString summary() const;
	// Return a BroadeningFunction tailored to the specified AtomType pair
	BroadeningFunction broadeningFunction(AtomType* at1, AtomType* at2, SpeciesIntra* intra = NULL);


	/*
	 * GenericItemBase Implementations
	 */
	public:
	// Return class name
	static const char* itemClassName();
	// Read data through specified LineParser
	bool read(LineParser& parser, const CoreData& coreData);
	// Write data through specified LineParser
	bool write(LineParser& parser);


	/*
	 * Parallel Comms
	 */
	public:
	// Broadcast data from Master to all Slaves
	bool broadcast(ProcessPool& procPool, const int root, const CoreData& coreData);
	// Check item equality
	bool equality(ProcessPool& procPool);
};

#endif

