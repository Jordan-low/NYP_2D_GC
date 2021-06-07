/**
 CPlayer2D
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include Singleton template
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

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include Mesh
#include "Primitives/Mesh.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

//Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

//Include Phys
#include "Physics2D.h"

//Include Inventory Manager
#include "InventoryManager.h"


class CPlayer2D : public CSingletonTemplate<CPlayer2D>, public CEntity2D
{
	friend CSingletonTemplate<CPlayer2D>;
public:

	enum MOUSE_CLICK
	{
		MOUSE_LEFT = 0,
		MOUSE_RIGHT,
		MOUSE_TOTAL,
	};

	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	// Let player interact with the map
	void Harvest(int x, int y);

	bool CheckQuantity(string itemName);

	void ReduceQuantity(string itemName, int quantity);

	int ConvertItemNameToBlockNumber(string itemName);

	void ResetPosition();

	void UpdateMouse(MOUSE_CLICK mouseClick, double x, double y, string itemName);		// Update Placing/Harvesting Blocks using mouse position

	void UpdateSeeds(double dt);

	glm::vec2 playerOffset;

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	// Store the old indices
	glm::i32vec2 i32vec2OldIndex;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	//CS: The quadMesh for drawing the tiles
	CMesh* quadMesh;

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	//Phys
	CPhysics2D cPhysics2D;

	//Inventory manager
	CInventoryManager* cInventoryManager;

	//Inventoy Item
	CInventoryItem* cInventoryItem;

	//Player Colour
	glm::vec4 playerColour;

	// Constructor
	CPlayer2D(void);

	// Destructor
	virtual ~CPlayer2D(void);

	// Load a texture
	bool LoadTexture(const char* filename, GLuint& iTextureID);

	// Constraint the player's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	// Check if a position is possible to move into
	bool CheckPosition(DIRECTION eDirection);

	//Update jump
	void UpdateJumpFall(const double dElapsedTime = 0.0166666666666667);

	bool IsMidAir();

	bool Collision(DIRECTION eDirection);
	bool CollisionEnd(DIRECTION eDirection);

private:
	bool reachLeftEnd = true;
	bool reachRightEnd = false;
	bool reachTopEnd = false;
	bool reachBotEnd = false;

	bool doubleJumpReady = false;
};

