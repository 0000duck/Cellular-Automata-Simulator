#pragma once

#include <string>

using namespace std;

enum RuleType { INVALID_RULE, LIFE, WOLFRAM };

class Rule
{
protected:
	string ruleString;

	//this variable stores the type of the current rule string
	//the character used by evaluateRuleType() to identify a rule string is deleted once the rule enters use
	RuleType currentRuleType = INVALID_RULE;
public:
	Rule(string ruleString)
	{
		this->ruleString = ruleString;
		currentRuleType = evaluateRuleType(ruleString);
	}
	static RuleType evaluateRuleType(string ruleString) //static function used to check what family of rule a given rule string belongs to
	{
		if (ruleString[0] == 'B' || ruleString[0] == 'b')
			return LIFE;
		else if (ruleString[0] == 'W' || ruleString[0] == 'w')
			return WOLFRAM;
		else
			return INVALID_RULE;
	}
	RuleType getRuleType(void) //returns rule type currently in use by this object
	{
		return currentRuleType;
	}
};

class LifeRule : public Rule
{
private:
	const static int SIZE = 10;

	//arrays store the numbers of neighbours necessary for a dead cell to be born, and for a living cell to survive
	char birthRule[SIZE];
	char survivalRule[SIZE];
public:
	LifeRule(string rule = "B3/S23") : Rule(rule)
	{
		//set every element in both arrays to 0
		for (int i = 0; i < SIZE; i++)
		{
			birthRule[i] = 0;
			survivalRule[i] = 0;
		}

		ruleString = rule;
		ruleString += '\n';

		try
		{
			int count = 0;
			int birthRuleIndex = 0;
			int survivalRuleIndex = 0;

			while (count < min(ruleString.length(), SIZE) && ruleString[count] != '/')
			{
				if (ruleString[count] != 'B' && ruleString[count] != 'b')
				{
					birthRule[birthRuleIndex] = ruleString[count] - '0';
					birthRuleIndex++;
				}
				count++;
			}

			//increment count past '/' char to start of survival segment of rule
			count++;

			while (count < min(ruleString.length(), SIZE) && ruleString[count] != '\n')
			{
				if (ruleString[count] != 'S' && ruleString[count] != 's')
				{
					survivalRule[survivalRuleIndex] = ruleString[count] - '0';
					survivalRuleIndex++;
				}
				count++;
			}

			birthRule[birthRuleIndex] = '\n';
			survivalRule[survivalRuleIndex] = '\n';

			//validate extracted rule
			count = 0;

			while (count < SIZE && birthRule[count] != '\n' && survivalRule[count] != '\n')
			{
				if (birthRule[count] >= SIZE || birthRule[count] < 0 || survivalRule[count] >= SIZE || survivalRule[count] < 0)
					throw INVALID_RULE;

				count++;
			}
		}
		//if rule string is invalid, default to conway's game of life
		catch (...)
		{
			ruleString = "B3/S23\n";

			birthRule[0] = 3;
			birthRule[1] = '\n';

			survivalRule[0] = 2;
			survivalRule[1] = 3;
			survivalRule[2] = '\n';
		}
	}
	const char* getBirthRule()
	{
		return birthRule;
	}
	const char* getSurvivalRule()
	{
		return survivalRule;
	}
};

class WolframRule : public Rule
{
private:
	const static int SIZE = 8;
	bool rule[8];
public:
	WolframRule(string ruleString = "W255") : Rule(ruleString)
	{
		//attempt to convert rule string to an int
		try
		{
			this->ruleString = ruleString;

			//delete "W" char from start of string
			this->ruleString.erase(0, 1);

			//convert rule string to an integer
			//store it as an unsigned char (guaranteed to be 1 byte, standard number representation) because bitwise operations will be performed on it
			unsigned char number = (char)stoi(this->ruleString);

			//assign the smaller of the converted rule string or the max number than can be held by an unsigned char (255) to "rule"
			number = min(255, number);

			//split number up into individual bits, each of which will be placed into an array element
			//number is stored reversed because the number representing the neighbour setup is used as an index into rule array when updating the CA
			for (int i = 0; i < SIZE; i++)
				rule[i] = number & (1 << i); //get the value of a bit at a given position by ANDing it with 0b1, 0b10, 0b100, 0b1000 etc.
		}
		//if rule string is invalid, default to rule 30 instead
		catch (...)
		{
			//assign char equivalent of 30 to number variable (storing 30 directly in number will not work as char can only treat single digits numerically)
			unsigned char number = (char)30;

			for (int i = 0; i < SIZE; i++)
				rule[i] = number & (1 << i);
		}
	}
	const bool* getRule(void)
	{
		return rule;
	}
};