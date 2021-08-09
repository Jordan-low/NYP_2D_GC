// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

#include "PlayGameState.h"

// Include CGameStateManager
#include "GameStateManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CPlayGameState::CPlayGameState(void)
	: cScene2D(NULL)
{

}

/**
 @brief Destructor
 */
CPlayGameState::~CPlayGameState(void)
{
	if (cScene2D)
	{
		cScene2D->Destroy();
		cScene2D = NULL;
	}
}

/**
 @brief Init this class instance
 */
bool CPlayGameState::Init(void)
{
	cout << "CPlayGameState::Init()\n" << endl;

	cScene2D = CScene2D::GetInstance();
	// Initialise the cScene2D instance
	if (!cScene2D->Init())
	{
		cout << "failed to load scene2d" << endl;
		return false;
	}

	// Load the images for buttons
	CImageLoader* il = CImageLoader::GetInstance();
	resumeButtonData.fileName = "Image\\GUI\\ResumeButton.png";
	resumeButtonData.textureID = il->LoadTextureGetID(resumeButtonData.fileName.c_str(), false);
	menuButtonData.fileName = "Image\\GUI\\MenuButton.png";
	menuButtonData.textureID = il->LoadTextureGetID(menuButtonData.fileName.c_str(), false);

	return true;
}

/**
 @brief Update this class instance
 */
bool CPlayGameState::Update(const double dElapsedTime)
{
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading PauseState" << endl;
		paused = true;
	}

	// Call the cScene2D's Update method
	if (!paused)
		cScene2D->Update(dElapsedTime);
	else
	{

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;

		float buttonWidth = 256;
		float buttonHeight = 128;

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Main Menu", NULL, window_flags);
			ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth / 2.0,
				CSettings::GetInstance()->iWindowHeight / 3.0));				// Set the top-left of the window at (10,10)
			ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));

			//Added rounding for nicer effect
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 200.0f;

			// Add codes for Start button here
			if (ImGui::ImageButton((ImTextureID)resumeButtonData.textureID,
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
			{
				// Reset the CKeyboardController
				CKeyboardController::GetInstance()->Reset();

				// Load the menu state
				cout << "Loading PlayGameState" << endl;
				CSoundController::GetInstance()->PlaySoundByName("uiClick");
				paused = false;
			}
			// Add codes for Exit button here
			if (ImGui::ImageButton((ImTextureID)menuButtonData.textureID,
				ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
			{
				// Reset the CKeyboardController
				CKeyboardController::GetInstance()->Reset();

				// Load the menu state
				cout << "Loading MenuState" << endl;
				CSoundController::GetInstance()->PlaySoundByName("uiClick");
				CGameStateManager::GetInstance()->SetActiveGameState("MenuState");
			}
			ImGui::End();
		}
	}

	return true;
}

/**
 @brief Render this class instance
 */
void CPlayGameState::Render(void)
{
	//cout << "CPlayGameState::Render()\n" << endl;

	// Call the cScene2D's Pre-Render method
	cScene2D->PreRender();

	// Call the cScene2D's Render method
	cScene2D->Render();

	// Call the cScene2D's PostRender method
	cScene2D->PostRender();

	if (!paused)
	{
		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

/**
 @brief Destroy this class instance
 */
void CPlayGameState::Destroy(void)
{
	cout << "CPlayGameState::Destroy()\n" << endl;

	// Destroy the cScene2D instance
	if (cScene2D)
	{
		cScene2D->Destroy();
		cScene2D = NULL;
	}

}