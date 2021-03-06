/*
	*** Mathematical Expression Generator
	*** src/expression/generator.cpp
	Copyright T. Youngs 2015-2020

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

#include "expression/generator.h"
#include "expression/expression.h"
#include "expression/generator_grammar.hh"
#include "base/sysfunc.h"
#include "base/messenger.h"
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// Static members
Expression* ExpressionGenerator::expression_ = NULL;
ExpressionGenerator* ExpressionGenerator::generator_ = NULL;

// Constructors
ExpressionGenerator::ExpressionGenerator(Expression& expression, const char* expressionText)
{
	// Private variables
	useAdditionalConstants_ = false;

	expression_ = &expression;
	generator_ = this;

	// Initialise Expression and generator
	expression_->clear();
	setSource(expressionText);
}

// Destructor
ExpressionGenerator::~ExpressionGenerator()
{
}

/*
 * Target Expression and Generator
 */

// Return target Expression (static to allow ExpressionGenerator_parse() to use it)
Expression* ExpressionGenerator::expression()
{
	return expression_;
}

// Return current ExpressionGenerator (static to allow ExpressionGenerator_parse() to use it)
ExpressionGenerator* ExpressionGenerator::generator()
{
	return generator_;
}

/*
 * Lexer
 */

// Return enum options for SymbolToken
EnumOptions<int> ExpressionGenerator::symbolTokens()
{
	static EnumOptionsList SymbolTokenOptions = EnumOptionsList() <<
		EnumOption(DISSOLVE_EXPR_EQ,		"==") <<
		EnumOption(DISSOLVE_EXPR_GEQ,		">=") <<
		EnumOption(DISSOLVE_EXPR_LEQ,		"<=") <<
		EnumOption(DISSOLVE_EXPR_NEQ,		"!=") <<
		EnumOption(DISSOLVE_EXPR_AND,		"&&") <<
		EnumOption(DISSOLVE_EXPR_OR,		"||");
	
	static EnumOptions<int> options("SymbolToken", SymbolTokenOptions);

	return options;
}

// Set string source for lexer
void ExpressionGenerator::setSource(const char* expressionText)
{
	// Set parsing source, always ensuring that we have a terminating ';'
	expressionString_ = expressionText;
	expressionString_ += ';';

	stringPos_ = 0;
	stringLength_ = strlen(expressionString_);
	tokenStart_ = 0;
	functionStart_ = -1;

	Messenger::printVerbose("Parser source string is '%s', length is %i\n", expressionString_.get(), stringLength_);
}

// Get next character from current input stream
char ExpressionGenerator::getChar()
{
	char c = 0;

	// Are we at the end of the current string?
	if (stringPos_ == stringLength_) return 0;

	// Return current char
	c = expressionString_[stringPos_];
	stringPos_++;
	return c;
}

// Peek next character from current input stream
char ExpressionGenerator::peekChar()
{
	return (stringPos_ == stringLength_ ? 0 : expressionString_[stringPos_]);
}

// 'Replace' last character read from current input stream
void ExpressionGenerator::unGetChar()
{
	--stringPos_;
}


// Bison-required ExpressionGenerator_lex()
int ExpressionGenerator_lex()
{
	if (!ExpressionGenerator::expression()) return 0;
	if (!ExpressionGenerator::generator()) return 0;
	return ExpressionGenerator::generator()->lex();
}

// Parser lexer, called by yylex()
int ExpressionGenerator::lex()
{
	int n;
	bool done, hasExp;
	static CharString token, name;
	char c;
	token.clear();

	// Skip over whitespace
	while ((c = getChar()) == ' ' || c == '\t' || c == '\r' || c == '\n' );

	if (c == 0) return 0;

	// Set this point as the start of our new token (for error reporting)
	tokenStart_ = stringPos_-1;

	/*
	 * Number Detection - Either '.' or a digit begins a number
	 */
	if (c == '.' || isdigit(c))
	{
		Messenger::printVerbose("LEXER (%p): found the start of a number...\n", this);
		bool isInteger = (c != '.');
		hasExp = false;
		token += c;
		done = false;
		do
		{
			c = getChar();
			if (isdigit(c)) token += c;
			else if (c == '.')
			{
				token += '.';
				isInteger = false;
			}
			else if ((c == 'e') || (c == 'E'))
			{
				// Check for previous exponential in number
				if (hasExp)
				{
					printf("Error: Number has two exponentiations (e/E).\n");
					return 0;
				}
				token += 'E';
				hasExp = true;
			}
			else if ((c == '-') || (c == '+'))
			{
				// We allow '-' or '+' only as part of an exponentiation, so if it is not preceeded by 'E' we stop parsing
				if ((!token.isEmpty()) && (!(token.lastChar() == 'E')))
				{
					unGetChar();
					done = true;
				}
				else token += c;
			}
			else
			{
				unGetChar();
				done = true;
			}
		} while (!done);

		// We now have the number as a text token...
		if (isInteger)
		{
			ExpressionGenerator_lval.integerConst = token.asInteger();
			Messenger::printVerbose("LEXER (%p): found an integer constant [%s] [%i]\n", this, token.get(), ExpressionGenerator_lval.integerConst);
			return DISSOLVE_EXPR_INTEGERCONSTANT;
		}
		else
		{
			ExpressionGenerator_lval.doubleConst = token.asDouble();
			Messenger::printVerbose("LEXER (%p): found a double constant [%s] [%e]\n", this, token.get(), ExpressionGenerator_lval.doubleConst);
			return DISSOLVE_EXPR_DOUBLECONSTANT;
		}
	}

	/*
	 * Alpha-token - function or variable
	 */
	if (isalpha (c))
	{
		do
		{
			token += c;
			c = getChar();
		}
		while (isalnum(c) || (c == '_'));
		unGetChar();
		Messenger::printVerbose("LEXER (%p): found an alpha token [%s]...\n", this, token.get());

		// Built-in numeric constants
		if (token == "Pi")
		{
			ExpressionGenerator_lval.doubleConst = PI;
			return DISSOLVE_EXPR_DOUBLECONSTANT;
		}

		// Additional numeric constants
		if (useAdditionalConstants_)
		{
			if (token == "DEGRAD")
			{
				ExpressionGenerator_lval.doubleConst = DEGRAD;
				return DISSOLVE_EXPR_DOUBLECONSTANT;
			}
			else if (token == "Bohr")
			{
				ExpressionGenerator_lval.doubleConst = BOHRRADIUS;
				return DISSOLVE_EXPR_DOUBLECONSTANT;
			}
			else if (token == "NA")
			{
				ExpressionGenerator_lval.doubleConst = AVOGADRO;
				return DISSOLVE_EXPR_DOUBLECONSTANT;
			}
			else if (token == "c")
			{
				ExpressionGenerator_lval.doubleConst = SPEEDOFLIGHT;
				return DISSOLVE_EXPR_DOUBLECONSTANT;
			}
			else if (token == "kb")
			{
				ExpressionGenerator_lval.doubleConst = BOLTZMANN;
				return DISSOLVE_EXPR_DOUBLECONSTANT;
			}
			else if (token == "h")
			{
				ExpressionGenerator_lval.doubleConst = PLANCK;
				return DISSOLVE_EXPR_DOUBLECONSTANT;
			}
			else if (token == "hbar")
			{
				ExpressionGenerator_lval.doubleConst = HBAR;
				return DISSOLVE_EXPR_DOUBLECONSTANT;
			}
		}

		// Is this a recognised high-level keyword?
		n = 0;
		if (token == "if") n = DISSOLVE_EXPR_IF;
		else if (token == "else") n = DISSOLVE_EXPR_ELSE;
		if (n != 0)
		{
			Messenger::printVerbose("LEXER (%p): ...which is a high-level keyword (%i)\n",this,n);
			return n;
		}

		// Is it an existing variable?
		ExpressionVariable* v = expression_->variable(token);
		if (v != NULL)
		{
			Messenger::printVerbose("LEXER (%p): ...which is an existing variable (->VAR)\n", this);
			ExpressionGenerator_lval.variable = v;
			return DISSOLVE_EXPR_VAR;
		}

		// Is it a known function keyword?
		n = ExpressionFunctions::function(token.get());
		if (n != ExpressionFunctions::nFunctions)
		{
			Messenger::printVerbose("LEXER (%p): ... which is a function (->FUNCCALL).\n", this);
			ExpressionGenerator_lval.functionId = n;
			functionStart_ = tokenStart_;
			return DISSOLVE_EXPR_FUNCCALL;
		}

		// If we get to here then we have found an unrecognised alphanumeric token
		Messenger::printVerbose("LEXER (%p): ...which is unrecognised (->NEWTOKEN)\n", this);
		name = token;
		ExpressionGenerator_lval.name = &name;
		return DISSOLVE_EXPR_NEWTOKEN;
	}

	// We have found a symbolic character (or a pair) that corresponds to an operator
	// Return immediately in the case of brackets, comma, and semicolon
	if ((c == '(') || (c == ')') || (c == ';') || (c == ',') || (c == '{') || (c == '}') || (c == '[') || (c == ']') || (c == '%') || (c == ':'))
	{
		Messenger::printVerbose("LEXER (%p): found symbol [%c]\n",this,c);
		return c;
	}
	token += c;

	// Similarly, if the next character is a period, bracket or double quotes, return immediately
	char c2 = peekChar();
	if ((c2 == '.') || (c2 == '(') || (c2 == ')') || (c2 == ';') || (c2 == '{') || (c2 == '}') || (c2 == '"')) return c;

	// If next character is '-', return now if previous char was *not* another '-'
	if ((c2 == '-') && (c != '-')) return c;

	// If it is 'punctuation', add this second character to our operator and search for it
	if (ispunct(c2))
	{
		c = getChar();
		token += c;
		Messenger::printVerbose("LEXER (%p): found symbol [%s]\n", this, token.get());
		if (symbolTokens().isValid(token.get())) return symbolTokens().enumeration(token.get());
		else Messenger::error("Unrecognised symbol '%s' found in input.\n", token.get());
 	}
	else
	{
		// Make sure that this is a known symbol
		if ((c == '$') || (c == '%') || (c == '&') || (c == '@') || (c == '?') || (c == ':'))
		{
			printf("Error: Unrecognised symbol found in input (%c).\n", c);
		}
		else return c;
	}

	return 0;
}

/*
 * Creation
 */

// Static generation functions
bool ExpressionGenerator::generate(Expression& expression)
{
	// Create a generator
	ExpressionGenerator generator(expression, expression.expressionString());

	// Generate expression
	bool result = ExpressionGenerator_parse() == 0;
	if (!result) expression_->clear();

	return result;
}

bool ExpressionGenerator::generate(Expression& expression, RefList<ExpressionVariable> externalVariables)
{
	// Create a generator
	ExpressionGenerator generator(expression, expression.expressionString());

	// Set the external variable source
	expression.setExternalVariables(externalVariables);

	// Generate expression
	bool result = ExpressionGenerator_parse() == 0;
	if (!result) expression_->clear();

	return result;
}
