/**
 CEnemy2D
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Enemy2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

#include "../SoundController/SoundController.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::CEnemy2D(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, animatedSprites(NULL)
	, sCurrentFSM(FSM::IDLE)
	, enemyType(ENEMY_TYPE::DEFAULT_ENEMY)
	, iFSMCounter(0)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	i32vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	i32vec2Destination = glm::i32vec2(0, 0);	// Initialise the iDestination
	i32vec2Direction = glm::i32vec2(0, 0);		// Initialise the iDirection
	i32vec2Player = glm::i32vec2(0, 0);			// Initialise the iPlayer
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::~CEnemy2D(void)
{
	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

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
bool CEnemy2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();


	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(300, uiRow, uiCol) == false)
	{
		if (cMap2D->FindValue(301, uiRow, uiCol) == false)
		{
			return false; // Unable to find the start position of the player, so quit this game
		}
		else
		{
			enemyType = ENEMY_TYPE::BOSS_ENEMY;
			health = 10.f;
			maxHealth = health;
		}
	}
	else
	{
		enemyType = ENEMY_TYPE::DEFAULT_ENEMY;
		health = 5.f;
		maxHealth = health;
	}
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Erase the value of the player in the arrMapInfo

	// Set the start position of the Player to iRow and iCol
	i32vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	switch (enemyType)
	{
	case DEFAULT_ENEMY:
		// Load the enemy2D texture
		if (LoadTexture("Image/Scene2D_EnemyTile.tga", iTextureID) == false)
		{
			std::cout << "Failed to load enemy2D tile texture" << std::endl;
			return false;
		}
		break;
	case BOSS_ENEMY:
		// Load the enemy2D texture
		if (LoadTexture("Image/Characters/Enemy111.png", iTextureID) == false)
		{
			std::cout << "Failed to load enemy2D tile texture" << std::endl;
			return false;
		}
		break;
	default:
		break;
	}

	//CS:: Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(5, 4,
		cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("runLeft", 0, 3);
	animatedSprites->AddAnimation("runRight", 4, 7);
	animatedSprites->AddAnimation("attackLeft", 8, 11);
	animatedSprites->AddAnimation("attackRight", 12, 15);
	animatedSprites->AddAnimation("idle", 16, 19);

	//CS: Play the "runLeft" animation as default
	animatedSprites->PlayAnimation("idle", -1, 1.0f);

	//CS: Init the color to white
	currentColor = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	return true;
}

/**
 @brief Update this instance
 */
void CEnemy2D::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return;

	if (i32vec2Direction.x > 0)
	{
		animatedSprites->PlayAnimation("runLeft", -1, 1.0f);
	}
	else
	{
		animatedSprites->PlayAnimation("runRight", -1, 1.0f);
	}

	if (health <= 0)
	{
		//kill enemy here
		bIsActive = false;
	}
	else if (health < maxHealth * 0.25) //red colour if health is less than 25% of max
	{
		currentColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
	}
	else if (health >= maxHealth * 0.25f && health < maxHealth * 0.5) //orange colour if health is less than 50% of max
	{
		currentColor = glm::vec4(1.0, 0.5, 0.0, 1.0);
	}
	else if (health >= maxHealth * 0.5f && health < maxHealth) //yellow colour if health is less than 1100% of max
	{
		currentColor = glm::vec4(1.0, 1.0, 0.0, 1.0);
	}
	else //normal white colour if health is less than 25% of max
	{
		currentColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
	}

	switch (enemyType)
	{
	case DEFAULT_ENEMY:
		UpdateDefaultEnemy();
		break;
	case BOSS_ENEMY:
		UpdateBossEnemy();
		break;
	}

	AttackEnemy();

	// Update Jump or Fall
	UpdateJumpFall(dElapsedTime);

	//CS: Update sprite animation
	animatedSprites->Update(dElapsedTime);

	unsigned int xOffset = 1;
	float xAxis = i32vec2Index.x;
	if (cPlayer2D->isCenter)
	{
		float displacementX = cPlayer2D->i32vec2Index.x - i32vec2Index.x;
		xAxis = cSettings->TILE_RATIO_XAXIS / 2 - displacementX;
	}

	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, xAxis, false, i32vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, i32vec2Index.y, false, i32vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemy2D::PreRender(void)
{
	if (!bIsActive)
		return;

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
void CEnemy2D::Render(void)
{
	if (!bIsActive)
		return;

	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtime_color");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
													vec2UVCoordinate.y,
													0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(currentColor));

	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	// Render the tile
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//quadMesh->Render();
	animatedSprites->Render();

	glBindVertexArray(0);

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CEnemy2D::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->i32vec2Index.x = iIndex_XAxis;
	this->i32vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CEnemy2D::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void CEnemy2D::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}


/**
@brief Load a texture, assign it a code and store it in MapOfTextureIDs.
@param filename A const char* variable which contains the file name of the texture
*/
bool CEnemy2D::LoadTexture(const char* filename, GLuint& iTextureID)
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
	unsigned char* data = cImageLoader->Load(filename, width, height, nrChannels, true);
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
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CEnemy2D::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (i32vec2Index.x < 0)
		{
			i32vec2Index.x = 0;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (i32vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			i32vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (i32vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			i32vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
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
		cout << "CEnemy2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CEnemy2D::CheckPosition(DIRECTION eDirection, int minIndex, int maxIndex)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) >= minIndex &&
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) <= maxIndex))
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) <= maxIndex) ||
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x) <= maxIndex))
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
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x + 1) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x + 1) <= maxIndex))
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x + 1) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x + 1) <= maxIndex) ||
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x + 1) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x + 1) <= maxIndex))
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
			if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x) <= maxIndex))
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x) <= maxIndex) ||
				(cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x + 1) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y + 1, i32vec2Index.x + 1) <= maxIndex))
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
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) >= minIndex) &&
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) <= maxIndex))
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x) <= maxIndex) ||
				(cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x + 1) >= minIndex) && (cMap2D->GetMapInfo(i32vec2Index.y, i32vec2Index.x + 1) <= maxIndex))
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

// Check if the enemy2D is in mid-air
bool CEnemy2D::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (i32vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((i32vec2NumMicroSteps.x == 0) &&
		(cMap2D->GetMapInfo(i32vec2Index.y - 1, i32vec2Index.x) == 0))
	{
		return true;
	}

	return false;
}

// Update Jump or Fall
void CEnemy2D::UpdateJumpFall(const double dElapsedTime)
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

		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS); //DIsplacement divide by distance for 1 microstep
		if (i32vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
		{
			i32vec2NumMicroSteps.y += iDisplacement_MicroSteps;
			if (i32vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				i32vec2NumMicroSteps.y -= cSettings->NUM_STEPS_PER_TILE_YAXIS;
				if (i32vec2NumMicroSteps.y < 0)
					i32vec2NumMicroSteps.y = 0;
				i32vec2Index.y++;
			}
		}

		// Constraint the player's position within the screen boundary
		Constraint(UP);

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
				// Align with the row
				i32vec2NumMicroSteps.y = 0;
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
		int iDisplacement_MicroSteps = (int)(v2Displacement.y / cSettings->MICRO_STEP_YAXIS);

		if (i32vec2Index.y >= 0)
		{
			i32vec2NumMicroSteps.y -= fabs(iDisplacement_MicroSteps);
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				i32vec2Index.y--;
			}
		}

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
				i32vec2NumMicroSteps.y = 0;
				break;
			}
		}
	}
}

/**
 @brief Let enemy2D interact with the player.
 */
bool CEnemy2D::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->i32vec2Index;
	
	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((i32vec2Index.x >= i32vec2PlayerPos.x - 0.9) && 
		(i32vec2Index.x <= i32vec2PlayerPos.x + 0.9))
		&& 
		((i32vec2Index.y >= i32vec2PlayerPos.y - 0.9) &&
		(i32vec2Index.y <= i32vec2PlayerPos.y + 0.9))
		&& 
		sCurrentFSM == ATTACK)
	{
		if (cPlayer2D->isSurvival)
		{
			CSoundController::GetInstance()->PlaySoundByName("playerAttack");
			cPlayer2D->health -= 10.f;
			cout << "Gotcha!" << endl;
			iFSMCounter = 0;
			//// Since the player has been caught, then reset the FSM
		}
		sCurrentFSM = PATROL;
		UpdateDirection();
		return true;
	}
	return false;
}

/**
 @brief Update the enemy's direction.
 */
void CEnemy2D::UpdateDirection(void)
{
	if (cPlayer2D->isSurvival)
		i32vec2Destination = cPlayer2D->i32vec2Index;
	else
		i32vec2Destination = glm::i32vec2(Math::RandIntMinMax(0, cSettings->NUM_TILES_XAXIS), i32vec2Index.y);

	// Calculate the direction between enemy2D and player2D
	i32vec2Direction = i32vec2Destination - i32vec2Index;

	// Calculate the distance between enemy2D and player2D
	float fDistance = cPhysics2D.CalculateDistance(i32vec2Index, i32vec2Destination);
	if (fDistance >= 0.01f)
	{
		// Calculate direction vector.
		// We need to round the numbers as it is easier to work with whole numbers for movements
		i32vec2Direction.x = (int)round(i32vec2Direction.x / fDistance);
		i32vec2Direction.y = (int)round(i32vec2Direction.y / fDistance);
	}
	else
	{
		// Since we are not going anywhere, set this to 0.
		i32vec2Direction = glm::i32vec2(0);
	}
}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void CEnemy2D::FlipHorizontalDirection(void)
{
	i32vec2Direction.x *= -1;
}

/**
@brief Update position.
*/
void CEnemy2D::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = i32vec2Index;

	// if the player is to the left or right of the enemy2D, then jump to attack
	if (i32vec2Direction.x < 0)
	{
		// Move left
		const int iOldIndex = i32vec2Index.x;
		if (i32vec2Index.x >= 0)
		{
			i32vec2NumMicroSteps.x--;
			if (i32vec2NumMicroSteps.x < 0)
			{
				i32vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				i32vec2Index.x--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(LEFT, 1, 99) == false)
		{
			FlipHorizontalDirection();
			i32vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		InteractWithPlayer();
	}
	else if (i32vec2Direction.x > 0)
	{
		// Move right
		const int iOldIndex = i32vec2Index.x;
		if (i32vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			i32vec2NumMicroSteps.x++;

			if (i32vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
			{
				i32vec2NumMicroSteps.x = 0;
				i32vec2Index.x++;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(RIGHT, 1, 99) == false)
		{
			FlipHorizontalDirection();
			i32vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;
		}

		// Check if enemy2D is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		// Interact with the Player
		InteractWithPlayer();
	}

	// if the player is above the enemy2D, then jump to attack
	if (i32vec2Direction.y > 0)
	{
		if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::IDLE)
		{
			cPhysics2D.SetStatus(CPhysics2D::STATUS::JUMP);
			cPhysics2D.SetInitialVelocity(glm::vec2(0.0f, 3.5f));
		}
	}
}

void CEnemy2D::UpdateDefaultEnemy()
{
	std::cout << "updating default" << std::endl;
	switch (sCurrentFSM)
	{
	case IDLE:
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "Switching to Patrol State" << endl;
		}
		iFSMCounter++;
		break;
	case PATROL:
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
		}
		else if (cPhysics2D.CalculateDistance(i32vec2Index, cPlayer2D->i32vec2Index) < 10.0f)
		{
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
		}
		else
		{
			// Patrol around
			// Update the Enemy2D's position for patrol
			UpdatePosition();
		}
		iFSMCounter++;
		break;
	case ATTACK:
		if (cPhysics2D.CalculateDistance(i32vec2Index, cPlayer2D->i32vec2Index) < 10.0f)
		{
			// Calculate a path to the player
			cMap2D->PrintSelf();
			/*cout << "StartPos: " << i32vec2Index.x << ", " << i32vec2Index.y << endl;
			cout << "TargetPos: " << cPlayer2D->i32vec2Index.x << ", "
				<< cPlayer2D->i32vec2Index.y << endl;*/
			auto path = cMap2D->PathFind(i32vec2Index,
				cPlayer2D->i32vec2Index,
				heuristic::euclidean,
				10);
			//cout << "=== Printing out the path ===" << endl;

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << "," << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					i32vec2Destination = coord;
					// Calculate the direction between enemy2D and this destination
					i32vec2Direction = i32vec2Destination - i32vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - i32vec2Destination) == i32vec2Direction)
					{
						// Set a destination
						i32vec2Destination = coord;
					}
					else
						break;
				}
			}

			//cout << "i32vec2Destination : " << i32vec2Destination.x
			//	<< ", " << i32vec2Destination.y << endl;
			//cout << "i32vec2Direction : " << i32vec2Direction.x
			//	<< ", " << i32vec2Direction.y << endl;
			//system("pause");

			// Attack
			// Update direction to move towards for attack
			//UpdateDirection();

			// Update the Enemy2D's position for attack
			UpdatePosition();
		}
		else
		{
			if (iFSMCounter > iMaxFSMCounter)
			{
				sCurrentFSM = PATROL;
				iFSMCounter = 0;
				cout << "ATTACK : Reset counter: " << iFSMCounter << endl;
			}
			iFSMCounter++;
		}
		break;
	default:
		break;
	}
}

void CEnemy2D::UpdateBossEnemy()
{
	std::cout << health << " " << maxHealth << std::endl;
	switch (sCurrentFSM)
	{
	case IDLE:
		cout << "idling" << endl;
		if (iFSMCounter > iMaxFSMCounter)
		{
			//check if health is less than max, then change to heal mode
			if (health < maxHealth)
			{
				sCurrentFSM = HEAL;
				iFSMCounter = 0;
				cout << "switching to healing" << endl;
			}
			else
			{
				//if health is full, change to patrol mode
				sCurrentFSM = PATROL;
				iFSMCounter = 0;
				cout << "Switching to Patrol State" << endl;
			}
		}
		iFSMCounter++;
		break;
	case PATROL:
		cout << "patrolling" << endl;
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			cout << "Switching to Idle State" << endl;
		}
		else if (cPhysics2D.CalculateDistance(i32vec2Index, cPlayer2D->i32vec2Index) < 2.0f)
		{
			if (!cPlayer2D->isSurvival)
				break;
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
		}
		else
		{
			// Patrol around
			// Update the Enemy2D's position for patrol
			UpdateDirection();
			UpdatePosition();
			if (i32vec2Direction.x > 0)
			{
				animatedSprites->PlayAnimation("runRight", -1, 1.0f);
			}
			else if (i32vec2Direction.x < 0)
			{
				animatedSprites->PlayAnimation("runLeft", -1, 1.0f);
			}
			else
			{
				animatedSprites->PlayAnimation("idle", -1, 1.0f);
			}
		}
		iFSMCounter++;
		break;
	case ATTACK:
		std::cout << "attacking" << std::endl;

		//check if distance between player is less than 10, if yes, attack player
		if (cPhysics2D.CalculateDistance(i32vec2Index, cPlayer2D->i32vec2Index) < 2.0f)
		{
			//check if health is less than 50% of max, if yes change to defend mode
			if (health < maxHealth * 0.5)
			{
				sCurrentFSM = DEFEND;
				iFSMCounter = 0;
			}
			// Calculate a path to the player
			cMap2D->PrintSelf();
			/*cout << "StartPos: " << i32vec2Index.x << ", " << i32vec2Index.y << endl;
			cout << "TargetPos: " << cPlayer2D->i32vec2Index.x << ", "
				<< cPlayer2D->i32vec2Index.y << endl;*/
			auto path = cMap2D->PathFind(i32vec2Index,
				cPlayer2D->i32vec2Index,
				heuristic::euclidean,
				10);
			//cout << "=== Printing out the path ===" << endl;

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << "," << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					i32vec2Destination = coord;
					// Calculate the direction between enemy2D and this destination
					i32vec2Direction = i32vec2Destination - i32vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - i32vec2Destination) == i32vec2Direction)
					{
						// Set a destination
						i32vec2Destination = coord;
					}
					else
						break;
				}
			}


			if (i32vec2Direction.x > 0)
			{
				animatedSprites->PlayAnimation("attackRight", -1, 1.0f);
			}
			else
			{
				animatedSprites->PlayAnimation("attackLeft", -1, 1.0f);
			}
			//cout << "i32vec2Destination : " << i32vec2Destination.x
			//	<< ", " << i32vec2Destination.y << endl;
			//cout << "i32vec2Direction : " << i32vec2Direction.x
			//	<< ", " << i32vec2Direction.y << endl;
			//system("pause");

			// Attack
			// Update direction to move towards for attack
			UpdateDirection();

			// Update the Enemy2D's position for attack
			UpdatePosition();
		}
		else
		{
			//if distance between player is far enough, change to patrol mode
			if (iFSMCounter > iMaxFSMCounter)
			{
				sCurrentFSM = PATROL;
				iFSMCounter = 0;
				cout << "ATTACK : Reset counter: " << iFSMCounter << endl;
			}
			iFSMCounter++;
		}
		break;
	case DEFEND:
		cout << "defending" << endl;

		//check if distance between player is less than 10, if yes, retreat and run the opposite direction of the player
		if (cPhysics2D.CalculateDistance(i32vec2Index, cPlayer2D->i32vec2Index) < 5.0f)
		{
			auto path = cMap2D->PathFind(i32vec2Index,
				cPlayer2D->i32vec2Index,
				heuristic::euclidean,
				10);
			//cout << "=== Printing out the path ===" << endl;

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << "," << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					i32vec2Destination = coord;
					// Calculate the direction between enemy2D and this destination
					i32vec2Direction = i32vec2Destination + i32vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord + i32vec2Destination) == i32vec2Direction)
					{
						// Set a destination
						i32vec2Destination = coord;
					}
					else
						break;
				}
			}
			UpdatePosition();
		}
		else
		{
			//once the distance between the player is far enough, change to idle mode
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
		}
	case HEAL:
		cout << "healing" << endl;

		//check if distance between player is less than 5, if yes, retreat and change to defend mode
		if (cPhysics2D.CalculateDistance(i32vec2Index, cPlayer2D->i32vec2Index) < 5.0f)
		{
			sCurrentFSM = DEFEND;
			iFSMCounter = 0;
		}
		else 
		{
			//check if health is less than max health, if yes, start to heal
			if (health < maxHealth)
			{
				if (iFSMCounter > iMaxFSMCounter)
				{
					health++;
					iFSMCounter = 0;
				}
				iFSMCounter++;
			}
			else //if health is full, change to idle mode
			{
				i32vec2Direction = glm::i32vec2(0, 0);
				sCurrentFSM = IDLE;
				iFSMCounter = 0;
			}
		}
	default:
		break;
	}
}

void CEnemy2D::AttackEnemy()
{
	if (cPhysics2D.CalculateDistance(i32vec2Index, cPlayer2D->i32vec2Index) < 3.0f)
	{
		if (CKeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E))
		{
			CSoundController::GetInstance()->PlaySoundByName("enemyAttack");
			health--;
			currentColor = glm::vec4(1.f, 0.f, 0.f, 1.f);
		}
	}
}
