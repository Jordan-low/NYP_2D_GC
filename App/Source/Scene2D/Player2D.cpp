/**
 Player2D
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Player2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"

// Include MeshBuilder
#include "Primitives/MeshBuilder.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::CPlayer2D(void)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, animatedSprites(NULL)
	, playerColour(glm::vec4(1.0f))
	, playerOffset(glm::vec2(0.0f))
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	i32vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::~CPlayer2D(void)
{
	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// We won't delete this since it was created elsewhere
	cInventoryManager = NULL;


	// Delete CAnimation Sprites
	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool CPlayer2D::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();

	health = 100.f;

	//Init player colour
	playerColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//CS:: Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(3, 3,
		cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("idle", 0, 2);
	animatedSprites->AddAnimation("right", 3, 5);
	animatedSprites->AddAnimation("left", 6, 8);

	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("idle", -1, 1.0f);

	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(201, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	//cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	i32vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	//CS: Create the Quad Mesh using the mesh builder
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the player texture
	if (LoadTexture("Image/Characters/player.png", iTextureID) == false)
	{
		std::cout << "Failed to load player tile texture" << std::endl;
		return false;
	}

	cInventoryManager = CInventoryManager::GetInstance();
	//add dirt item
	cInventoryItem = cInventoryManager->Add("DirtSeed", "Image/Items/DirtSeed.png", 64, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	//add dirt item
	cInventoryItem = cInventoryManager->Add("DirtBlock", "Image/Blocks/DirtBlockUI.png", 64, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	cInventoryItem = cInventoryManager->Add("GrassSeed", "Image/Items/GrassSeed.png", 64, 10);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	cInventoryItem = cInventoryManager->Add("GrassBlock", "Image/Blocks/GrassBlock.png", 64, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	cInventoryItem = cInventoryManager->Add("Cheese", "Image/Items/Cheese.png", 32, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	cInventoryItem = cInventoryManager->Add("Stone", "Image/Items/Stone.png", 64, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	//add selector
	cInventoryItem = cInventoryManager->Add("Selector", "Image/UI/selector.png", 1, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	//block items
	itemList.push_back(make_pair("Bedrock", 1));
	itemList.push_back(make_pair("GrassBlock", 2));
	itemList.push_back(make_pair("DirtBlock", 3));
	itemList.push_back(make_pair("Chest", 4));
	itemList.push_back(make_pair("LavaBlock", 5));
	itemList.push_back(make_pair("GrassSeed", 100));
	itemList.push_back(make_pair("GrassTreeGrown", 101));
	itemList.push_back(make_pair("DirtSeed", 102));
	itemList.push_back(make_pair("DirtTreeGrown", 103));
	itemList.push_back(make_pair("Stone", 301));
	itemList.push_back(make_pair("Cheese", 302));
	
	//for seeds and tree blocks maps
	MapOfBlocksToSeeds.insert(make_pair("DirtTreeGrown", "DirtBlock"));
	MapOfBlocksToSeeds.insert(make_pair("GrassTreeGrown", "GrassBlock"));

	MapOfTreesToBlocks.insert(make_pair("GrassSeed", "GrassTreeGrown"));
	MapOfTreesToBlocks.insert(make_pair("DirtSeed", "DirtTreeGrown"));

	for (int i = 0; i < 5; i++)
		cInventoryManager->CycleThroughInventory();

	cInventoryManager->currentItem = cInventoryManager->inventoryArray[0];

	return true;
}

/**
 @brief Update this instance
 */
void CPlayer2D::Update(const double dElapsedTime)
{
	if (reachLeftEnd || reachRightEnd)
		playerPosition = glm::vec2(i32vec2Index.x - (int)playerOffset.x, i32vec2Index.y);
	else
		playerPosition = glm::vec2(cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x, i32vec2Index.y);

	CollectChest(4, 4);
	CollectItem(301, 302);
	CollideDamageBlock(dElapsedTime, 5, 5);
	SetInventorySelector();

	// Store the old position
	i32vec2OldIndex = i32vec2Index;

	// Get keyboard updates
	if (cKeyboardController->IsKeyDown(GLFW_KEY_A))
	{
		if (reachRightEnd && i32vec2Index.x < 17)
		{
			reachRightEnd = false;
		}

		if (reachRightEnd || reachLeftEnd)
		{
			if (!CollisionEnd(LEFT, 1, 99))
			{
				const int iOldIndex = i32vec2Index.x;
				if (i32vec2Index.x >= 0)
				{
					i32vec2NumMicroSteps.x--;
					if (i32vec2NumMicroSteps.x < 0)
					{
						i32vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS);
						i32vec2Index.x--;
					}
				}
			}
			if (reachLeftEnd)
				playerOffset.x = 0;
		}
		else
		{
			i32vec2NumMicroSteps.x = 0;
			if (!Collision(LEFT, 1, 99))
			{
				cMap2D->mapOffset_MicroSteps.x += cSettings->TILE_WIDTH / 2;
				if (cMap2D->mapOffset_MicroSteps.x >= cSettings->TILE_WIDTH)
				{
					cMap2D->mapOffset.x += cSettings->TILE_WIDTH;
					cMap2D->mapOffset_MicroSteps.x = 0;
					playerOffset.x++;
				}
			}
		}
		Constraint(LEFT);

		/*if (!CheckPosition(LEFT))
		{
			i32vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}*/

		if (IsMidAir() && cPhysics2D.GetStatus() != CPhysics2D::STATUS::JUMP)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		//CS: Play left animation
		animatedSprites->PlayAnimation("left", -1, 1.0f);
		//playerColour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_D))
	{
		if (reachLeftEnd && i32vec2Index.x > 15)
		{
			reachLeftEnd = false;
		}

		if (reachLeftEnd || reachRightEnd)
		{
			if (!CollisionEnd(RIGHT, 1, 99))
			{
				const int iOldIndex = i32vec2Index.x;
				if (i32vec2Index.x < cSettings->TILE_RATIO_XAXIS - 1)
				{
					i32vec2NumMicroSteps.x++;
					if (i32vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
					{
						i32vec2NumMicroSteps.x = 0;
						i32vec2Index.x++;
					}
				}
			}
			if (reachRightEnd)
				playerOffset.x = (float)cSettings->TILE_RATIO_XAXIS - cSettings->NUM_TILES_XAXIS;
		}
		else
		{
			i32vec2NumMicroSteps.x = 0;
			if (!Collision(RIGHT, 1, 99))
			{
				cMap2D->mapOffset_MicroSteps.x -= cSettings->TILE_WIDTH / 2;
				if (cMap2D->mapOffset_MicroSteps.x <= -cSettings->TILE_WIDTH)
				{
					cMap2D->mapOffset.x -= cSettings->TILE_WIDTH;
					cMap2D->mapOffset_MicroSteps.x = 0;
					playerOffset.x--;
				}
			}
		}
		Constraint(RIGHT);

		/*if (!CheckPosition(RIGHT))
		{
			i32vec2NumMicroSteps.x = 0;
		}*/

		if (IsMidAir() && cPhysics2D.GetStatus() != CPhysics2D::STATUS::JUMP)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		//CS: Play right animation
		animatedSprites->PlayAnimation("right", -1, 1.0f);
		//playerColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		animatedSprites->PlayAnimation("idle", -1, 1.0f);
	}
	/*if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
	{
		const int iOldIndex = i32vec2Index.y;
		i32vec2NumMicroSteps.y++;
		if (i32vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
		{
			i32vec2NumMicroSteps.y = 0;
			i32vec2Index.y++;
		}
		if (!Collision(UP))
		{
			cMap2D->mapOffset_MicroSteps.y -= cSettings->TILE_HEIGHT/2;
			if (cMap2D->mapOffset_MicroSteps.y <= -cSettings->TILE_HEIGHT)
			{
				cMap2D->mapOffset.y -= cSettings->TILE_HEIGHT;
				cMap2D->mapOffset_MicroSteps.y = 0;
				playerOffset.y--;
			}
		}

		if (reachBotEnd && i32vec2Index.y < 18)
		{
			reachBotEnd = false;
		}
		if (reachTopEnd || reachBotEnd)
		{

				if (reachTopEnd)
			playerOffset.y = 0;
		}
		else
		{
			cMap2D->mapOffset_MicroSteps.y -= 0.03125f;
			cout << "OFFSET: " << cMap2D->mapOffset_MicroSteps.y << endl;
			if (cMap2D->mapOffset_MicroSteps.y <= -0.0625f)
			{
				cMap2D->mapOffset.y -= 0.0625f;
				cMap2D->mapOffset_MicroSteps.y = 0;
				playerOffset.y--;
			}
		}
		
		Constraint(UP);

		// If the new position is not feasible, then revert to old position
		if (!CheckPosition(UP))
		{
			i32vec2NumMicroSteps.y = 0;
		}

		//CS: Play up animation
		animatedSprites->PlayAnimation("idle", -1, 1.0f);
		playerColour = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
	{
		const int iOldIndex = i32vec2Index.y;
		if (i32vec2Index.y >= 0)
		{
			i32vec2NumMicroSteps.y--;
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				i32vec2Index.y--;
			}
		}
		if (!Collision(DOWN))
		{
			cMap2D->mapOffset_MicroSteps.y += cSettings->TILE_HEIGHT / 2;
			if (cMap2D->mapOffset_MicroSteps.y >= cSettings->TILE_HEIGHT)
			{
				cMap2D->mapOffset.y += cSettings->TILE_HEIGHT;
				cMap2D->mapOffset_MicroSteps.y = 0;
				playerOffset.y++;
			}
		}

		if (reachTopEnd && i32vec2Index.y > 15)
		{
			reachTopEnd = false;
		}

		if (reachTopEnd || reachBotEnd)
		{
					if (reachBotEnd)
			playerOffset.y = (float)cSettings->TILE_RATIO_YAXIS - cSettings->NUM_TILES_YAXIS;
		}
		else
		{
			cMap2D->mapOffset_MicroSteps.y += 0.03125f;
			if (cMap2D->mapOffset_MicroSteps.y >= 0.0625f)
			{
				cMap2D->mapOffset.y += 0.0625f;
				cMap2D->mapOffset_MicroSteps.y = 0;
				playerOffset.y++;
			}
			
		}
		
		Constraint(DOWN);

		if (!CheckPosition(DOWN))
		{
			i32vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.y = 0;
		}

		//CS: Play down animation
		animatedSprites->PlayAnimation("idle", -1, 1.0f);
		playerColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	}*/
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_SPACE))
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			doubleJumpReady = true;
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 2.0f));
		}
		else if (doubleJumpReady)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 2.0f));
			doubleJumpReady = false;
		}
	}

	UpdateJumpFall(dElapsedTime);
	//InteractWithMap();

	//CS: Update sprite animation
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, i32vec2Index.x, false, i32vec2NumMicroSteps.x*cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, i32vec2Index.y, false, i32vec2NumMicroSteps.y*cSettings->MICRO_STEP_YAXIS);

}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlayer2D::PreRender(void)
{
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CPlayer2D::Render(void)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colourLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtime_color");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
													vec2UVCoordinate.y,
													0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colourLoc, 1, glm::value_ptr(playerColour));

	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	// Render the tile
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//CS: Render the tile
	//quadMesh->Render();
	//CS: Render the animation
	animatedSprites->Render();
	glBindVertexArray(0);

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlayer2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Load a texture, assign it a code and store it in MapOfTextureIDs.
@param filename A const char* variable which contains the file name of the texture
*/
bool CPlayer2D::LoadTexture(const char* filename, GLuint& iTextureID)
{
	// Variables used in loading the texture
	int width, height, nrChannels;
	
	// texture 1
	// ---------
	glGenTextures(1, &iTextureID);
	glBindTexture(GL_TEXTURE_2D, iTextureID);
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
 @brief Constraint the player's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CPlayer2D::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (cMap2D->mapOffset.x > 0)
		{
			reachLeftEnd = true;
			cMap2D->mapOffset.x = 0;
			cMap2D->mapOffset_MicroSteps.x = 0;
		}
		if (reachLeftEnd)
		{
			if (i32vec2Index.x < 0)
			{
				//cMap2D->offset += 0.065f * 0.1;
				i32vec2Index.x = 0;
				i32vec2NumMicroSteps.x = 0;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		float maxOffset = ((int)cSettings->TILE_RATIO_XAXIS - (int)cSettings->NUM_TILES_XAXIS) * cSettings->TILE_WIDTH;
		if (cMap2D->mapOffset.x <= maxOffset)
		{
			reachRightEnd = true;
			cMap2D->mapOffset.x = maxOffset;
			cMap2D->mapOffset_MicroSteps.x = 0;
		}

		if (reachRightEnd)
		{
			if (i32vec2Index.x >= ((int)cSettings->NUM_TILES_XAXIS) - 1)
			{
				i32vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
				i32vec2NumMicroSteps.x = 0;
			}
		}
	}
	else if (eDirection == UP)
	{
		if (i32vec2Index.y >= ((int)cSettings->TILE_RATIO_YAXIS) - 1)
		{
			i32vec2Index.y = ((int)cSettings->TILE_RATIO_YAXIS) - 1;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (i32vec2Index.y < 0)
		{
			i32vec2Index.y = 0;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CPlayer2D::Constraint: Unknown direction." << endl;
	}
	//if (eDirection == LEFT)
	//{
	//	if (i32vec2Index.x < 0)
	//	{
	//		//cMap2D->offset += 0.065f * 0.1;
	//		i32vec2Index.x = 0;
	//		i32vec2NumMicroSteps.x = 0;
	//	}
	//}
	//else if (eDirection == RIGHT)
	//{
	//	if (i32vec2Index.x >= ((int)cSettings->NUM_TILES_XAXIS) - 1)
	//	{
	//		//cMap2D->offset -= 0.065f * 0.1;
	//		i32vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
	//		i32vec2NumMicroSteps.x = 0;
	//	}
	//}
	//else if (eDirection == UP)
	//{
	//	if (i32vec2Index.y >= ((int)cSettings->NUM_TILES_YAXIS) - 1)
	//	{
	//		i32vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
	//		i32vec2NumMicroSteps.y = 0;
	//	}
	//}
	//else if (eDirection == DOWN)
	//{
	//	if (i32vec2Index.y < 0)
	//	{
	//		i32vec2Index.y = 0;
	//		i32vec2NumMicroSteps.y = 0;
	//	}
	//}
	//else
	//{
	//	cout << "CPlayer2D::Constraint: Unknown direction." << endl;
	//}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CPlayer2D::CheckPosition(DIRECTION eDirection, int minIndex, int maxIndex)
{
	if (reachLeftEnd || reachRightEnd)
	{
		if (eDirection == LEFT)
		{
			// If the new position is fully within a row, then check this row only
			if (i32vec2NumMicroSteps.y == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) >= minIndex && 
					cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x - 1) >= minIndex) && 
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x - 1) <= maxIndex))
				{
					return false;
				}
			}
		}
		else if (eDirection == RIGHT)
		{
			// If the new position is at the top row, then return true
			if (i32vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
			{
				i32vec2NumMicroSteps.x = 0;
				return true;
			}

			// If the new position is fully within a row, then check this row only
			if (i32vec2NumMicroSteps.y == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex && 
					cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) && 
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex))
				{
					return false;
				}
			}

		}
		else if (eDirection == UP)
		{
			// If the new position is at the top row, then return true
			if (i32vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
			{
				i32vec2NumMicroSteps.y = 0;
				return true;
			}

			// If the new position is fully within a column, then check this column only
			if (i32vec2NumMicroSteps.x == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) >= minIndex && 
					cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (i32vec2NumMicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex))
				{
					return false;
				}
			}
		}
		else if (eDirection == DOWN)
		{
			// If the new position is fully within a column, then check this column only
			if (i32vec2NumMicroSteps.x == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) >= minIndex &&
					cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (i32vec2NumMicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) && 
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex))
				{
					return false;
				}
			}
		}
		else
		{
			cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
		}

		return true;
	}
	else
	{
		if (eDirection == LEFT)
		{
			// If the new position is fully within a row, then check this row only
			if (i32vec2NumMicroSteps.y == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) >= minIndex &&
					cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x - 2) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x - 2) <= maxIndex))
				{
					return false;
				}
			}
		}
		else if (eDirection == RIGHT)
		{
			// If the new position is at the top row, then return true
			if (cMap2D->mapOffset.x >= cSettings->NUM_TILES_XAXIS - 1)
			{
				cMap2D->mapOffset_MicroSteps.x = 0;
				return true;
			}

			// If the new position is fully within a row, then check this row only
			if (i32vec2NumMicroSteps.y == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) >= minIndex &&
					cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x + 2) >= minIndex) && 
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x + 2) <= maxIndex))
				{
					return false;
				}
			}

		}
		else if (eDirection == UP)
		{
			// If the new position is at the top row, then return true
			if (i32vec2NumMicroSteps.y >= cSettings->NUM_TILES_YAXIS - 1)
			{
				i32vec2NumMicroSteps.y = 0;
				return true;
			}

			// If the new position is fully within a column, then check this column only
			if (cMap2D->mapOffset_MicroSteps.x == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex && 
					cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (cMap2D->mapOffset_MicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) >= minIndex) && 
						(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) <= maxIndex))
				{
					return false;
				}
			}
		}
		else if (eDirection == DOWN)
		{
			// If the new position is fully within a column, then check this column only
			if (cMap2D->mapOffset_MicroSteps.x == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex && 
					cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) <= maxIndex)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (cMap2D->mapOffset_MicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex) && 
					(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) <= maxIndex) ||

					(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) >= minIndex) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) <= maxIndex))
				{
					return false;
				}
			}
		}
		else
		{
			cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
		}

		return true;
	}
}

/**
 @brief Harvest fully grown trees for item blocks
 @param x An int for positionX to harvest
 @param y An int for positionY to harvest
 */
void CPlayer2D::Harvest(int x, int y)
{
	string harvestItemName = GetStringItemList(cMap2D->GetMapInfo(y, x, false));

	//get seed name or get block name for item harvested
	string dropName = "";
	bool changeToSeed = false;
	for (auto const& x : MapOfBlocksToSeeds)
	{
		if (harvestItemName == x.first)
		{
			dropName = x.second;
			break;
		}
		else if (harvestItemName == x.second)
		{
			dropName = x.first;
			changeToSeed = true;
			break;
		}
	}

	if (changeToSeed)
	{
		for (auto const& x : MapOfTreesToBlocks)
		{
			if (dropName == x.second)
			{
				dropName = x.first;
				break;
			}
		}
	}

	if (!cInventoryManager->Check(dropName))
		return;

	cInventoryItem = cInventoryManager->GetItem(dropName);
	cInventoryItem->Add(Math::RandIntMinMax(1, 4));
}

/**
 @brief Return the item block's quantity
 @param itemName A string for item block name
 */
bool CPlayer2D::CheckQuantity(string itemName)
{
	cInventoryItem = cInventoryManager->GetItem(itemName);
	if (cInventoryItem->CheckAvailableQuantity())
		return true;
	return false;
}

/**
 @brief Reduce item block quantity by a set amount
 @param itemName A string for item block name
 @param quantity An int for the amount of quantity to reduce
 */
void CPlayer2D::ReduceQuantity(string itemName, int quantity)
{
	if (itemName == "none")
		return;
	cInventoryItem = cInventoryManager->GetItem(itemName);
	cInventoryItem->Remove(quantity);
}

/**
 @brief Update player's jump and fall
 @param dElapsedTime A double for delta time
 */
void CPlayer2D::UpdateJumpFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.AddElapsedTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current i32vec2Index.y
		int iIndex_YAxis_OLD = i32vec2Index.y;

		// Translate the displacement from pixels to indices
		int iDisplacement = (int)(v2Displacement.y / cSettings->TILE_HEIGHT);
		int iDisplacement_MicroSteps = (int)((v2Displacement.y * cSettings->iWindowHeight) - iDisplacement) /
			(int)cSettings->NUM_STEPS_PER_TILE_YAXIS;
		if (iDisplacement_MicroSteps > 0)
		{
			iDisplacement++;
		}

		// Update the indices
		i32vec2Index.y += iDisplacement;
		i32vec2NumMicroSteps.y = 0;

		// Constraint the player's position within the screen boundary
		Constraint(UP);
		cout << i32vec2Index.y << endl;

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop jump if so.
		int iIndex_YAxis_Proposed = i32vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i <= iIndex_YAxis_Proposed; i++)
		{
			// Change the player's index to the current i value
			i32vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(UP, 1, 99) == false)
			{
				// Set the Physics to fall status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
				break;
			}
		}

		// If the player is still jumping and the initial velocity has reached zero or below zero, 
		// then it has reach the peak of its jump
		if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::JUMP) && (cPhysics2D.GetInitialVelocity().y <= 0.0f))
		{
			// Set status to fall
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.AddElapsedTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update();
		// Get the displacement from the physics engine
		glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

		// Store the current i32vec2Index.y
		int iIndex_YAxis_OLD = i32vec2Index.y;

		// Translate the displacement from pixels to indices
		int iDisplacement = (int)(v2Displacement.y / cSettings->TILE_HEIGHT);
		int iDisplacement_MicroSteps = (int)((v2Displacement.y * cSettings->iWindowHeight) - iDisplacement) /
			(int)cSettings->NUM_STEPS_PER_TILE_YAXIS;

		if (iDisplacement_MicroSteps > 0)
		{
			iDisplacement++;
		}

		// Update the indices
		i32vec2Index.y += iDisplacement;
		i32vec2NumMicroSteps.y = 0;

		// Constraint the player's position within the screen boundary
		Constraint(DOWN);

		// Iterate through all rows until the proposed row
		// Check if the player will hit a tile; stop fall if so.
		int iIndex_YAxis_Proposed = i32vec2Index.y;
		for (int i = iIndex_YAxis_OLD; i >= iIndex_YAxis_Proposed; i--)
		{
			// Change the player's index to the current i value
			i32vec2Index.y = i;
			// If the new position is not feasible, then revert to old position
			if (CheckPosition(DOWN, 1, 99) == false)
			{
				// Revert to the previous position
				if (i != iIndex_YAxis_OLD)
					i32vec2Index.y = i + 1;
				// Set the Physics to idle status
				cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
				break;
			}
		}
	}
}

/**
 @brief Check for player is in mid air
 */
bool CPlayer2D::IsMidAir()
{
	//if player is at the bottom row, skip
	if (i32vec2Index.y == 0)
		return false;
	
	//check if the tile below the player's current pos is empty
	if ((i32vec2NumMicroSteps.x == 0) && (cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x) == 0))
		return true;

	else if ((i32vec2NumMicroSteps.x == 0) && (cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x - (int)playerOffset.x) == 0))
		return true;

	return false;
}

/**
 @brief Check for player's collision at both the left and right ends
 @param eDirection An enum for player's direction
 */
bool CPlayer2D::CollisionEnd(DIRECTION eDirection, int minIndex, int maxIndex)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) >= minIndex && 
				cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) <= maxIndex)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) <= maxIndex) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) <= maxIndex))
			{
				return true;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (i32vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			i32vec2NumMicroSteps.x = 0;
			return false;
		}

		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex &&
				cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex))
			{
				return true;
			}
		}

	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (i32vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			i32vec2NumMicroSteps.y = 0;
			return false;
		}

		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) <= maxIndex))
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) <= maxIndex) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex))
			{
				return true;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x - (int)playerOffset.x) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x - (int)playerOffset.x) <= maxIndex))
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x - (int)playerOffset.x) >= minIndex) &&
				((cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x - (int)playerOffset.x) <= maxIndex) ||

				(cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x - (int)playerOffset.x + 1) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x - (int)playerOffset.x + 1) <= maxIndex)))
			{									   
				return true;
			}
		}
	}
	else
	{
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}
	return false;
}

/**
 @brief Check for player's collision in general central area
 @param eDirection An enum for player's direction
 */
bool CPlayer2D::Collision(DIRECTION eDirection, int minIndex, int maxIndex)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) >= minIndex && 
				cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) <= maxIndex)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) <= maxIndex) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 2) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 2) <= maxIndex))
			{
				return true;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (cMap2D->mapOffset.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			cMap2D->mapOffset_MicroSteps.x = 0;
			return false;
		}

		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) >= minIndex && 
				cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) <= maxIndex)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) <= maxIndex) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 2) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 2) <= maxIndex))
			{
				return true;
			}
		}

	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (i32vec2NumMicroSteps.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			i32vec2NumMicroSteps.y = 0;
			return false;
		}

		// If the new position is fully within a column, then check this column only
		if (cMap2D->mapOffset_MicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex && 
				cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) <= maxIndex)
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (cMap2D->mapOffset_MicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) <= maxIndex) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) <= maxIndex))
			{
				return true;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (cMap2D->mapOffset_MicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y - 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex && 
				cMap2D->GetMapInfo(i32vec2Index.y - 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) <= maxIndex)
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (cMap2D->mapOffset_MicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y - 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex) ||
				(cMap2D->GetMapInfo(i32vec2Index.y - 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) >= minIndex) && 
												   
				(cMap2D->GetMapInfo(i32vec2Index.y - 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) <= maxIndex) ||
				(cMap2D->GetMapInfo(i32vec2Index.y - 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) <= maxIndex))
			{
				return true;
			}
		}
	}
	else
	{
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}

	return false;
}

/**
 @brief Reset player position to the door's position
 */
void CPlayer2D::ResetPosition()
{
	//reset player pos to door
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	cMap2D->FindValue(201, uiRow, uiCol);

	i32vec2Index = glm::i32vec2(uiCol, uiRow);
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);
	playerOffset = glm::vec2(0.f);

	reachLeftEnd = true;
	reachRightEnd = false;
	reachTopEnd = false;
	reachBotEnd = false;
	doubleJumpReady = false;

	//reset map offsets
	cMap2D->mapOffset = glm::vec2(0.0f);
	cMap2D->mapOffset_MicroSteps = glm::vec2(0.0f);
}

/**
 @brief Update mouse clicks
 @param mouseClick An enum for which mouse click used
 @param dt Delta time
 @param x A double value for mouse position X
 @param y A double value for mouse position Y
 */
void CPlayer2D::UpdateMouse(MOUSE_CLICK mouseClick, double x, double y, string itemName)
{
	if (x >= cSettings->NUM_TILES_XAXIS || x < 0)
		return;
	else if (y >= cSettings->NUM_TILES_YAXIS || y < 0)
		return;

	int blockNumber = GetIntItemList(itemName);

	switch (mouseClick)
	{
	case MOUSE_LEFT:
		if (itemName == "")
			return;

		//do not allow for blocks to get replaced.
		if (cMap2D->GetMapInfo(y, x, false) != 0)
			return;

		//check if player has enough block quantity
		if (CheckQuantity(itemName))
		{

			switch (cMap2D->GetBlockType(blockNumber))
			{
			case CMap2D::BLOCK_TYPE::BLOCKS:
				if (UpdateBlockItem(blockNumber, glm::vec2(x, y)))
					ReduceQuantity(itemName, 1);
				break;
			case CMap2D::BLOCK_TYPE::TREES:
				if (UpdateTreeItem(blockNumber, glm::vec2(x, y)))
					ReduceQuantity(itemName, 1);
				break;
			case CMap2D::BLOCK_TYPE::COLLECTABLES:
				if (UpdateCollectableItem(itemName))
					ReduceQuantity(itemName, 1);
				break;
			}
		}
		break;
	case MOUSE_RIGHT:
		//break blocks or trees
		if (cMap2D->GetMapInfo(y, x, false) == 1 || cMap2D->GetMapInfo(y, x, false) == 0 || cMap2D->GetMapInfo(y, x, false) == 201)
			return;

		Harvest(x, y);

		//set tile to be 0
		cMap2D->SetSaveMapInfo(y, x, 0, false);
		break;
	}
}

bool CPlayer2D::UpdateCollectableItem(string itemName)
{
	if (itemName == "Cheese")
	{
		health += 50;
		return true;
	}
	return false;
}

bool CPlayer2D::UpdateTreeItem(int blockNumber, glm::vec2 position)
{
	int getBlockBelow = cMap2D->GetMapInfo(position.y + 1, position.x, false);
	CMap2D::BLOCK_TYPE temp = cMap2D->GetBlockType(getBlockBelow);

	if (temp == CMap2D::BLOCK_TYPE::BLOCKS)
	{
		cMap2D->SetSaveMapInfo(position.y, position.x, blockNumber, false);
		return true;
	}
	return false;
}

bool CPlayer2D::UpdateBlockItem(int blockNumber, glm::vec2 position)
{			
	//set the tile to be the block
	cMap2D->SetSaveMapInfo(position.y, position.x, blockNumber, false);
	return true;
}


/**
 @brief Update all individual seed timer
 @param dt Delta time
 */
void CPlayer2D::UpdateSeeds(double dt)
{
	cMap2D->UpdateSeed("GrassTree", dt, 100, 2);
	cMap2D->UpdateSeed("DirtTree", dt, 102, 1);
}

/**
 @brief Checks if player collects chest
 @param minIndex minimum block number to start with
 @param maxIndex maximum block number to end with
 */
void CPlayer2D::CollectChest(int minIndex, int maxIndex)
{
	bool isBesideChest = false;
	int offset = 0;

	if (reachLeftEnd || reachRightEnd)
	{
		if (CollisionEnd(LEFT, minIndex, maxIndex))
		{
			isBesideChest = true;
			offset = -1;
		}
		else if (CollisionEnd(RIGHT, minIndex, maxIndex))
		{
			isBesideChest = true;
			offset = 1;
		}
	}
	else
	{
		if (Collision(LEFT, minIndex, maxIndex))
		{
			isBesideChest = true;
			offset = -1;
		}
		else if (Collision(RIGHT, minIndex, maxIndex))
		{
			isBesideChest = true;
			offset = 1;
		}
	}
	
	if (!isBesideChest)
		return;

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_E))
	{
		cMap2D->OpenedChest(glm::vec2(playerPosition.x + offset, playerPosition.y));
	}
}

void CPlayer2D::CollideDamageBlock(double dt, int minIndex, int maxIndex)
{
	bool isBesideDamageBlock = false;

	if (reachLeftEnd || reachRightEnd)
	{
		if (CollisionEnd(LEFT, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
		else if (CollisionEnd(RIGHT, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
		if (CollisionEnd(UP, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
		else if (CollisionEnd(DOWN, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
	}
	else
	{
		if (Collision(LEFT, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
		else if (Collision(RIGHT, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
		if (Collision(LEFT, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
		else if (Collision(RIGHT, minIndex, maxIndex))
		{
			isBesideDamageBlock = true;
		}
	}

	if (isBesideDamageBlock)
		health -= 0.1f;
}

void CPlayer2D::SetInventorySelector()
{
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_TAB))
	{
		cInventoryManager->CycleThroughInventory();
	}
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_1))
	{
		if (cInventoryManager->inventoryArray[0] != nullptr)
			cInventoryManager->currentItem = cInventoryManager->inventoryArray[0];
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_2))
	{
		if (cInventoryManager->inventoryArray[1] != nullptr)
			cInventoryManager->currentItem = cInventoryManager->inventoryArray[1];
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_3))
	{
		if (cInventoryManager->inventoryArray[2] != nullptr)
			cInventoryManager->currentItem = cInventoryManager->inventoryArray[2];
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_4))
	{
		if (cInventoryManager->inventoryArray[3] != nullptr)
			cInventoryManager->currentItem = cInventoryManager->inventoryArray[3];
	}
	else if (cKeyboardController->IsKeyPressed(GLFW_KEY_5))
	{
		if (cInventoryManager->inventoryArray[4] != nullptr)
			cInventoryManager->currentItem = cInventoryManager->inventoryArray[4];
	}
	if (cKeyboardController->IsKeyPressed(GLFW_KEY_CAPS_LOCK))
	{
		for (int i = 0; i < sizeof(cInventoryManager->inventoryArray) / sizeof(*cInventoryManager->inventoryArray); i++)
		{
			if (cInventoryManager->inventoryArray[i] != nullptr)
				std::cout << "ELEMENT " << i << ": " << cInventoryManager->inventoryArray[i]->sName << std::endl;
		}
		if (cInventoryManager->currentItem != nullptr)
			std::cout << "CURRENT: " << cInventoryManager->currentItem->sName << std::endl;
	}
}

/**
 @brief Update if player has collected item from chest
 @param minIndex minimum block number to start with
 @param maxIndex maximum block number to end with
 */
void CPlayer2D::CollectItem(int minIndex, int maxIndex)
{
	bool isCollected = false;
	glm::i32vec2 itemPosition;

	if (reachLeftEnd || reachRightEnd)
	{
		if (CollisionEnd(LEFT, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
		else if (CollisionEnd(RIGHT, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
		if (CollisionEnd(UP, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
		else if (CollisionEnd(DOWN, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
	}
	else
	{
		if (Collision(LEFT, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
		else if (Collision(RIGHT, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
		if (Collision(UP, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
		else if (Collision(DOWN, minIndex, maxIndex))
		{
			isCollected = true;
			itemPosition = playerPosition;
		}
	}

	if (!isCollected)
		return;

	int itemNumber = cMap2D->GetMapInfo(itemPosition.y, itemPosition.x);

	string itemName = GetStringItemList(itemNumber);

	if (!cInventoryManager->Check(itemName))
		return;

	cMap2D->SetSaveMapInfo(itemPosition.y, itemPosition.x, 0);
	cInventoryItem = cInventoryManager->GetItem(itemName);
	cInventoryItem->Add(1);
}

/**
 @brief Get string pair from item list vector
 @param blockNumber An int for block number
 */
string CPlayer2D::GetStringItemList(int blockNumber)
{
	string itemName = "";
	for (int i = 0; i < itemList.size(); i++)
	{
		if (itemList[i].second == blockNumber)
			itemName = itemList[i].first;
	}
	return itemName;
}

/**
 @brief Get int pair from item list vector
 @param itemName A string for item name
 */
int CPlayer2D::GetIntItemList(string itemName)
{
	int blockNumber = 0;
	for (int i = 0; i < itemList.size(); i++)
	{
		if (itemList[i].first == itemName)
			blockNumber = itemList[i].second;
	}
	return blockNumber;
}

bool CPlayer2D::CheckBlockType(int blockNumber)
{
	switch (cMap2D->GetBlockType(blockNumber))
	{
	case CMap2D::BLOCK_TYPE::BLOCKS:
		break;
	case CMap2D::BLOCK_TYPE::TREES:
		break;
	}
	return false;
}

