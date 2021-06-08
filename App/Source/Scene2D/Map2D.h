/**
 Map2D
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include the RapidCSV
#include "System/rapidcsv.h"
#include "System/MyMath.h"
// Include map storage
#include <map>

// Include Settings
#include "GameControl\Settings.h"

// Include Entity2D
#include "Primitives/Entity2D.h"

// Include Mesh
#include "Primitives/Mesh.h"
#include "Primitives/MeshBuilder.h"

// A structure storing information about Map Sizes
struct MapSize {
	unsigned int uiRowSize;
	unsigned int uiColSize;
};

// A structure storing information about a map grid
// It includes data to be used for A* Path Finding
struct Grid {
	unsigned int value;
	float timer;
	bool updated;

	// Row and Column index of its parent
	// Note that 0 <= i < uiRowSize & 0 <= j < uiColSize
	unsigned int uiParentRow, uiParentCol;
	// Movement Costs: f = g + h
	double f, g, h;
};

class CMap2D : public CSingletonTemplate<CMap2D>, public CEntity2D
{
	friend CSingletonTemplate<CMap2D>;
public:

	enum BLOCK_TYPE
	{
		BLOCKS = 0,
		TREES,
		BACKGROUND_BLOCKS,
		COLLECTABLES,
		TOTAL_BLOCK_TYPE
	};

	static BLOCK_TYPE blockType;

	// Init
	bool Init(	const unsigned int uiNumLevels = 1,
				const unsigned int uiNumRows = 24,
				const unsigned int uiNumCols = 32);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	// Set the specifications of the map
	void SetNumTiles(const CSettings::AXIS sAxis, const unsigned int uiValue);
	void SetNumSteps(const CSettings::AXIS sAxis, const unsigned int uiValue);

	// Set the value at certain indices in the arrMapInfo
	void SetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert = true);

	void SetSaveMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert = true);

	// Get the value at certain indices in the arrMapInfo
	int GetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const bool bInvert = true) const;

	// Load a map
	bool LoadMap(string filename, const unsigned int uiLevel = 0);

	// Save a tilemap
	bool SaveMap(string filename, const unsigned int uiLevel = 0);

	//generate a new default map and file
	bool GenerateNewMap(string worldName = "START");

	//load a new default map
	bool LoadNewMap(string filename);

	//generate a randomly loaded map
	bool GenerateRandomMap();

	// Find the indices of a certain value in arrMapInfo
	bool FindValue(const int iValue, unsigned int& uirRow, unsigned int& uirCol, const bool bInvert = true);

	//return active world path with extention
	string GetActiveWorldPath();

	//update chest 
	string OpenedChest(glm::vec2);

	//return world name list vector
	vector<string> GetWorldNameList();
	
	//return total worlds generated
	int GetTotalWorldsGenerated();

	//set total worlds generated
	void SetTotalWorldsGenerated(int);

	//active world name
	static string activeWorld;

	//world names list
	vector<string> worldNameList;

	//map offset for side scrolling effect
	glm::vec2 mapOffset;
	glm::vec2 mapOffset_MicroSteps;

	//update individual block seeds based on timing
	void UpdateSeed(string itemName, double dt, int blockNumber, float timer);

	BLOCK_TYPE GetBlockType(int blockNumber);

protected:

	// The variable containing the rapidcsv::Document
	// We will load the CSV file's content into this Document
	rapidcsv::Document doc;

	// A 3-D array which stores the values of the tile map
	Grid*** arrMapInfo;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	// The current level
	unsigned int uiCurLevel;

	// The number of levels
	unsigned int uiNumLevels;

	// A 1-D array which stores the map sizes for each level
	MapSize* arrMapSizes;

	// Map containing texture IDs
	map<int, int> MapOfTextureIDs;

	// Constructor
	CMap2D(void);

	// Destructor
	virtual ~CMap2D(void);

	// Load a texture
	bool LoadTexture(const char* filename, const int iTextureCode);

	// Render a tile
	void RenderTile(const unsigned int uiRow, const unsigned int uiCol);

private:
	//total worlds generated
	int totalWorlds = 1;

	//total textures
	int totalTextures = 1000;
};

