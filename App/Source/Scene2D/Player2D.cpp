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

	//Init player colour
	playerColour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

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
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	i32vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	//CS: Create the Quad Mesh using the mesh builder
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the player texture
	if (LoadTexture("Image/scene2d_player.png", iTextureID) == false)
	{
		std::cout << "Failed to load player tile texture" << std::endl;
		return false;
	}

	cInventoryManager = CInventoryManager::GetInstance();

	//add dirt item
	cInventoryItem = cInventoryManager->Add("DirtSeed", "Image/DirtSeed.png", 64, 10);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	//add dirt item
	cInventoryItem = cInventoryManager->Add("DirtBlock", "Image/DirtBlock.png", 64, 10);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	cInventoryItem = cInventoryManager->Add("GrassSeed", "Image/GrassSeed.png", 64, 10);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	cInventoryItem = cInventoryManager->Add("GrassBlock", "Image/GrassBlock.png", 64, 10);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	return true;
}

/**
 @brief Update this instance
 */
void CPlayer2D::Update(const double dElapsedTime)
{
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
			if (!CollisionEnd(LEFT))
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
			if (!Collision(LEFT))
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
		playerColour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_D))
	{
		if (reachLeftEnd && i32vec2Index.x > 15)
		{
			reachLeftEnd = false;
		}

		if (reachLeftEnd || reachRightEnd)
		{
			if (!CollisionEnd(RIGHT))
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
			if (!Collision(RIGHT))
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
		playerColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
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
bool CPlayer2D::CheckPosition(DIRECTION eDirection)
{
	if (reachLeftEnd || reachRightEnd)
	{
		if (eDirection == LEFT)
		{
			// If the new position is fully within a row, then check this row only
			if (i32vec2NumMicroSteps.y == 0)
			{
				// If the grid is not accessible, then return false
				if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) > 0 && 
					cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x - 1) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x - 1) > 0) && 
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x - 1) < 100))
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
				if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) > 0 && 
					cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) > 0) && 
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) < 100))
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
				if (cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) > 0 && 
					cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (i32vec2NumMicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) < 100))
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
				if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) > 0 &&
					cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (i32vec2NumMicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) > 0) && 
					(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) < 100))
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
				if (cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) > 0 &&
					cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x - 1) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x - 2) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x - 2) < 100))
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
				if (cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) > 0 &&
					cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 rows, then check both rows as well
			else if (i32vec2NumMicroSteps.y != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, cMap2D->mapOffset.x - (int)playerOffset.x + 1) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x + 2) > 0) && 
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cMap2D->mapOffset.x - (int)playerOffset.x + 2) < 100))
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
				if (cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0 && 
					cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (cMap2D->mapOffset_MicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) > 0) && 
						(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) < 100))
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
				if (cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0 && 
					cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100)
				{
					return false;
				}
			}
			// If the new position is between 2 columns, then check both columns as well
			else if (cMap2D->mapOffset_MicroSteps.x != 0)
			{
				// If the 2 grids are not accessible, then return false
				if ((cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0) && 
					(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100) ||

					(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) > 0) &&
					(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) < 100))
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
 @brief Let player interact with the map. You can add collectibles such as powerups and health here.
 */
void CPlayer2D::Harvest(int x, int y)
{
	std::cout << "BLOCK: " << (cMap2D->GetMapInfo(y, x, false)) << std::endl;
	switch (cMap2D->GetMapInfo(y, x, false))
	{
	case 101:
		cInventoryItem = cInventoryManager->GetItem("GrassBlock");
		cInventoryItem->Add(rand() % 4 + 1);
		break;
	case 103:
		cInventoryItem = cInventoryManager->GetItem("DirtBlock");
		cInventoryItem->Add(rand() % 4 + 1);
		break;
	case 2:
		cInventoryItem = cInventoryManager->GetItem("GrassSeed");
		cInventoryItem->Add(rand() % 2 + 1);
		break;
	case 3:
		cInventoryItem = cInventoryManager->GetItem("DirtSeed");
		cInventoryItem->Add(rand() % 2 + 1);
		break;
	default:
		break;
	}
}

bool CPlayer2D::CheckQuantity(string itemName)
{
	cInventoryItem = cInventoryManager->GetItem(itemName);
	if (cInventoryItem->CheckAvailableQuantity())
		return true;
	return false;
}

void CPlayer2D::ReduceQuantity(string itemName)
{
	if (itemName == "none")
		return;
	cInventoryItem = cInventoryManager->GetItem(itemName);
	cInventoryItem->Remove(1);
}

int CPlayer2D::ConvertItemNameToBlockNumber(string itemName)
{
	if (itemName == "Bedrock")
		return 1;
	else if (itemName == "GrassBlock")
		return 2;
	else if (itemName == "DirtBlock")
		return 3;
	else if (itemName == "GrassSeed")
		return 100;
	else if (itemName == "DirtSeed")
		return 102;
	else
		return 0;


	//switch (itemName)
	//{
	//case 1:
	//	return "Bedrock";
	//	break;
	//case 2:
	//	return "GrassBlock";
	//	break;
	//case 3:
	//	return "DirtBlock";
	//	break;
	//case 100:
	//	return "GrassSeed";
	//	break;
	//case 102:
	//	return "DirtSeed";
	//	break;
	//default:
	//	return "none";
	//	break;
	//}
}

//Update Jump or Fall
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
			if (CheckPosition(UP) == false)
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
			if (CheckPosition(DOWN) == false)
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


bool CPlayer2D::IsMidAir()
{
	//if player is at the bottom row, skip
	if (i32vec2Index.y == 0)
		return false;
	
	//check if the tile below the player's current pos is empty
	if ((i32vec2NumMicroSteps.x == 0) && (cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x) == 0))
		return true;

	return false;
}

bool CPlayer2D::CollisionEnd(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) > 0 && 
				cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) < 100)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) > 0) && 
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) < 100) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) > 0) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) < 100))
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
			if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) > 0 && cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) < 100)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) > 0) &&
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) < 100) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) > 0) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) < 100))
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
			if (cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) > 0)
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) > 0) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x) < 100) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) > 0) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x - (int)playerOffset.x + 1) < 100))
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
			if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) > 0)
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) > 0) &&
				((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x) < 100) ||

				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) > 0) && 
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x - (int)playerOffset.x + 1) < 100)))
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

bool CPlayer2D::Collision(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) > 0 && 
				cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) < 100)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) > 0) &&
				(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 1) < 100) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 2) > 0) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x - 2) < 100))
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
			if (cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) > 0 && 
				cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) < 100)
			{
				return true;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) > 0) && 
				(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) < 100) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 2) > 0) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 2) < 100))
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
			if (cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0 && 
				cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100)
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (cMap2D->mapOffset_MicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0) && 
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100) ||

				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) > 0) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) < 100))
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
			if (cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0 && 
				cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100)
			{
				return true;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (cMap2D->mapOffset_MicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) > 0) ||
				(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) > 0) && 

				(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x) < 100) ||
				(cMap2D->GetMapInfo(i32vec2Index.y, cSettings->TILE_RATIO_XAXIS / 2 - (int)playerOffset.x + 1) < 100))
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

void CPlayer2D::ResetPosition()
{
	//reset player pos to door
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	cMap2D->FindValue(201, uiRow, uiCol);

	i32vec2Index = glm::i32vec2(uiCol, uiRow);
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);
	playerOffset = glm::vec2(0.0f);

	reachLeftEnd = true;
	reachRightEnd = false;
	reachTopEnd = false;
	reachBotEnd = false;
	doubleJumpReady = false;

	//reset map offsets
	cMap2D->mapOffset = glm::vec2(0.0f);
	cMap2D->mapOffset_MicroSteps = glm::vec2(0.0f);
}

void CPlayer2D::UpdateMouse(MOUSE_CLICK mouseClick, double x, double y, string itemName)
{
	if (itemName == "")
		return;

	int blockNumber = ConvertItemNameToBlockNumber(itemName);

	switch (mouseClick)
	{
	case MOUSE_LEFT:
		//do not allow for blocks to get replaced.
		if (cMap2D->GetMapInfo(y, x, false) != 0)
			return;

		//check if player has enough block quantity
		if (CheckQuantity(itemName))
		{
			//set the tile to be the block
			cMap2D->SetSaveMapInfo(y, x, blockNumber, false);
			//reduce block quantity
			ReduceQuantity(itemName);
		}
		break;
	case MOUSE_RIGHT:
		//break blocks or trees
		Harvest(x, y);
		//set tile to be 0
		cMap2D->SetSaveMapInfo(y, x, 0, false);
		break;
	}
}
