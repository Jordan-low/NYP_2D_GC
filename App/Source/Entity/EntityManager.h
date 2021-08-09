/**
 CEnemy2D
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include Settings
#include "Primitives/Entity2D.h"
#include "GameControl\Settings.h"
#include <vector>

class CEntityManager : public CSingletonTemplate<CEntityManager>
{
	friend CSingletonTemplate<CEntityManager>;
public:
	// Constructor
	CEntityManager(void);

	// Destructor
	~CEntityManager(void);

	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// Render
	void Render(void);

	std::vector<CEntity2D*> entityList;
};

