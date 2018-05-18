#pragma once

#include <iostream>
#include <vector>
#include <glew\glew.h>
#include <random>
#include <string>
#include "Rule.h"

using namespace std;

struct Coord
{
	float x, y;

	//default constructor
	Coord()
	{
		x = -1;
		y = -1;
	}
	Coord(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
};

//an object of this type should ONLY be allocated on the heap - otherwise, the large arrays will cause a stack overflow
class Grid
{
private:
	//width and height of grid (counted in number of cells)
	//ideally should be constants but this makes it impossible to pass in a different grid width and height upon object construction
	int GRID_WIDTH = 500;
	int GRID_HEIGHT = 500;

	//width and height of each cell
	const static int CELL_WIDTH = 1;
	const static int CELL_HEIGHT = 1;

	//number of vertices used for drawing the gridlines
	//in both x and y dimensions, there is one more vertex than cells (a cell has two vertices along both x and y dimensions)
	int GRID_VERTEX_NUMBER = (GRID_WIDTH + 1) * (GRID_HEIGHT + 1);

	//pointers to 2D arrays storing cell states for current CA generation and next CA generation
	char** cellStates;
	char** newCellStates;

	//time step used to update CA cell states (in ms)
	int timeStep;

	//when the CA is one of Wolfram's elementary 1D rules, we update the grid one row at a time
	//this variable keeps track of the row to begin updates from when the CA starts
	int wolframStartRow = -1;

	//used to define limits of the grid
	Coord bottomLeftPos;

	//used to store vertex position and colour data used by OpenGL to draw grid
	vector<GLfloat> vertexPosArray;// [2 * GRID_VERTEX_NUMBER]; //x and y coords for each vertex
	vector<GLuint> vertexIndexArray;// [2 * GRID_VERTEX_NUMBER]; //each vertex will be drawn twice (once for the horizontal line passing it and once for the vertical line passing it)

	//represent locations of vertex buffer objects and vertex array objects used by OpenGL to store vertex data in GPU memory
	GLuint gridVAO;
	GLuint vertexPosVBO;
	GLuint vertexIndexVBO;

	random_device rd;
	mt19937 mt;
	uniform_int_distribution<int> dist;

	void setupGridVertexData(void)
	{
		//setup vertex position and colour values for vertex buffer object
		int vertexPosArrayIndex = 0;

		for (int i = 0; i <= GRID_WIDTH; i++)
		{
			for (int j = 0; j <= GRID_HEIGHT; j++, vertexPosArrayIndex += 2)
			{
				vertexPosArray[vertexPosArrayIndex] = bottomLeftPos.x + i * CELL_WIDTH;
				vertexPosArray[vertexPosArrayIndex + 1] = bottomLeftPos.y + j * CELL_HEIGHT;
			}
		}

		//set up vertex indices for vertical grid lines
		for (int i = 0; i < GRID_VERTEX_NUMBER; i++)
			vertexIndexArray[i] = i;

		//stores "jump" distance between array values aligned on horizontal lines
		const int JUMP = GRID_HEIGHT + 1;

		//keeps track of position in index array
		int indexArrayPos = GRID_VERTEX_NUMBER;

		//set up vertex indices for horizontal grid lines
		for (int i = 0; i <= GRID_HEIGHT; i++)
			for (int j = 0; j <= GRID_WIDTH; j++, indexArrayPos++)
				vertexIndexArray[indexArrayPos] = i + j * JUMP;
	}
	void setupGridVAO(void)
	{
		glGenVertexArrays(1, &gridVAO);
		glBindVertexArray(gridVAO);

		glGenBuffers(1, &vertexPosVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexPosVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexPosArray.size() * sizeof(GLfloat), vertexPosArray.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &vertexIndexVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndexArray.size() * sizeof(GLuint), vertexIndexArray.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}
public:
	Grid()
	{
		vertexPosArray.resize(2 * GRID_VERTEX_NUMBER);
		vertexIndexArray.resize(2 * GRID_VERTEX_NUMBER);

		//setup 2D arrays used to store cell states
		cellStates = new char*[GRID_WIDTH];
		newCellStates = new char*[GRID_WIDTH];

		for (int i = 0; i < GRID_WIDTH; i++)
		{
			cellStates[i] = new char[GRID_HEIGHT];
			newCellStates[i] = new char[GRID_HEIGHT];
		}

		//set location of bottom left corner of grid
		bottomLeftPos.x = -0.5f * CELL_WIDTH * GRID_WIDTH;
		bottomLeftPos.y = -0.5f * CELL_HEIGHT * GRID_HEIGHT;

		//setup arrays that will be placed into a VBO for drawing grid
		setupGridVertexData();
		setupGridVAO();

		//zero all elements of both arrays
		for (int i = 0; i < GRID_WIDTH; i++)
		{
			for (int j = 0; j < GRID_HEIGHT; j++)
			{
				cellStates[i][j] = 0;
				newCellStates[i][j] = 0;
			}
		}

		//set-up random number generator objects
		mt = mt19937(rd());
		dist = uniform_int_distribution<int>(0, 1);
	}
	Grid(int width, int height)
	{
		GRID_WIDTH = width;
		GRID_HEIGHT = height;
		GRID_VERTEX_NUMBER = (GRID_WIDTH + 1) * (GRID_HEIGHT + 1);

		vertexPosArray.resize(2 * GRID_VERTEX_NUMBER);
		vertexIndexArray.resize(2 * GRID_VERTEX_NUMBER);

		//setup 2D arrays used to store cell states
		cellStates = new char*[GRID_WIDTH];
		newCellStates = new char*[GRID_WIDTH];

		for (int i = 0; i < GRID_WIDTH; i++)
		{
			cellStates[i] = new char[GRID_HEIGHT];
			newCellStates[i] = new char[GRID_HEIGHT];
		}

		//set location of bottom left corner of grid
		bottomLeftPos.x = -0.5f * CELL_WIDTH * GRID_WIDTH;
		bottomLeftPos.y = -0.5f * CELL_HEIGHT * GRID_HEIGHT;

		//setup arrays that will be placed into a VBO for drawing grid
		setupGridVertexData();
		setupGridVAO();

		//zero all elements of both arrays
		for (int i = 0; i < GRID_WIDTH; i++)
		{
			for (int j = 0; j < GRID_HEIGHT; j++)
			{
				cellStates[i][j] = 0;
				newCellStates[i][j] = 0;
			}
		}
	}
	~Grid()
	{
		//delete cell states and new cell states
		for (int i = 0; i < GRID_WIDTH; i++)
		{
			delete[] cellStates[i];
			delete[] newCellStates[i]; //debug error happens when this is called - newCellStates points to the same address as cellStates
														//error ONLY happens when wolfram rule that has hit the limits of the grid is changed to a life rule
		}

		delete[] cellStates;
		delete[] newCellStates;

		cellStates = nullptr;
		newCellStates = nullptr;
	}

	void highlightCell(int x, int y)
	{
		if (x >= GRID_WIDTH || y >= GRID_HEIGHT || x < 0 || y < 0)
			return;

		float X = bottomLeftPos.x + x;
		float Y = bottomLeftPos.y + y;

		glBegin(GL_TRIANGLE_STRIP);

		glVertex2f(X, Y);
		glVertex2f(X + CELL_WIDTH, Y);
		glVertex2f(X, Y + CELL_HEIGHT);
		glVertex2f(X + CELL_WIDTH, Y + CELL_HEIGHT);

		glEnd();
	}
	void drawGrid(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//since all gridline vertices are the same colour, it is more efficient to set colour once using immediate mode than to set up a vertex colour VBO
		glColor3ub(17, 173, 111);

		glBindVertexArray(gridVAO);

		//draw vertical lines of grid
		for (int i = 0; i <= GRID_WIDTH; i++)
			glDrawElements(GL_LINE_STRIP, GRID_HEIGHT + 1, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (GRID_HEIGHT + 1) * i));

		//draw horizontal lines of grid
		for (int i = 0; i <= GRID_HEIGHT; i++)
			glDrawElements(GL_LINE_STRIP, GRID_WIDTH + 1, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (GRID_WIDTH + 1) * i + sizeof(GLuint) * GRID_VERTEX_NUMBER));

		glBindVertexArray(0);

		glColor3ub(255, 255, 255);

		//iterate through grid, highlighting whichever cells are "alive"
		for (int i = 0; i < GRID_WIDTH; i++)
			for (int j = 0; j < GRID_HEIGHT; j++)
				if (cellStates[i][j] == 1)
					highlightCell(i, j);
	}

	//return the number of living neighbours around the current cell - their position does not matter since Conway's Game of Life is a totalistic CA
	unsigned char countMooreNeighbours(int x, int y)
	{
		//make sure cell and its Moore neighbours actually exist within the grid
		if ((x - 1 < 0) || (x + 1 >= GRID_WIDTH) || (y - 1 < 0) || (y + 1 >= GRID_HEIGHT))
			return 0;

		//corrects calculation for the current cell
		char total = - cellStates[x][y];

		//iterate through current cell and its eight immediate neighbours, adding up the number of living cells found
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				total += cellStates[x + i - 1][y + j - 1];

		return total;
	}

	//pack neighbour data (left cell state, current cell state, right cell state) into a single 8-bit binary number (of which 3 bits are used), using 1s and 0s to represent on and off states
	unsigned char getWolframNeighbours(int x, int y)
	{
		if (x - 1 < 0 || x + 1 >= GRID_WIDTH)
			return 0;

		unsigned char neighbours = 0;

		//set up neighbour with 3 least significant bits representing states of left neighbour, current cell, right neighbour
		neighbours = (cellStates[x - 1][y] << 2) | (cellStates[x][y] << 1) | (cellStates[x + 1][y]);

		return neighbours;
	}
	
	//get von Neumann neighbour configuration (top, left, current, right, bottom) and pack it into a char using 5 least significant bits
	unsigned char getVonNeumannNeighbours(int x, int y)
	{

		if (x - 1 < 0 || x + 1 >= GRID_WIDTH || y - 1 < 0 || y + 1 >= GRID_HEIGHT)
			return '_';

		unsigned char neighbours = 0;

		neighbours = (cellStates[x][y + 1] << 4) | (cellStates[x - 1][y] << 3) | (cellStates[x][y] << 2) | (cellStates[x + 1][y] << 1) | (cellStates[x][y - 1]);
		return neighbours;
	}

	void randomiseGrid()
	{
		for (int i = 0; i < GRID_WIDTH; i++)
		{
			for (int j = 0; j < GRID_HEIGHT; j++)
			{
				cellStates[i][j] = dist(mt);
				newCellStates[i][j] = 0;
			}
		}
	}

	void updateGridWithLifeRule(Rule* rule)
	{
		//clear the array containing new cell states ready to update CA
		for (int i = 0; i < GRID_WIDTH; i++)
			for (int j = 0; j < GRID_HEIGHT; j++)
				newCellStates[i][j] = 0;

		const char* birthRule = ((LifeRule*)rule)->getBirthRule();
		const char* survivalRule = ((LifeRule*)rule)->getSurvivalRule();

		//calculate next generation states of every cell, storing them in newCellStates
		for (int i = 0; i < GRID_WIDTH; i++)
		{
			for (int j = 0; j < GRID_HEIGHT; j++)
			{
				short neighbourCount = countMooreNeighbours(i, j);

				//apply survival section of rule
				if (cellStates[i][j] == 1)
				{
					int count = 0;
					bool survive = false;

					while (survivalRule[count] != '\n')
					{
						if (neighbourCount == survivalRule[count])
							survive = true;
						count++;
					}

					//if no match has been found for neighbour count in survival section of rule, current cell will die
					newCellStates[i][j] = (char)survive;
				}
				//apply birth section of rule
				else if (cellStates[i][j] == 0)
				{
					int count = 0;

					while (birthRule[count] != '\n')
					{
						if (neighbourCount == birthRule[count])
							newCellStates[i][j] = 1;
						count++;
					}
				}
			}
		}

		//swap pointers to cellStates and newCellStates arrays
		char** temp = cellStates;
		cellStates = newCellStates;
		newCellStates = temp;
	}
	void updateGridWithWolframRule(Rule* rule, int& generation)
	{
		//get rule array
		const bool* ruleArray = ((WolframRule*)rule)->getRule();

		//if the row to start updates from is unknown, iterate around grid from top to bottom, left to right, looking for any cell that is alive
		//its row will become the start row
		if (wolframStartRow == -1)
		{
			for (int i = GRID_HEIGHT - 1; i >= 0 && wolframStartRow == -1; i--)
				for (int j = 0; j < GRID_WIDTH && wolframStartRow == -1; j++)
					wolframStartRow = ((cellStates[j][i] == 1) ? i : wolframStartRow); //shorthand: if cellStates[i][j] == 1, set start row to j, else assign already existing value

			//clear the array containing new cell states ready to update CA - this will ONLY be done on the first call to this function
			//this is because we will never want to update any part of the grid apart from the row representing the current generation
			for (int i = 0; i < GRID_WIDTH; i++)
				for (int j = 0; j < GRID_HEIGHT; j++)
					newCellStates[i][j] = 0;

			//ensure generation count starts from 0 when the user sets a cell state (otherwise updates will be performed on the wrong row)
			generation = 0;
		}

		int height = wolframStartRow - generation;

		for (int i = 0; i < GRID_WIDTH && height > 0; i++)
		{
			unsigned char neighbours = getWolframNeighbours(i, height);
			newCellStates[i][height - 1] = ruleArray[neighbours];

			newCellStates[i][height] = cellStates[i][height];
		}

		//swap pointers to cellStates and newCellStates arrays
		char** temp = cellStates;
		cellStates = newCellStates;
		newCellStates = temp;
	}
	
	void updateGridWithBinaryRule(char32_t rule)
	{
		//clear the array containing new cell states ready to update CA
		for (int i = 0; i < GRID_WIDTH; i++)
			for (int j = 0; j < GRID_HEIGHT; j++)
				newCellStates[i][j] = 0;

		//char32_t representing rule:
		//1st msb = state of cell when neighbours are 11111, 2nd msb = state of cell when neighbours are 11110, etc.

		for (int i = 0; i < GRID_WIDTH; i++)
		{
			for (int j = 0; j < GRID_HEIGHT; j++)
			{
				unsigned char neighbours = getVonNeumannNeighbours(i, j);

				if (neighbours == '_')
					continue;

				newCellStates[i][j] = (rule >> neighbours) & 1; //get bit from relevant position in rule
			}
		}

		//swap pointers to cellStates and newCellStates arrays
		char** temp = cellStates;
		cellStates = newCellStates;
		newCellStates = temp;
	}

	int getGridWidth(void)
	{
		return GRID_WIDTH;
	}
	int getGridHeight(void)
	{
		return GRID_HEIGHT;
	}
	int findGridXCoord(double column)
	{
		int columnInt = round(column);

		for (int i = 0; i < GRID_WIDTH; i++)
		{
			if (columnInt >= bottomLeftPos.x + CELL_WIDTH * i && columnInt < bottomLeftPos.x + CELL_WIDTH * (i + 1))
				return i;
		}

		//should never reach this point
		return 0;
	}
	int findGridYCoord(double row)
	{
		int rowInt = round(row);

		for (int i = 0; i < GRID_HEIGHT; i++)
		{
			if (rowInt > bottomLeftPos.y + CELL_HEIGHT * i && rowInt < bottomLeftPos.y + CELL_HEIGHT * (i + 1))
				return i;
		}

		//should never reach this point
		return 0;
	}
	
	//gives access to pointer to actual grid data
	//used for copying data between standard grid object and test grid used to try out rules for the genetic CA
	char** getGridDataPtr(void)
	{
		return cellStates;
	}

	void editCell(int x, int y)
	{
		if (x >= GRID_WIDTH || y >= GRID_HEIGHT || x < 0 || y < 0)
			return;

		if (cellStates[x][y] == 0)
			cellStates[x][y] = 1;
		else
			cellStates[x][y] = 0;
	}
	void clear(void)
	{
		for (int i = 0; i < GRID_WIDTH; i++)
			for (int j = 0; j < GRID_HEIGHT; j++)
				cellStates[i][j] = newCellStates[i][j] = 0;

		wolframStartRow = -1;
	}
};