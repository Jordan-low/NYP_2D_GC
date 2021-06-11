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


	//// set up vertex data (and buffer(s)) and configure vertex attributes
	//float vertices[] = {	// positions          // texture coords
	//	(cSettings->TILE_WIDTH / 2.0f), (cSettings->TILE_HEIGHT / 2.0f), 0.0f, 1.0f, 1.0f, // top right
	//	(cSettings->TILE_WIDTH / 2.0f), -(cSettings->TILE_HEIGHT / 2.0f), 0.0f, 1.0f, 0.0f, // bottom right
	//	-(cSettings->TILE_WIDTH / 2.0f), -(cSettings->TILE_HEIGHT / 2.0f), 0.0f, 0.0f, 0.0f, // bottom left
	//	-(cSettings->TILE_WIDTH / 2.0f), (cSettings->TILE_HEIGHT / 2.0f), 0.0f, 0.0f, 1.0f  // top left 
	//};
	//unsigned int indices[] = { 0, 1, 3, // first triangle
	//	1, 2, 3  // second triangle
	//};

	//glGenVertexArrays(1, &VAO);
	//glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	//glBindVertexArray(VAO);

	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//// position attribute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//// texture coord attribute
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

	// Load and create textures
	// Load the grass texture

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
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

	//// Load the ground texture
	//if (LoadTexture("Image/Scene2D_GroundTile.tga", 100) == false)
	//{
	//	std::cout << "Failed to load ground tile texture" << std::endl;
	//	return false;
	//}
	//// Load the tree texture
	//if (LoadTexture("Image/Scene2D_TreeTile.tga", 101) == false)
	//{
	//	std::cout << "Failed to load tree tile texture" << std::endl;
	//	return false;
	//}


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
			transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			transform = glm::translate(transform, glm::vec3(cSettings->ConvertIndexToUVSpace(cSettings->x, uiCol, false, 0) + mapOffset.x,
															cSettings->ConvertIndexToUVSpace(cSettings->y, uiRow, true, 0),
															0.0f));
			//transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

			// Update the shaders with the latest transform
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			// Render a tile
			RenderTile(uiRow, uiCol);
		}
	}
}

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

void CMap2D::SetSaveMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert)
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
	
	// Check if the sizes of CSV data matches the declared arrMapInfo sizes
	if ((cSettings->NUM_TILES_XAXIS != (unsigned int)doc.GetColumnCount()) ||
		(cSettings->NUM_TILES_YAXIS != (unsigned int)doc.GetRowCount()))
	{
		cout << "Sizes of CSV map does not match declared arrMapInfo sizes." << endl;
		return false;
	}

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

	// Check if the sizes of CSV data matches the declared arrMapInfo sizes
	if ((cSettings->NUM_TILES_XAXIS != (unsigned int)doc.GetColumnCount()) ||
		(cSettings->NUM_TILES_YAXIS != (unsigned int)doc.GetRowCount()))
	{
		cout << "Sizes of CSV map does not match declared arrMapInfo sizes." << endl;
		return false;
	}

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

	for (int i = 3; i < 100; i++)
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
	if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == 200)
		arrMapInfo[uiCurLevel][uiRow][uiCol].value = 0;

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
					SetSaveMapInfo(uiRow, uiCol, arrMapInfo[uiCurLevel][uiRow][uiCol].value, false);
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
