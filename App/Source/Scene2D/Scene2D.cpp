#include "Scene2D.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"

bool is_file_exist(string fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScene2D::CScene2D(void)
	: cKeyboardController(NULL)
	, cMouseController(NULL)
	, cGUI(NULL)
	, cGUI2(NULL)
	, cMap2D(NULL)
	, cPlayer2D(NULL)
	, cSoundController(NULL)
{
}

/**
 @brief Destructor
 */
CScene2D::~CScene2D(void)
{
	if (cGUI)
	{
		cGUI->Destroy();
		cGUI = NULL;
	}

	if (cGUI2)
	{
		cGUI2->Destroy();
		cGUI2 = NULL;
	}
	
	if (cMap2D)
	{
		cMap2D->Destroy();
		cMap2D = NULL;
	}

	if (cPlayer2D)
	{	
		cPlayer2D->Destroy();
		cPlayer2D = NULL;
	}

	for (CEntity2D* enemy : enemyVector)
	{
		delete enemy;
		enemy = NULL;
	}

	if (cSoundController)
	{
		cSoundController->Destroy();
		cSoundController = NULL;
	}

	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;
	cMouseController = NULL;
}

/**
@brief Init Initialise this instance
*/ 
bool CScene2D::Init(void)
{
	// Include Shader Manager
	//CShaderManager::GetInstance()->Add("2DShader", "Shader//Scene2D.vs", "Shader//Scene2D.fs");
	CShaderManager::GetInstance()->Use("2DShader");
	CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	// Create and initialise the Map 2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("2DShader");


	// Load Scene2DColor into ShaderManager
	CShaderManager::GetInstance()->Use("2DColorShader");
	CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);
	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();
	// Pass shader to cPlayer2D
	cPlayer2D->SetShader("2DColorShader");


	// Setup the shaders
	CShaderManager::GetInstance()->Use("textShader");

	// Store the keyboard and mouse controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	cMouseController = CMouseController::GetInstance();
	cInventoryManager = CInventoryManager::GetInstance();

	// Setup cGUI
	cGUI = CGUI_Scene2D::GetInstance();
	cGUI->Init();

	cGUI2 = CGUI::GetInstance();
	cGUI2->Init();

	cSoundController = CSoundController::GetInstance();
	cSoundController->Init();
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\jump.wav"), "jump", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\click.wav"), "uiClick", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hit.wav"), "playerAttack", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\hurt2.wav"), "enemyAttack", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\placeBlock.wav"), "place", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\placeBlockFail.wav"), "placeFail", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\breakBlock.wav"), "break", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\pickUp.wav"), "pickUp", true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\openChest.wav"), "openChest", true);

	//init map instance
	if (cMap2D->Init(1, 24, 100) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}


	if (cMap2D->LoadMap("Maps/START.csv", 0) == false)
	{
		cout << "loading of default map has failed." << endl;
		return false;
	}

	//init player instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load cPlayer2D" << endl;
		return false;
	}

	//Create and Init CEnemy2D
	enemyVector.clear();
	while (true)
	{
		CEnemy2D* cEnemy2D = new CEnemy2D();
		//Pass shader to enemy
		cEnemy2D->SetShader("2DColorShader");
		//Init instance
		if (cEnemy2D->Init())
		{
			cEnemy2D->SetPlayer2D(cPlayer2D);
			enemyVector.push_back(cEnemy2D);
		}
		else
			break;
	}

	return true;
}

/**
@brief Update Update this instance
*/
void CScene2D::Update(const double dElapsedTime)
{
	//update gui
	cGUI->Update(dElapsedTime);
	cGUI->playerHealth = cPlayer2D->health;
	cGUI->playerMaxHealth = cPlayer2D->maxHealth;
	cGUI2->Update(dElapsedTime);

	//update enemy before map
	for (CEntity2D* enemy : enemyVector)
	{
		enemy->Update(dElapsedTime);
	}

	//update player
	if (!enableTyping)
		cPlayer2D->Update(dElapsedTime);

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_ENTER))
	{
		if (cGUI2->worldInput.length() == 0)
		{
			enableTyping = !enableTyping;
			cSoundController->PlaySoundByName("uiClick");
		}
	}

	if (enableTyping)
	{
		//listens for key pressed and adds to worldInput string to show on display
		if (cKeyboardController->KeyListener() != ' ')
		{
			cGUI2->worldInput += cKeyboardController->KeyListener();
		}

		//backspace to delete last key
		if (cKeyboardController->IsKeyReleased(GLFW_KEY_BACKSPACE) && cGUI2->worldInput.length() > 0)
		{
			cGUI2->worldInput.erase(cGUI2->worldInput.length() - 1);
		}

		//enter to submit
		else if (cKeyboardController->IsKeyReleased(GLFW_KEY_ENTER) && cGUI2->worldInput.length() > 0)
		{
			enableTyping = false;
			try 
			{
				bool worldExists = false;
				fstream inFile;
				inFile.open("Maps/WorldsList.txt");
				string line;
				string worldName;

				if (inFile.fail()) //if file fails to load
				{
					cout << "Error loading WorldsList file" << endl;
				}

				while (getline(inFile, line)) //get each line of the file and add the existing world names to vector
				{
					stringstream ss(line);
					getline(ss, worldName);
					cMap2D->worldNameList.push_back(worldName);
				}

				for (int i = 0; i < cMap2D->worldNameList.size(); i++) //cycle through vector to see if user inputted world name has already existed before
				{
					if (cMap2D->worldNameList.at(i) == cGUI2->worldInput) //if world already exist, turn worldExist bool to true
					{
						worldExists = true;
					}
				}

				if (worldExists) //if world already exist, load that world
				{
					cMap2D->activeWorld = cGUI2->worldInput; //set active world to current loaded world
					if (!cMap2D->LoadMap("Maps/" + cGUI2->worldInput + ".csv")) //check if world is able to load
					{
						std::cout << "Unable to load existing map: " << cGUI2->worldInput << std::endl;
						return;
					}
				}
				else
				{
					throw runtime_error("world name " + cGUI2->worldInput + " does not exists, creating a new world for it.");
				}
			}
			catch (runtime_error e) //else generate a new world with user inputted world name
			{
				//update saved map
				if (cMap2D->GenerateNewMap(cGUI2->worldInput))
				{
					ofstream outFile;
					outFile.open("Maps/WorldsList.txt", ios::app);
					outFile << cGUI2->worldInput << endl;
					outFile.close();
					cMap2D->activeWorld = cGUI2->worldInput;
					cMap2D->SetTotalWorldsGenerated(cMap2D->GetTotalWorldsGenerated() + 1);
				}
				else
				{
					cout << "failed to create a new map for " + cGUI2->worldInput << endl;
				}
				//cMap2D->SaveMap("Maps/" + cGUI->worldInput + ".csv");
			}

			cGUI2->worldInput.clear();
			cPlayer2D->ResetPosition();
			cMouseController->mouseOffset = glm::vec2(0.0f);
		}
	}

	////procedural generation
	//proceduralGenerationCooldown -= dElapsedTime;

	//if (cPlayer2D->i32vec2Index.x + 35 >= CSettings::GetInstance()->NUM_TILES_XAXIS)
	//{
	//	if (proceduralGenerationCooldown < 0)
	//	{
	//		std::cout << "COMMENCE GENERATION" << std::endl;
	//		cMap2D->ProceduralGeneration();
	//		proceduralGenerationCooldown = 2;
	//	}
	//}

	//update CMap2D
	//cMap2D->Update(dElapsedTime);
	
	//Update seeds timer
	cPlayer2D->UpdateSeeds(dElapsedTime);

	int multiplier = 0;
	if (cPlayer2D->isCenter)
	{
		multiplier = 1;
	}
	cMouseController->mouseOffset.x = multiplier * (cPlayer2D->i32vec2Index.x - 15.5);
	cMouseController->mouseOffset.y = 0;

	//get mouse updates
	if (cMouseController->IsButtonPressed(GLFW_MOUSE_BUTTON_1))
	{
		if (cInventoryManager->currentItem != cInventoryManager->currentWeapon)
			cPlayer2D->UpdateMouse(cPlayer2D->MOUSE_LEFT, cMouseController->GetMousePositionXOnScreen(), cMouseController->GetMousePositionYOnScreen(), cInventoryManager->currentItem->sName);
	}

	if (cMouseController->IsButtonPressed(GLFW_MOUSE_BUTTON_2))
	{
		if (cInventoryManager->currentItem != cInventoryManager->currentWeapon)
			cPlayer2D->UpdateMouse(cPlayer2D->MOUSE_RIGHT, cMouseController->GetMousePositionXOnScreen(), cMouseController->GetMousePositionYOnScreen(), cInventoryManager->currentItem->sName);
	}
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CScene2D::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);

	//call cGUI preRender
	cGUI->PreRender();
	//call cGUI render
	cGUI->Render();
	//call cGUI postRender
	cGUI->PostRender();

	//call cGUI preRender
	cGUI2->PreRender();
	//call cGUI render
	cGUI2->Render(cMap2D->activeWorld);
	//call cGUI postRender
	cGUI2->PostRender();
}

/**
 @brief Render Render this instance
 */
void CScene2D::Render(void)
{
	//call map2d preRender
	cMap2D->PreRender();
	//call map2d render
	cMap2D->Render();
	//call map2d postRender
	cMap2D->PostRender();

	// Call the cGUI's PreRender()
	cGUI->PreRender();
	// Call the cGUI's Render()
	cGUI->Render();
	// Call the cGUI's PostRender()
	cGUI->PostRender();

	// Call the cGUI's PreRender()
	cGUI2->PreRender();
	// Call the cGUI's Render()
	cGUI2->Render(cMap2D->activeWorld);
	// Call the cGUI's PostRender()
	cGUI2->PostRender();

	for (CEntity2D* enemy : enemyVector)
	{
		// Call the enemy PreRender()
		enemy->PreRender();
		// Call the enemy Render()
		enemy->Render();
		// Call the enemy PostRender()
		enemy->PostRender();
	}

	// Call the cPlayer2D's PreRender()
	cPlayer2D->PreRender();
	// Call the cPlayer2D's Render()
	cPlayer2D->Render();
	// Call the cPlayer2D's PostRender()
	cPlayer2D->PostRender();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScene2D::PostRender(void)
{
}