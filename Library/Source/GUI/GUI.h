/**
 CGUI
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

 // Include SingletonTemplate
#include "../DesignPatterns/SingletonTemplate.h"

 // Include CEntity2D
#include "../Primitives/Entity2D.h"

// Include TextRenderer
#include "../RenderControl/TextRenderer.h"

// FPS Counter
#include "../TimeControl\FPSCounter.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

#include <string>
using namespace std;

class CGUI : public CSingletonTemplate<CGUI>, public CEntity2D
{
	friend CSingletonTemplate<CGUI>;
public:
	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(string activeWorld);

	// PostRender
	void PostRender(void);

	string worldInput;
protected:
	// Constructor
	CGUI();

	// Destructor
	virtual ~CGUI(void);

	// The handler containing the instance of the CTextRenderer
	CTextRenderer* cTextRenderer;

	// FPS Control
	CFPSCounter* cFPSCounter;
};
