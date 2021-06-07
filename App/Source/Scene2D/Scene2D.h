/**
 CScene2D
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

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include Keyboard controller
#include "Inputs\KeyboardController.h"
#include "Inputs\MouseController.h"

// GUI
#include "GUI/GUI.h"
#include "GUI_Scene2D.h"

//CMap2D
#include "Map2D.h"


//Player
#include "Player2D.h"

#include <sstream>

class CScene2D : public CSingletonTemplate<CScene2D>
{
	friend CSingletonTemplate<CScene2D>;
public:
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
protected:
	// Constructor
	CScene2D(void);
	// Destructor
	virtual ~CScene2D(void);

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	// Mouse Controller singleton instance
	CMouseController* cMouseController;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// GUI
	CGUI* cGUI2;
	CGUI_Scene2D* cGUI;

	//CMap2D
	CMap2D* cMap2D;

	//Player
	CPlayer2D* cPlayer2D;

private:
	bool enableTyping = false;

	int totalCounter = 1;

	string itemName = "";
};

