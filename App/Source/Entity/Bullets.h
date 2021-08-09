#pragma once

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include Settings
#include "GameControl\Settings.h"

class CBullets : public CEntity2D
{
public:
	// Constructor
	CBullets(void);

	// Destructor
	~CBullets(void);

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
};

