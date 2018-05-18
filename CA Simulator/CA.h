#pragma once

#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include "Grid.h"
#include "Rule.h"

using namespace std;

class CA
{
protected:
	//pointers to Grid object (for storing cell state and grid position data, and drawing the grid) and Rule object (for storing and evaluating rule strings selected by the user)
	Grid* grid;
	Rule* rule;

	string ruleString;

	int generation;
	bool paused;

	float timeStep;
public:
	CA()
	{
		grid = new Grid;
		rule = nullptr;

		ruleString = "\n";
		generation = 0;
		paused = false;

		//set initial CA update time step to 500 milliseconds
		timeStep = 500;
	}
	CA(int width, int height)
	{
		grid = new Grid(width, height);
		rule = nullptr;

		ruleString = "\n";
		generation = 0;
		paused = false;

		//set initial CA update time step to 500 milliseconds
		timeStep = 500;
	}
	~CA()
	{
		if (grid)
		{
			delete grid;
			grid = nullptr;
		}

		//if there is an existing rule associated with CA, deallocate memory used for it
		if (rule)
		{
			delete rule;
			rule = nullptr;
		}
	}
	void pause(bool value)
	{
		//set value of pause flag to true or false, whichever user specifies
		paused = value;
	}
	bool isPaused(void)
	{
		return paused;
	}

	RuleType getRuleType()
	{
		if (rule != nullptr)
			return rule->getRuleType();
		else
			return INVALID_RULE;
	}
	virtual void setNewRule(string ruleString) = 0;

	virtual char getCellNeighbours(int x, int y) = 0;

	int getGridWidth(void)
	{
		return grid->getGridWidth();
	}
	int getGridHeight(void)
	{
		return grid->getGridHeight();
	}
	int findGridXCoord(double column)
	{
		return grid->findGridXCoord(column);
	}
	int findGridYCoord(double row)
	{
		return grid->findGridXCoord(row);
	}

	float getTimeStep(void)
	{
		return timeStep;
	}
	void setTimeStep(float timeStep)
	{
		this->timeStep = timeStep;
	}
	void step(void)
	{
		//unpause automata so that it will update when drawGrid() is called
		paused = false;

		//update and draw
		drawGrid();

		//pause automata so user can view next generation
		paused = true;
	}

	void editGridCell(int x, int y)
	{
		grid->editCell(x, y);
	}
	virtual void drawGrid() = 0;
	void resetAutomata(void)
	{
		generation = 0;
		grid->clear();
	}
};

class LifeCA : public CA
{
private:
	int neighbourCount;
public:
	LifeCA(string ruleString = "B3/S23") : CA()
	{
		this->ruleString = ruleString + '\n';
		this->rule = new LifeRule(this->ruleString);
	}
	~LifeCA()
	{
		if (rule)
		{
			delete rule;
			rule = nullptr;
		}
	}
	void setNewRule(string ruleString)
	{
		if (rule)
			delete rule;

		this->ruleString = ruleString + '\n';

		rule = new LifeRule(this->ruleString);

		generation = 0;
	}
	char getCellNeighbours(int x, int y) //not currently used, but may be useful in future
	{
		return grid->countMooreNeighbours(x, y);
	}
	void drawGrid()
	{
		//if CA is not paused, update grid
		if (!paused)
		{
			grid->updateGridWithLifeRule(rule);
			generation++;
		}

		//draw CA (if CA is paused, no updates are made)
		grid->drawGrid();
	}
};

class WolframCA : public CA
{
private:
	char neighbours[3];
public:
	WolframCA(string ruleString = "W30") : CA()
	{
		this->ruleString = ruleString;
		this->rule = new WolframRule(this->ruleString);
	}
	~WolframCA()
	{
		if (rule)
		{
			delete rule;
			rule = nullptr;
		}
	}
	void setNewRule(string ruleString)
	{
		if (rule)
			delete rule;

		this->ruleString = ruleString;
		rule = new WolframRule(this->ruleString);

		generation = 0;

		grid->clear();

		pause(true);
	}
	char getCellNeighbours(int x, int y) //not currently used
	{
		return grid->getWolframNeighbours(x, y);
	}
	void drawGrid()
	{
		//if CA is not paused, update grid
		if (!paused)
		{
			grid->updateGridWithWolframRule(rule, generation);
			generation++;
		}
		//draw CA (if CA is paused, no updates are made)
		grid->drawGrid();
	}
	void resetAutomata(void)
	{
		generation = 0;
		grid->clear();
	}
};

//holds a binary string representing evolved rule, and a number representing that rule's fitness
struct Chromosome
{
public:
	char32_t binaryRuleString;
	int fitness = 0;
};

class GeneticCA : public CA
{
private:
	/*
	//Breukelaar and Back (2005)//

	population size = 100, initial population random
	tournament selection

	to create new population, 20 random members of old population are selected, and the fittest 1 added to new generation
	this happens until newPOP_SIZE = oldPOP_SIZE

	once new generation created:
	random 60% then put through single-point crossover and mutation
	40% stays the same

	100 generations
	mutation probability = 1/16

	fitness function evaluates each transition rule’s fitness using a variety of initial states.
	For each initial state, a number of CA generations are run, and the fitness function then
	evaluates the ending states based on the relative number of adjacent cells that fit the
	required alternating pattern

	lambda = population size
	c = crossover-rate
	m = mutation probability

	*/

	//width and height used for declaring second grid object
	const static int GENETIC_CA_GRID_WIDTH = 50;
	const static int GENETIC_CA_GRID_HEIGHT = 50;

	//this is a grid object private to this class, used as a testing ground for evaluating a rule's fitness
	Grid* testGrid;

	char neighbours;

	Chromosome bestRule; //fittest individual from current population - used to update the grid at each CA generation (NOT GA generation)
	vector<Chromosome> population; //size instantiated to 100 in constructor - will hold each individual chromosome (rule) in population
	vector<Chromosome> nextGenPopulation;
	vector<Chromosome> tournamentMembers;

	const static int POP_SIZE = 100;
	const static int TOURNAMENT_SIZE = 20; //number of individuals selected for tournaments each iteration of each generation
	
	float mutationProb = 2.f / 32.f; //probability of mutation for each allele (bit) of each individual 
	float crossoverRate = 0.6f; //proportion of population that undergoes crossover and mutation

	int generation = 0;

	//random number generator objects (initialised in constructor)
	random_device rd;
	mt19937 mt;
	uniform_int_distribution<char32_t> ruleDist; //used for generating random rule strings
	uniform_int_distribution<int> populationDist; //used for choosing random members of the population for tournaments
	uniform_real_distribution<float> mutationDist; //used for deciding whether to mutate a bit or not

	int CAUpdatesPerGAPopulation = 20; //number of times to update the CA with the best rule from a given GA generation
	int updatesDoneWithRule = 0; //number of times current best rule has been used to update grid

	//copy current CA pattern to test grid so that it can be used as the starting point for evaluating an evolved rule's fitness
	void copyInitStateToTestGrid(void)
	{
		//allocation and deallocation of memory is handled by grid objects to which these pointers belong
		//this function only borrows the pointers to copy the data across
		char** gridDataPtr = grid->getGridDataPtr();
		char** testGridDataPtr = testGrid->getGridDataPtr();

		//copy the test state data across.
		//this is an expensive operation to carry out often, but there is no avoiding it because the array data
		//is modified in the course of updating the CA, and the test state must therefore be reset each time we want to
		//measure a rule's fitness
		for (int i = 0; i < GENETIC_CA_GRID_WIDTH; i++)
			for (int j = 0; j < GENETIC_CA_GRID_HEIGHT; j++)
				testGridDataPtr[i][j] = gridDataPtr[i][j];
	}

	void crossover(Chromosome& partner1, Chromosome& partner2)
	{
		//4294901760 in binary is 16 1's followed by 16 0's
		//65536 in binary is 16 0's followed by 16 1's
		//OR operation with these numbers leaves first half of number untouched, and negates second half, and vice-versa
		char32_t temp = (partner1.binaryRuleString | 4294901760) + (partner2.binaryRuleString | 65536);
		partner2.binaryRuleString = (partner2.binaryRuleString | 4294901760) + (partner1.binaryRuleString | 65536);
		partner1.binaryRuleString = temp;
	}
	void mutate(Chromosome& victim)
	{
		//for every bit in rule string
		for (int i = 0; i < 32; i++)
		{
			float mutationValue = mutationDist(mt);

			if (mutationValue <= mutationProb)
				//flip bit at this position using XOR operation on that bit (with a 1 as the second operand)
				victim.binaryRuleString = victim.binaryRuleString ^ (1 << i);
		}
	}
public:
	GeneticCA() : CA(GENETIC_CA_GRID_WIDTH, GENETIC_CA_GRID_HEIGHT), population(POP_SIZE), nextGenPopulation(POP_SIZE)
	{
		tournamentMembers.resize(TOURNAMENT_SIZE);

		//initialise random number generator objects
		mt = mt19937(rd());
		ruleDist = uniform_int_distribution<char32_t>(0, 4294967295); //upper limit = pow(2, 32) - 1
		populationDist = uniform_int_distribution<int>(0, population.size() - 1); //0 - 99
		mutationDist = uniform_real_distribution<float>(0, 1);

		//initialise grid used for testing rule fitness
		testGrid = new Grid(GENETIC_CA_GRID_WIDTH, GENETIC_CA_GRID_HEIGHT);
		
		//set CA grid to a random initial configuration and draw it
		grid->randomiseGrid();
		paused = true; //do not want to perform any updates yet, just draw the initial state
		grid->drawGrid();
		paused = false;

		//create random starting population of rules and evaluate each individual's fitness
		for (int i = 0; i < population.size(); i++)
		{
			population[i].binaryRuleString = ruleDist(mt);
			evaluateFitness(population[i]);
		}
	}
	~GeneticCA()
	{
		if (testGrid)
			delete testGrid;
	}
	int evaluateFitness(Chromosome& chromosome)
	{
		int fitness = 0;

		//copy current CA grid state to test grid state
		copyInitStateToTestGrid();

		//to assess the quality of a rule, simulate it for a few generations with current pattern as a starting point, and assess the resulting neighbourhood configurations
		for (int i = 0; i < CAUpdatesPerGAPopulation; i++)
			testGrid->updateGridWithBinaryRule(chromosome.binaryRuleString);

		//neighbour configuration: 5 least significant bits represent: top, left, current, right, bottom cells in neighbourhood
		//ideal neighbour configurations for checkerboard pattern are therefore 11011 and 00100 (27 and 4)
		//1_0_1, 0_1_0, _010_ and _101_ also have some value because orthogonally adjacent cells have alternating values
		//no other configurations are of any particular interest

		for (int i = 0; i < GENETIC_CA_GRID_WIDTH - 1; i++)
		{
			for (int j = 0; j < GENETIC_CA_GRID_HEIGHT - 1; j++)
			{
				unsigned char neighbours = testGrid->getVonNeumannNeighbours(i+1, j+1);

				if (neighbours == 27 || neighbours == 4)
				{
					fitness += 2;
					continue; //no need to check the other states because the rule has already been given a perfect fitness score for this particular neighbour configuration
				}

				unsigned char bit[5] = {
					(neighbours >> 4) & 1,
					(neighbours >> 3) & 1,
					(neighbours >> 2) & 1,
					(neighbours >> 1) & 1,
					neighbours & 1
				};

				if ((bit[0] == 1 && bit[2] == 0 && bit[4] == 1) || (bit[0] == 0 && bit[2] == 1 && bit[4] == 0))
					fitness += 1;
				if ((bit[1] == 1 && bit[2] == 0 && bit[3] == 1) || (bit[1] == 0 && bit[2] == 1 && bit[3] == 0))
					fitness += 1;
			}
		}

		//store chromosome's fitness value inside Chromosome object for later, and also return it for quick use
		chromosome.fitness = fitness;
		return fitness;
	}
	void setNewRule(string ruleString)
	{

	}
	char getCellNeighbours(int x, int y)
	{
		return grid->getVonNeumannNeighbours(x, y);
	}
	void genNextPopulation()
	{
		//repeatedly run tournaments to create the initial next-gen population
		for (int i = 0; i < population.size(); i++)
		{
			int winnerIndex = 0;

			for (int j = 0; j < TOURNAMENT_SIZE; j++)
			{
				tournamentMembers[j] = population[populationDist(mt)];
				if (tournamentMembers[j].fitness > tournamentMembers[winnerIndex].fitness)
					winnerIndex = j;
			}

			nextGenPopulation[i] = tournamentMembers[winnerIndex];
		}

		//swap contents of population and nextGenPopulation - population vector now contains the new generation
		//swap() member has constant time complexity, so this should not be an expensive operation
		population.swap(nextGenPopulation);

		Chromosome partner1;
		Chromosome partner2;

		//perform crossover and mutation on subset of initial next-gen population
		for (int i = 0; i < ceil(crossoverRate * population.size()); i += 2)
		{
			//crossover will be performed by pairing each individual with the one next to it in this subset of the population
			//the crossover point for each crossover event will be decided randomly
			partner1 = population[i];
			partner2 = population[i + 1];

			crossover(partner1, partner2);
		}

		for (int i = 0; i < ceil(crossoverRate * population.size()); i++)
			mutate(population[i]);

		for (int i = 0; i < population.size(); i++)
			evaluateFitness(population[i]);
	}
	void findBestRule()
	{
		//find best rule from current generation (fitness values have been pre-calculated)
		
		int bestRuleIndex = 0;

		for (int i = 0; i < population.size(); i++)
		{
			if (population[i].fitness > population[bestRuleIndex].fitness)
				bestRuleIndex = i;
		}

		//if none of the rules from the current generation beat the best rule from the previous generation, use that instead
		bestRule = (population[bestRuleIndex].fitness > bestRule.fitness) ? population[bestRuleIndex] : bestRule;
		bestRule = population[bestRuleIndex];
		updatesDoneWithRule = 0;
	}
	void drawGrid()
	{
		//if CA is not paused, update grid
		if (!paused)
		{
			//only create the next generation for the genetic algorithm once the current best rule has been used to
			//update the CA 20 times in a row. Doing this helps us to escape local minima
			//(i.e. patterns difficult to resolve into a checkerboard pattern using one CA update per rule)
			if (updatesDoneWithRule >= CAUpdatesPerGAPopulation)
			{
				genNextPopulation();

				for (int i = 0; i < population.size(); i++)
					evaluateFitness(population[i]);

				findBestRule();
				updatesDoneWithRule = 0;
			}

			grid->updateGridWithBinaryRule(bestRule.binaryRuleString);
			updatesDoneWithRule++;
			generation++;
		}

		//draw CA (if CA is paused, no updates are made)
		grid->drawGrid();
	}
};
