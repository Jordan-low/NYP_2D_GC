/**
 Map2D
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Map2D.h"

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include Filesystem
#include "System\filesystem.h"

// Include ImageLoader
#include "System\ImageLoader.h"

#include <iostream>
#include <vector>
using namespace std;
string CMap2D::activeWorld = "START";
CMap2D::BLOCK_TYPE blockType;

// For AStar PathFinding
using namespace std::placeholders;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CMap2D::CMap2D(void)
	: uiCurLevel(0)
	, mapOffset(glm::vec2(0.0f))
	, mapOffset_MicroSteps(glm::vec2(0.0f))
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CMap2D::~CMap2D(void)
{
	// Dynamically deallocate the 3D array used to store the map information
	for (unsigned int uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		for (unsigned int iRow = 0; iRow < cSettings->NUM_TILES_YAXIS; iRow++)
		{
			delete[] arrMapInfo[uiLevel][iRow];
		}
		delete [] arrMapInfo[uiLevel];
	}
	delete[] arrMapInfo;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);

	// Set this to NULL since it was created elsewhere, so we let it be deleted there.
	cSettings = NULL;

	// Delete AStar lists
	DeleteAStarLists();
}

/**
@brief Init Initialise this instance
*/ 
bool CMap2D::Init(	const unsigned int uiNumLevels,
					const unsigned int uiNumRows,
					const unsigned int uiNumCols)
{
	//setting the default doc
	doc = rapidcsv::Document(FileSystem::getPath("Maps/START.csv").c_str());

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Create the arrMapInfo and initialise to 0
	// Start by initialising the number of levels
	arrMapInfo = new Grid** [uiNumLevels];
	for (int uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		arrMapInfo[uiLevel] = new Grid* [uiNumRows];
		for (int uiRow = 0; uiRow < uiNumRows; uiRow++)
		{
			arrMapInfo[uiLevel][uiRow] = new Grid[uiNumCols];
			for (int uiCol = 0; uiCol < uiNumCols; uiCol++) 
			{
				arrMapInfo[uiLevel][uiRow][uiCol].value = 0;
				arrMapInfo[uiLevel][uiRow][uiCol].timer = 0;
				arrMapInfo[uiLevel][uiRow][uiCol].health = 100.f;
			}
		}
	}

	//tile_yaxis - 24
	//mapOffset.y = 6 * cSettings->TILE_HEIGHT;

	// Store the map sizes in cSettings
	uiCurLevel = 0;
	this->uiNumLevels = uiNumLevels;
	cSettings->NUM_TILES_XAXIS = uiNumCols;
	cSettings->NUM_TILES_YAXIS = uiNumRows;
	cSettings->UpdateSpecifications();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Initialise the variables for AStar
	m_weight = 1;
	m_startPos = glm::i32vec2(0, 0);
	m_targetPos = glm::i32vec2(0, 0);
	//m_size = cSettings->NUM_TILES_YAXIS* cSettings->NUM_TILES_XAXIS;

	m_nrOfDirections = 4;
	m_directions = { { -1, 0 }, { 1, 0 }, { 0, 1 }, { 0, -1 },
						{ -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 } };

	// Resize these 2 lists
	m_cameFromList.resize(cSettings->NUM_TILES_YAXIS * cSettings->NUM_TILES_XAXIS);
	m_closedList.resize(cSettings->NUM_TILES_YAXIS * cSettings->NUM_TILES_XAXIS, false);
	
	//generate the quad mesh using the meshbuilder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	//default blocks
	if (LoadTexture("Image/Blocks/Bedrock.png", 1) == false)
	{
		std::cout << "Failed to load bedrock block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Blocks/GrassBlock.png", 2) == false)
	{
		std::cout << "Failed to load grass block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Blocks/DirtBlock.png", 3) == false)
	{
		std::cout << "Failed to load dirt block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Blocks/Chest.png", 4) == false)
	{
		std::cout << "Failed to load dirt block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Blocks/Lava.png", 5) == false)
	{
		std::cout << "Failed to load lava block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Blocks/Shop.png", 6) == false)
	{
		std::cout << "Failed to load shop block texture" << std::endl;
		return false;
	}
	  
	//trees & seeds
	if (LoadTexture("Image/Trees/GrassTree.png", 100) == false)
	{
		std::cout << "Failed to load grass tree seed block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Trees/GrassTreeGrown.png", 101) == false)
	{
		std::cout << "Failed to load grass tree block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Trees/DirtTree.png", 102) == false)
	{
		std::cout << "Failed to load dirt tree seed block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Trees/DirtTreeGrown.png", 103) == false)
	{
		std::cout << "Failed to load dirt tree block texture" << std::endl;
		return false;
	}

	//drop items
	if (LoadTexture("Image/Items/Stone.png", 301) == false)
	{
		std::cout << "Failed to load stone block texture" << std::endl;
		return false;
	}
	if (LoadTexture("Image/Items/Cheese.png", 302) == false)
	{
		std::cout << "Failed to load dirt seed drop texture" << std::endl;
		return false;
	}

	//background items
	if (LoadTexture("Image/Blocks/Door.png", 201) == false)
	{
		std::cout << "Failed to load door texture" << std::endl;
		return false;
	}
	//background items
	if (LoadTexture("Image/UI/blockRange.png", 202) == false)
	{
		std::cout << "Failed to load block range texture" << std::endl;
		return false;
	}

	//Enemy
	if (LoadTexture("Image/Characters/Enemy.png", 401) == false)
	{
		std::cout << "Failed to load enemy texture" << std::endl;
		return false;
	}

	// Load the map into an array
	if (LoadMap("Maps/START.csv", 0) == false)
	{
		// The initialisation of this map has failed. Return false
		return false;
	}

	return true;
}

/**
@brief Update Update this instance
*/
void CMap2D::Update(const double dElapsedTime)
{	
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CMap2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render Render this instance
 */
void CMap2D::Render(void)
{
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	
	// Render
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
		{
			float xOffset = 0.f;
			if (CPlayer2D::GetInstance()->i32vec2Index.x > cSettings->TILE_RATIO_XAXIS / 2)
				xOffset = (CPlayer2D::GetInstance()->i32vec2Index.x + (CPlayer2D::GetInstance()->i32vec2NumMicroSteps.x / cSettings->NUM_STEPS_PER_TILE_XAXIS)) - cSettings->TILE_RATIO_XAXIS / 2;

			if (CPlayer2D::GetInstance()->i32vec2Index.x > cSettings->NUM_TILES_XAXIS - cSettings->TILE_RATIO_XAXIS / 2)
				xOffset = cSettings->NUM_TILES_XAXIS - cSettings->TILE_RATIO_XAXIS;

			/*float yOffset = 0.f;
			if (CPlayer2D::GetInstance()->i32vec2Index.y > cSettings->TILE_RATIO_YAXIS / 2)
				yOffset = (CPlayer2D::GetInstance()->i32vec2Index.y + (CPlayer2D::GetInstance()->i32vec2NumMicroSteps.y / cSettings->NUM_STEPS_PER_TILE_YAXIS)) - cSettings->TILE_RATIO_YAXIS / 2;

			if (CPlayer2D::GetInstance()->i32vec2Index.y > cSettings->NUM_TILES_YAXIS - cSettings->TILE_RATIO_YAXIS / 2)
				yOffset = cSettings->NUM_TILES_YAXIS - cSettings->TILE_RATIO_YAXIS;*/

			transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			transform = glm::translate(transform, glm::vec3(cSettings->ConvertFloatIndexToUVSpace(cSettings->x, uiCol - xOffset, false, 0),
				cSettings->ConvertFloatIndexToUVSpace(cSettings->y, uiRow, true, 0),
				0.0f));

			//transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			//transform = glm::translate(transform, glm::vec3(cSettings->ConvertIndexToUVSpace(cSettings->x, uiCol, false, 0) + mapOffset.x,
			//												cSettings->ConvertIndexToUVSpace(cSettings->y, uiRow, true, 0),
			//												0.0f));
			//transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

			// Update the shaders with the latest transform
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			// Render a tile
			RenderTile(uiRow, uiCol);
		}
	}
}

//unsigned CMap2D::getCurrX(void) {
//	return xList[uiCurLevel];
//}


/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CMap2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

// Set the specifications of the map
void CMap2D::SetNumTiles(const CSettings::AXIS sAxis, const unsigned int uiValue)
{
	// Check if the value is valid
	if (uiValue <= 0)
	{
		cout << "CMap2D::SetNumTiles() : value must be more than 0" << endl;
		return;
	}

	if (sAxis == CSettings::x)
	{
		cSettings->NUM_TILES_XAXIS = uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::y)
	{
		cSettings->NUM_TILES_YAXIS = uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::z)
	{
		// Not used in here
	}
	else
	{
		cout << "Unknown axis" << endl;
	}
}

// Set the specifications of the map
void CMap2D::SetNumSteps(const CSettings::AXIS sAxis, const unsigned int uiValue)
{
	// Check if the value is valid
	if (uiValue <= 0)
	{
		cout << "CMap2D::SetNumSteps() : value must be more than 0" << endl;
		return;
	}

	if (sAxis == CSettings::x)
	{
		cSettings->NUM_STEPS_PER_TILE_XAXIS = (float)uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::y)
	{
		cSettings->NUM_STEPS_PER_TILE_YAXIS = (float)uiValue;
		cSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::z)
	{
		// Not used in here
	}
	else
	{
		cout << "Unknown axis" << endl;
	}
}

/**
 @brief Set the value at certain indices in the arrMapInfo
 @param iRow A const int variable containing the row index of the element to set to
 @param iCol A const int variable containing the column index of the element to set to
 @param iValue A const int variable containing the value to assign to this arrMapInfo
 */
void CMap2D::SetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert)
{
	if (bInvert)
		arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value = iValue;
	else
		arrMapInfo[uiCurLevel][uiRow][uiCol].value = iValue;
}

void CMap2D::SetSaveMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert, const bool docSave)
{
	if (uiRow >= 0 && uiRow < cSettings->NUM_TILES_YAXIS && uiCol >= 0 && uiCol < cSettings->NUM_TILES_XAXIS) //check if its within screen
	{
		if (bInvert)
		{
			arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value = iValue;
			doc.SetCell(uiCol, cSettings->NUM_TILES_YAXIS - uiRow - 1, arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value);
		}
		else
		{
			arrMapInfo[uiCurLevel][uiRow][uiCol].value = iValue;
			doc.SetCell(uiCol, uiRow, arrMapInfo[uiCurLevel][uiRow][uiCol].value);
		}
		if (docSave)
			doc.Save(FileSystem::getPath(GetActiveWorldPath()).c_str());
	}
}

/**
 @brief Get the value at certain indices in the arrMapInfo
 @param iRow A const int variable containing the row index of the element to get from
 @param iCol A const int variable containing the column index of the element to get from
 @param bInvert A const bool variable which indicates if the row information is inverted
 */
int CMap2D::GetMapInfo(const unsigned int uiRow, const int unsigned uiCol, const bool bInvert) const
{
	if (uiRow >= 0 && uiRow < cSettings->NUM_TILES_YAXIS && uiCol >= 0 && uiCol < cSettings->NUM_TILES_XAXIS) //check if its within screen
	{
		if (bInvert)
			return arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value;
		else
			return arrMapInfo[uiCurLevel][uiRow][uiCol].value;
	}
}

/**
 @brief Load a map
 */ 
bool CMap2D::LoadMap(string filename, const unsigned int uiCurLevel)
{
	doc = rapidcsv::Document(FileSystem::getPath(filename).c_str());
	
	//// Check if the sizes of CSV data matches the declared arrMapInfo sizes
	//if ((cSettings->NUM_TILES_XAXIS != v) ||
	//	(cSettings->NUM_TILES_YAXIS != (unsigned int)doc.GetRowCount()))
	//{
	//	cout << "Sizes of CSV map does not match declared arrMapInfo sizes." << endl;
	//	return false;
	//}

	cSettings->NUM_TILES_XAXIS = (unsigned int)doc.GetColumnCount();
	cSettings->NUM_TILES_YAXIS = (unsigned int)doc.GetRowCount();

	// Read the rows and columns of CSV data into arrMapInfo
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		// Read a row from the CSV file
		std::vector<std::string> row = doc.GetRow<std::string>(uiRow);
		
		// Load a particular CSV value into the arrMapInfo
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; ++uiCol)
		{
			arrMapInfo[uiCurLevel][uiRow][uiCol].value = (int)stoi(row[uiCol]);
		}
	}

	return true;
}

/**
 @brief Save the tilemap to a text file
 @param filename A string variable containing the name of the text file to save the map to
 */
bool CMap2D::SaveMap(string filename, const unsigned int uiCurLevel)
{
	// Update the rapidcsv::Document from arrMapInfo
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
		{
			doc.SetCell(uiCol, uiRow, arrMapInfo[uiCurLevel][uiRow][uiCol].value);
		}
		cout << endl;
	}

	// Save the rapidcsv::Document to a file
	doc.Save(FileSystem::getPath(filename).c_str());

	return true;
}

/**
 @brief generate a new random map
 @param filename A string variable containing the name of the world
 */
bool CMap2D::GenerateNewMap(string worldName)
{
	LoadNewMap("Maps/" + worldName + ".csv");
	GenerateRandomMap();
	SaveMap("Maps/" + worldName + ".csv");
	return true;
}

/**
 @brief Load a new default map
 @param filename A string variable containing the name of the world
 */
bool CMap2D::LoadNewMap(string filename)
{
	doc = rapidcsv::Document(FileSystem::getPath("Maps/DEFAULT.csv").c_str());

	//// Check if the sizes of CSV data matches the declared arrMapInfo sizes
	//if ((cSettings->NUM_TILES_XAXIS != (unsigned int)doc.GetColumnCount()) ||
	//	(cSettings->NUM_TILES_YAXIS != (unsigned int)doc.GetRowCount()))
	//{
	//	cout << "Sizes of CSV map does not match declared arrMapInfo sizes." << endl;
	//	return false;
	//}

	cSettings->NUM_TILES_XAXIS = (unsigned int)doc.GetColumnCount();
	cSettings->NUM_TILES_YAXIS = (unsigned int)doc.GetRowCount();

	// Read the rows and columns of CSV data into arrMapInfo
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		// Read a row from the CSV file
		std::vector<std::string> row = doc.GetRow<std::string>(uiRow);

		// Load a particular CSV value into the arrMapInfo
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; ++uiCol)
		{
			arrMapInfo[uiCurLevel][uiRow][uiCol].value = (int)stoi(row[uiCol]);
		}
	}

	return true;
}

/**
@brief Generate a fully loaded random map
*/
bool CMap2D::GenerateRandomMap()
{
	int current = 20;
	int chestSpawned = 0;
	int chestSpawnRate = 20;
	int lavaSpawnRate = 10;
	int treeSpawnRate = 8;
	int enemySpawnRate = 20;

	for (int i = 3; i < cSettings->NUM_TILES_XAXIS; i++)
	{
		bool done = false;
		while (!done)
		{
			int random = Math::RandIntMinMax(18, 24);
			if (random + 1 > cSettings->NUM_TILES_YAXIS - 1 || random - 1 < 0)
				continue;

			if (random + 1 == current || random - 1 == current || random == current)
			{
				//spawn trees
				int treeRandom = Math::RandIntMinMax(0, treeSpawnRate);
				if (treeRandom == 0)
				{
					int treeType = Math::RandIntMinMax(1, 2);
					switch (treeType)
					{
					case 1:
						SetMapInfo(random - 1, i, 100, false);
						break;
					case 2:
						SetMapInfo(random - 1, i, 102, false);
						break;
					default:
						break;
					}
				}

				//spawn chest
				int chestRandom = Math::RandIntMinMax(0, chestSpawnRate);
				if (chestRandom == 0)
				{
					SetMapInfo(random - 1, i, 4, false);
					chestSpawned++;
				}

				//spawn enemies
				//spawn chest
				int enemyRandom = Math::RandIntMinMax(0, enemySpawnRate);
				if (enemyRandom == 0)
				{
					SetMapInfo(random - 1, i, 401, false);
				}

				//force spawn chest
				if (i == 99 && chestSpawned == 0)
					SetMapInfo(random - 1, i, 4, false);

				//set grass and dirt floor
				SetMapInfo(random, i, 2, false);
				for (int j = 1; j < cSettings->NUM_TILES_YAXIS - random; j++)
				{
					if (j == cSettings->NUM_TILES_YAXIS - random - 1)
						SetMapInfo(random + j, i, 1, false);
					else
						SetMapInfo(random + j, i, 3, false);
				}

				int lavaRandom = Math::RandIntMinMax(0, lavaSpawnRate);
				if (lavaRandom == 0)
				{
					if (random + 1 == cSettings->NUM_TILES_YAXIS - 1)
						continue;
					SetMapInfo(random + 1, i, 5, false);
				}

				done = true;
				current = random;
			}
		}
	}

	return true;
}

bool CMap2D::ProceduralGeneration()
{
	std::cout << "GENERATING" << std::endl;
	int initialNumTilesXAxis = cSettings->NUM_TILES_XAXIS;
	cSettings->NUM_TILES_XAXIS += 10;
	std::cout << cSettings->NUM_TILES_XAXIS << std::endl;

	int current = 20;
	int chestSpawned = 0;
	int chestSpawnRate = 20;
	int lavaSpawnRate = 10;
	int treeSpawnRate = 8;
	int enemySpawnRate = 20;

	for (int i = initialNumTilesXAxis; i < cSettings->NUM_TILES_XAXIS; i++)
	{
		for (int j = 0; j < cSettings->NUM_TILES_YAXIS; j++)
		{
			SetSaveMapInfo(j, i, 0, false, false);
		}
	}

	std::cout << "SET ALL TO 0" << std::endl;

	for (int i = initialNumTilesXAxis; i < cSettings->NUM_TILES_XAXIS; i++)
	{
		bool done = false;
		while (!done)
		{
			int random = Math::RandIntMinMax(18, 24);
			if (random + 1 > cSettings->NUM_TILES_YAXIS - 1 || random - 1 < 0)
				continue;

			if (random + 1 == current || random - 1 == current || random == current)
			{
				//spawn trees
				int treeRandom = Math::RandIntMinMax(0, treeSpawnRate);
				if (treeRandom == 0)
				{
					int treeType = Math::RandIntMinMax(1, 2);
					switch (treeType)
					{
					case 1:
						SetSaveMapInfo(random - 1, i, 100, false, false);
						break;
					case 2:
						SetSaveMapInfo(random - 1, i, 102, false, false);
						break;
					default:
						break;
					}
				}

				//spawn chest
				int chestRandom = Math::RandIntMinMax(0, chestSpawnRate);
				if (chestRandom == 0)
				{
					SetSaveMapInfo(random - 1, i, 4, false, false);
					chestSpawned++;
				}

				//spawn enemies
				//spawn chest
				int enemyRandom = Math::RandIntMinMax(0, enemySpawnRate);
				if (enemyRandom == 0)
				{
					SetSaveMapInfo(random - 1, i, 401, false, false);
				}

				//force spawn chest
				if (i == 99 && chestSpawned == 0)
					SetSaveMapInfo(random - 1, i, 4, false, false);

				//set grass and dirt floor
				SetSaveMapInfo(random, i, 2, false, false);
				for (int j = 1; j < cSettings->NUM_TILES_YAXIS - random; j++)
				{
					if (j == cSettings->NUM_TILES_YAXIS - random - 1)
						SetSaveMapInfo(random + j, i, 1, false, false);
					else
						SetSaveMapInfo(random + j, i, 3, false, false);
				}

				int lavaRandom = Math::RandIntMinMax(0, lavaSpawnRate);
				if (lavaRandom == 0)
				{
					if (random + 1 == cSettings->NUM_TILES_YAXIS - 1)
						continue;
					SetSaveMapInfo(random + 1, i, 5, false, false);
				}

				done = true;
				current = random;
			}
		}
	}
	std::cout << "SAVING FILE" << std::endl;
	doc.Save(FileSystem::getPath(GetActiveWorldPath()).c_str());
	return true;
}

/**
@brief Find the indices of a certain value in arrMapInfo
@param iValue A const int variable containing the row index of the found element
@param iRow A const int variable containing the row index of the found element
@param iCol A const int variable containing the column index of the found element
@param bInvert A const bool variable which indicates if the row information is inverted
*/
bool CMap2D::FindValue(const int iValue, unsigned int& uirRow, unsigned int& uirCol, const bool bInvert)
{
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
		{
			if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == iValue)
			{
				if (bInvert)
					uirRow = cSettings->NUM_TILES_YAXIS - uiRow - 1;
				else
					uirRow = uiRow;
				uirCol = uiCol;
				return true;	// Return true immediately if the value has been found
			}
		}
	}
	return false;
}

/**
@brief Get current active world path
*/
string CMap2D::GetActiveWorldPath()
{
	return "Maps/" + activeWorld + ".csv";
}

/**
@brief Update open chest
*/
string CMap2D::OpenedChest(glm::vec2 playerPos)
{
	SetSaveMapInfo(playerPos.y, playerPos.x, 0);

	int random = Math::RandIntMinMax(301, 302);
	SetSaveMapInfo(playerPos.y + 1, playerPos.x, random);

	return "";
}

string CMap2D::KilledEnemy(glm::vec2 playerPos, float damage)
{
	arrMapInfo[0][(int)playerPos.y][(int)playerPos.x].health -= damage;
	
	if (arrMapInfo[0][(int)playerPos.y][(int)playerPos.x].health <= 0.f)
	{
		SetSaveMapInfo(playerPos.y, playerPos.x, 0);

		int random = Math::RandIntMinMax(301, 301);
		SetSaveMapInfo(playerPos.y + 1, playerPos.x, random);
	}

	return "";
}

/**
 @brief Load a texture, assign it a code and store it in MapOfTextureIDs.
 @param filename A const char* variable which contains the file name of the texture
 @param iTextureCode A const int variable which is the texture code.
 */
bool CMap2D::LoadTexture(const char* filename, const int iTextureCode)
{
	// Variables used in loading the texture
	int width, height, nrChannels;
	unsigned int textureID;

	// texture 1
	// ---------
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	CImageLoader* cImageLoader = CImageLoader::GetInstance();
	unsigned char *data = cImageLoader->Load(filename, width, height, nrChannels, true);
	if (data)
	{
		if (nrChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (nrChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		// Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);

		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(iTextureCode, textureID));
	}
	else
	{
		return false;
	}
	// Free up the memory of the file data read in
	free(data);

	return true;
}

/**
 @brief Render a tile at a position based on its tile index
 @param iRow A const int variable containing the row index of the tile
 @param iCol A const int variable containing the column index of the tile
 */
void CMap2D::RenderTile(const unsigned int uiRow, const unsigned int uiCol)
{
	if (arrMapInfo[uiCurLevel][uiRow][uiCol].value != 0)
	{
		if (arrMapInfo[uiCurLevel][uiRow][uiCol].value < totalTextures)
 			glBindTexture(GL_TEXTURE_2D, MapOfTextureIDs.at(arrMapInfo[uiCurLevel][uiRow][uiCol].value));

		glBindVertexArray(VAO);
		// Render the tile
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		quadMesh->Render();
		glBindVertexArray(0);
	}
}

/**
 @brief Return world name vector list
 */
vector<string> CMap2D::GetWorldNameList()
{
	return worldNameList;
}

/**
 @brief Get total worlds generated
 */
int CMap2D::GetTotalWorldsGenerated()
{
	return totalWorlds;
}

/**
 @brief Set total worlds generated
 */
void CMap2D::SetTotalWorldsGenerated(int num)
{
	totalWorlds = num;
}

/**
 @brief Update individual seed timer
 @param name (Optional) A string for item name, for reference only.
 @param dt Delta time
 @param blockNumber The seed's block number to be updated
 @param timer A float value for how long it takes for the seed to grow
 */
void CMap2D::UpdateSeed(string itemName, double dt, int blockNumber, float timer)
{
	for (unsigned int uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
	{
		for (unsigned int uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
		{
			if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == blockNumber)
			{
				arrMapInfo[uiCurLevel][uiRow][uiCol].timer += dt;
				if (arrMapInfo[uiCurLevel][uiRow][uiCol].timer > timer)
				{
					arrMapInfo[uiCurLevel][uiRow][uiCol].value = blockNumber + 1;
					SetSaveMapInfo(uiRow, uiCol, arrMapInfo[uiCurLevel][uiRow][uiCol].value, false, true);
					arrMapInfo[uiCurLevel][uiRow][uiCol].timer = 0;
				}
			}
		}
	}
}

/**
@brief Get block type
@param blockNumber An int for block number
*/
CMap2D::BLOCK_TYPE CMap2D::GetBlockType(int blockNumber)
{
	if (blockNumber >= 1 && blockNumber < 100)
		return BLOCKS;
	else if (blockNumber >= 100 && blockNumber < 200)
		return TREES;
	else if (blockNumber >= 200 && blockNumber < 300)
		return BACKGROUND_BLOCKS;
	else if (blockNumber >= 300 && blockNumber < 400)
		return COLLECTABLES;
	else
		return TOTAL_BLOCK_TYPE;
}

/**
 @brief Find a path
 */
std::vector<glm::i32vec2> CMap2D::PathFind(const glm::i32vec2& startPos, const glm::i32vec2& targetPos, HeuristicFunction heuristicFunc, int weight)
{
	// Check if the startPos and targetPost are blocked
	if (isBlocked(startPos.y, startPos.x) ||
		(isBlocked(targetPos.y, targetPos.x)))
	{
		//cout << "Invalid start or target position." << endl;
		// Return an empty path
		std::vector<glm::i32vec2> path;
		return path;
	}

	// Set up the variables and lists
	m_startPos = startPos;
	m_targetPos = targetPos;
	m_weight = weight;
	m_heuristic = std::bind(heuristicFunc, _1, _2, _3);

	// Reset AStar lists
	ResetAStarLists();

	// Add the start pos to 2 lists
	m_cameFromList[ConvertTo1D(m_startPos)].parent = m_startPos;
	m_openList.push(Grid(m_startPos, 0));

	unsigned int fNew, gNew, hNew;
	glm::i32vec2 currentPos;

	// Start the path finding...
	while (!m_openList.empty())
	{
		// Get the node with the least f value
		currentPos = m_openList.top().pos;
		//cout << endl << "*** New position to check: " << currentPos.x << ", " << currentPos.y << endl;
		//cout << "*** targetPos: " << m_targetPos.x << ", " << m_targetPos.y << endl;

		// If the targetPos was reached, then quit this loop
		if (currentPos == m_targetPos)
		{
			//cout << "=== Found the targetPos: " << m_targetPos.x << ", " << m_targetPos.y << endl;
			while (m_openList.size() != 0)
				m_openList.pop();
			break;
		}

		m_openList.pop();
		m_closedList[ConvertTo1D(currentPos)] = true;

		// Check the neighbors of the current node
		for (unsigned int i = 0; i < m_nrOfDirections; ++i)
		{
			const auto neighborPos = currentPos + m_directions[i];
			const auto neighborIndex = ConvertTo1D(neighborPos);

			//cout << "\t#" << i << ": Check this: " << neighborPos.x << ", " << neighborPos.y << ":\t";
			if (!isValid(neighborPos) ||
				isBlocked(neighborPos.y, neighborPos.x) ||
				m_closedList[neighborIndex] == true)
			{
				//cout << "This position is not valid. Going to next neighbour." << endl;
				continue;
			}

			gNew = m_cameFromList[ConvertTo1D(currentPos)].g + 1;
			hNew = m_heuristic(neighborPos, m_targetPos, m_weight);
			fNew = gNew + hNew;

			if (m_cameFromList[neighborIndex].f == 0 || fNew < m_cameFromList[neighborIndex].f)
			{
				//cout << "Adding to Open List: " << neighborPos.x << ", " << neighborPos.y;
				//cout << ". [ f : " << fNew << ", g : " << gNew << ", h : " << hNew << "]" << endl;
				m_openList.push(Grid(neighborPos, fNew));
				m_cameFromList[neighborIndex] = { neighborPos, currentPos, fNew, gNew, hNew };
			}
			else
			{
				//cout << "Not adding this" << endl;
			}
		}
		//system("pause");
	}

	return BuildPath();
}

/**
 @brief Build a path
 */
std::vector<glm::i32vec2> CMap2D::BuildPath() const
{
	std::vector<glm::i32vec2> path;
	auto currentPos = m_targetPos;
	auto currentIndex = ConvertTo1D(currentPos);

	while (!(m_cameFromList[currentIndex].parent == currentPos))
	{
		path.push_back(currentPos);
		currentPos = m_cameFromList[currentIndex].parent;
		currentIndex = ConvertTo1D(currentPos);
	}

	// If the path has only 1 entry, then it is the the target position
	if (path.size() == 1)
	{
		// if m_startPos is next to m_targetPos, then having 1 path point is OK
		if (m_nrOfDirections == 4)
		{
			if (abs(m_targetPos.y - m_startPos.y) + abs(m_targetPos.x - m_startPos.x) > 1)
				path.clear();
		}
		else
		{
			if (abs(m_targetPos.y - m_startPos.y) + abs(m_targetPos.x - m_startPos.x) > 2)
				path.clear();
			else if (abs(m_targetPos.y - m_startPos.y) + abs(m_targetPos.x - m_startPos.x) > 1)
				path.clear();
		}
	}
	else
		std::reverse(path.begin(), path.end());

	return path;
}

/**
 @brief Toggle the checks for diagonal movements
 */
void CMap2D::SetDiagonalMovement(const bool bEnable)
{
	m_nrOfDirections = (bEnable) ? 8 : 4;
}

/**
 @brief Print out the details about this class instance in the console
 */
void CMap2D::PrintSelf(void) const
{
	/*cout << endl << "AStar::PrintSelf()" << endl;

	for (unsigned uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		cout << "Level: " << uiLevel << endl;
		for (unsigned uiRow = 0; uiRow < cSettings->NUM_TILES_YAXIS; uiRow++)
		{
			for (unsigned uiCol = 0; uiCol < cSettings->NUM_TILES_XAXIS; uiCol++)
			{
				cout.fill('0');
				cout.width(3);
				cout << arrMapInfo[uiLevel][uiRow][uiCol].value;
				if (uiCol != cSettings->NUM_TILES_XAXIS - 1)
					cout << ", ";
				else
					cout << endl;
			}
		}
	}*/

	/*cout << "m_openList: " << m_openList.size() << endl;
	cout << "m_cameFromList: " << m_cameFromList.size() << endl;
	cout << "m_closedList: " << m_closedList.size() << endl;

	cout << "===== AStar::PrintSelf() =====" << endl;*/
}

/**
 @brief Check if a position is valid
 */
bool CMap2D::isValid(const glm::i32vec2& pos) const
{
	//return (pos.x >= 0) && (pos.x < m_dimensions.x) &&
	//	(pos.y >= 0) && (pos.y < m_dimensions.y);
	return (pos.x >= 0) && (pos.x < cSettings->NUM_TILES_XAXIS) &&
		(pos.y >= 0) && (pos.y < cSettings->NUM_TILES_YAXIS);
}

/**
 @brief Check if a grid is blocked
 */
bool CMap2D::isBlocked(const unsigned int uiRow, const unsigned int uiCol, const bool bInvert) const
{
	if (bInvert == true)
	{
		if ((arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value >= 100) &&
			(arrMapInfo[uiCurLevel][cSettings->NUM_TILES_YAXIS - uiRow - 1][uiCol].value < 200))
			return true;
		else
			return false;
	}
	else
	{
		if ((arrMapInfo[uiCurLevel][uiRow][uiCol].value >= 100) &&
			(arrMapInfo[uiCurLevel][uiRow][uiCol].value < 200))
			return true;
		else
			return false;
	}
}

/**
 @brief Returns a 1D index based on a 2D coordinate using row-major layout
 */
int CMap2D::ConvertTo1D(const glm::i32vec2& pos) const
{
	//return (pos.y * m_dimensions.x) + pos.x;
	return (pos.y * cSettings->NUM_TILES_XAXIS) + pos.x;
}

/**
 @brief Delete AStar lists
 */
bool CMap2D::DeleteAStarLists(void)
{
	// Delete m_openList
	while (m_openList.size() != 0)
		m_openList.pop();
	// Delete m_cameFromList
	m_cameFromList.clear();
	// Delete m_closedList
	m_closedList.clear();

	return true;
}


/**
 @brief Reset AStar lists
 */
bool CMap2D::ResetAStarLists(void)
{
	// Delete m_openList
	while (m_openList.size() != 0)
		m_openList.pop();
	// Reset m_cameFromList
	for (int i = 0; i < m_cameFromList.size(); i++)
	{
		m_cameFromList[i].pos = glm::i32vec2(0, 0);
		m_cameFromList[i].parent = glm::i32vec2(0, 0);
		m_cameFromList[i].f = 0;
		m_cameFromList[i].g = 0;
		m_cameFromList[i].h = 0;
	}
	// Reset m_closedList
	for (int i = 0; i < m_closedList.size(); i++)
	{
		m_closedList[i] = false;
	}

	return true;
}


/**
 @brief manhattan calculation method for calculation of h
 */
unsigned int heuristic::manhattan(const glm::i32vec2& v1, const glm::i32vec2& v2, int weight)
{
	glm::i32vec2 delta = v2 - v1;
	return static_cast<unsigned int>(weight * (delta.x + delta.y));
}

/**
 @brief euclidean calculation method for calculation of h
 */
unsigned int heuristic::euclidean(const glm::i32vec2& v1, const glm::i32vec2& v2, int weight)
{
	glm::i32vec2 delta = v2 - v1;
	return static_cast<unsigned int>(weight * sqrt((delta.x * delta.x) + (delta.y * delta.y)));
}