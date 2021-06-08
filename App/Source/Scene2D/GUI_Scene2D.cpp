/**
 CGUI_Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"

#include <iostream>
using namespace std;
/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::CGUI_Scene2D(void)
	: cSettings(NULL),
	window_flags(0),
	m_fProgressBar(0.0f),
	cInventoryManager(NULL),
	cInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	// We won't delete this since it was created elsewhere
	cSettings = NULL;

	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
	}

	//cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

/**
  @brief Initialise this instance
  */
bool CGUI_Scene2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	cFPSCounter = CFPSCounter::GetInstance();

	//init inven manager
	cInventoryManager = CInventoryManager::GetInstance();
	//add tree to inven item
	//cInventoryItem = cInventoryManager->Add("DirtTree", "Image/Scene2D_TreeTile.tga", 5, 0);
	//cInventoryItem = cInventoryManager->Add("Dirt", "Image/DirtBlock.png", 5, 50);
	//cInventoryItem->vec2Size = glm::vec2(25, 25);
	
	
	//setup imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//setup imgui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	//setup renderer
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance()->pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	//define window flags
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	
	return true;
}

/**
 @brief Update this instance
 */
void CGUI_Scene2D::Update(const double dElapsedTime)
{
	//start imgui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//create an invis window
	ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
	ImGui::SetWindowFontScale(1.5f);

	ImVec4 col = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", cFPSCounter->GetFrameRate());
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "HEALTH: %d", (int)playerHealth);
	
	//m_fProgressBar += 0.001f;
	//if (m_fProgressBar > 1.0f)
	//	m_fProgressBar = 0.0f;

	//col = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	//ImGui::PushStyleColor(ImGuiCol_PlotHistogram, col);
	//	col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	//	ImGui::PushStyleColor(ImGuiCol_FrameBg, col);
	//		ImGui::ProgressBar(m_fProgressBar, ImVec2(100.0f, 20.0f));
	//	ImGui::PopStyleColor();
	//ImGui::PopStyleColor();
	//ImGui::End();

	//cInventoryItem = cInventoryManager->GetItem("Tree");
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
	//ImGuiWindowFlags inventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//										ImGuiWindowFlags_NoTitleBar |
	//										ImGuiWindowFlags_NoMove |
	//										ImGuiWindowFlags_NoResize |
	//										ImGuiWindowFlags_NoCollapse |
	//										ImGuiWindowFlags_NoScrollbar;
	//
	//ImGui::Begin("Image", NULL, inventoryWindowFlags);
	//	ImGui::SetWindowPos(ImVec2(25.0f, 550.0f));
	//	ImGui::SetWindowSize(ImVec2(200.0f, 25.0f));
	//	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y), 
	//		ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::SameLine();
	//	ImGui::SetWindowFontScale(1.5f);
	//	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Tree: %d / %d",
	//		cInventoryItem->GetCount(), cInventoryItem->GetMaxCount());
	//ImGui::PopStyleColor();
	//ImGui::End();

	/*cInventoryItem = cInventoryManager->GetItem("Dirt");
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGuiWindowFlags inventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin("Image", NULL, inventoryWindowFlags);
	ImGui::SetWindowPos(ImVec2(600.0f, 650.0f));
	ImGui::SetWindowSize(ImVec2(25.0f, 25.0f));
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::PopStyleColor();	
	ImGui::SetWindowFontScale(1.5f);
	ImGui::TextColored(ImVec4(0, 0, 0, 1), "%d", cInventoryItem->GetCount());
	ImGui::End();*/

	//// Render the Health
	//ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("DirtTree", NULL, healthWindowFlags);
	//ImGui::SetWindowPos(ImVec2(25.0f, 25.0f));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->GetItem("DirtSeed");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
	//ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	//ImGui::ProgressBar(cInventoryItem->GetCount() /
	//	(float)cInventoryItem->GetMaxCount(), ImVec2(100.0f, 20.0f));
	//ImGui::PopStyleColor();
	//ImGui::PopStyleColor();
	//ImGui::End();


	for (int i = 0; i < sizeof(cInventoryManager->inventoryArray) / sizeof(*cInventoryManager->inventoryArray); i++)
	{
		if (cInventoryManager->inventoryArray[i] == nullptr)
			break;

		//Render the inventory
		ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;

		string begin = "InventoryItem" + to_string(i);
		ImGui::Begin(begin.c_str(), NULL, InventoryWindowFlags);
		ImGui::SetWindowPos(ImVec2(1210, 240 + i * 30));
		ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
		cInventoryItem = cInventoryManager->inventoryArray[i];
		ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
			ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.5f);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
			cInventoryItem->GetCount());
		ImGui::End();
	}

	if (cInventoryManager->currentItem != nullptr)
	{
		float currentItemPosY = 240;
		for (int i = 0; i < sizeof(cInventoryManager->inventoryArray) / sizeof(*cInventoryManager->inventoryArray); i++)
		{
			if (cInventoryManager->currentItem == cInventoryManager->inventoryArray[i])
				break;
			currentItemPosY += 30;
		}

		//Render the current item box
		ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar;

		ImGui::Begin("CurrentItem", NULL, InventoryWindowFlags);
		ImGui::SetWindowPos(ImVec2(1210, currentItemPosY));
		ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
		cInventoryItem = cInventoryManager->GetItem("Selector");
		ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
			ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}


	//if (cInventoryManager->currentItem != nullptr)
	//{
	//	// Render the dirt seeds
	//	ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//		ImGuiWindowFlags_NoBackground |
	//		ImGuiWindowFlags_NoTitleBar |
	//		ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_NoResize |
	//		ImGuiWindowFlags_NoCollapse |
	//		ImGuiWindowFlags_NoScrollbar;
	//	ImGui::Begin("InventoryItem", NULL, InventoryWindowFlags);
	//	ImGui::SetWindowPos(ImVec2(1210, 240));
	//	ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//	cInventoryItem = cInventoryManager->currentItem;
	//	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//		ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::SameLine();
	//	ImGui::SetWindowFontScale(1.5f);
	//	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//		cInventoryItem->GetCount());
	//	ImGui::End();
	//}

	//if (cInventoryManager->currentItem != nullptr)
	//{
	//	// Render the dirt seeds
	//	ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//		ImGuiWindowFlags_NoBackground |
	//		ImGuiWindowFlags_NoTitleBar |
	//		ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_NoResize |
	//		ImGuiWindowFlags_NoCollapse |
	//		ImGuiWindowFlags_NoScrollbar;
	//	ImGui::Begin("InventoryItem", NULL, InventoryWindowFlags);
	//	ImGui::SetWindowPos(ImVec2(1210, 270));
	//	ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//	cInventoryItem = cInventoryManager->currentItem;
	//	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//		ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::SameLine();
	//	ImGui::SetWindowFontScale(1.5f);
	//	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//		cInventoryItem->GetCount());
	//	ImGui::End();
	//}


	//// Render the dirt seeds
	//ImGuiWindowFlags SelectorWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("SelectorItem", NULL, SelectorWindowFlags);
	//ImGui::SetWindowPos(ImVec2(1210, 270));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->GetItem("Selector");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::SetWindowFontScale(1.5f);
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//	cInventoryItem->GetCount());
	//ImGui::End();


	//if (cInventoryManager->inventoryArray[0] != nullptr)
	//{
	//	// Render the dirt seeds
	//	ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//		ImGuiWindowFlags_NoBackground |
	//		ImGuiWindowFlags_NoTitleBar |
	//		ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_NoResize |
	//		ImGuiWindowFlags_NoCollapse |
	//		ImGuiWindowFlags_NoScrollbar;
	//	ImGui::Begin("InventoryItem", NULL, InventoryWindowFlags);
	//	ImGui::SetWindowPos(ImVec2(1210, 300));
	//	ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//	cInventoryItem = cInventoryManager->inventoryArray[0];
	//	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//		ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::SameLine();
	//	ImGui::SetWindowFontScale(1.5f);
	//	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//		cInventoryItem->GetCount());
	//	ImGui::End();
	//}
	//
	//if (cInventoryManager->inventoryArray[1] != nullptr)
	//{
	//	// Render the dirt seeds
	//	ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//		ImGuiWindowFlags_NoBackground |
	//		ImGuiWindowFlags_NoTitleBar |
	//		ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_NoResize |
	//		ImGuiWindowFlags_NoCollapse |
	//		ImGuiWindowFlags_NoScrollbar;
	//	ImGui::Begin("InventoryItem", NULL, InventoryWindowFlags);
	//	ImGui::SetWindowPos(ImVec2(1210, 330));
	//	ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//	cInventoryItem = cInventoryManager->inventoryArray[1];
	//	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//		ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::SameLine();
	//	ImGui::SetWindowFontScale(1.5f);
	//	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//		cInventoryItem->GetCount());
	//	ImGui::End();
	//}

	//if (cInventoryManager->inventoryArray[2] != nullptr)
	//{
	//	// Render the dirt seeds
	//	ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//		ImGuiWindowFlags_NoBackground |
	//		ImGuiWindowFlags_NoTitleBar |
	//		ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_NoResize |
	//		ImGuiWindowFlags_NoCollapse |
	//		ImGuiWindowFlags_NoScrollbar;
	//	ImGui::Begin("InventoryItem", NULL, InventoryWindowFlags);
	//	ImGui::SetWindowPos(ImVec2(1210, 360));
	//	ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//	cInventoryItem = cInventoryManager->inventoryArray[2];
	//	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//		ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::SameLine();
	//	ImGui::SetWindowFontScale(1.5f);
	//	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//		cInventoryItem->GetCount());
	//	ImGui::End();
	//}

	//if (cInventoryManager->inventoryArray[3] != nullptr)
	//{
	//	// Render the dirt seeds
	//	ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//		ImGuiWindowFlags_NoBackground |
	//		ImGuiWindowFlags_NoTitleBar |
	//		ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_NoResize |
	//		ImGuiWindowFlags_NoCollapse |
	//		ImGuiWindowFlags_NoScrollbar;
	//	ImGui::Begin("InventoryItem", NULL, InventoryWindowFlags);
	//	ImGui::SetWindowPos(ImVec2(1210, 390));
	//	ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//	cInventoryItem = cInventoryManager->inventoryArray[3];
	//	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//		ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//		ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::SameLine();
	//	ImGui::SetWindowFontScale(1.5f);
	//	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//		cInventoryItem->GetCount());
	//	ImGui::End();
	//}

	//if (cInventoryManager->prevCurrentItem == nullptr)
	//{
	//	ImGui::End();
	//	return;
	//}

	//// Render the dirt seeds
	//ImGuiWindowFlags prevInventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("PrevInventoryItem", NULL, prevInventoryWindowFlags);
	//ImGui::SetWindowPos(ImVec2(1210, 300));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->prevCurrentItem;
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::SetWindowFontScale(1.5f);
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//	cInventoryItem->GetCount());
	//ImGui::End();

	//// Render the dirt
	//ImGuiWindowFlags grassBlockWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("GrassSeed", NULL, grassBlockWindowFlags);
	//ImGui::SetWindowPos(ImVec2(1210, 300));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->GetItem("GrassSeed");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::SetWindowFontScale(1.5f);
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//	cInventoryItem->GetCount());
	//ImGui::End();

	//// Render the dirt
	//ImGuiWindowFlags grassSeedWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("GrassBlock", NULL, grassSeedWindowFlags);
	//ImGui::SetWindowPos(ImVec2(1210, 330));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->GetItem("GrassBlock");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::SetWindowFontScale(1.5f);
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//	cInventoryItem->GetCount());
	//ImGui::End();

	//// Render the dirt block
	//ImGuiWindowFlags dirtBlockWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("DirtBlock", NULL, dirtBlockWindowFlags);
	//ImGui::SetWindowPos(ImVec2(1210, 360));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->GetItem("DirtBlock");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::SetWindowFontScale(1.5f);
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//	cInventoryItem->GetCount());
	//ImGui::End();

	//// Render the dirt block
	//ImGuiWindowFlags StoneWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("Stone", NULL, StoneWindowFlags);
	//ImGui::SetWindowPos(ImVec2(1210, 390));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->GetItem("Stone");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::SameLine();
	//ImGui::SetWindowFontScale(1.5f);
	//ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d",
	//	cInventoryItem->GetCount());
	//ImGui::End();
	//

	//float selectorPos = 270;
	//for (int i = 0; i < cInventoryManager->slotNumber; i++)
	//{
	//	selectorPos += 30;
	//}

	//// Render the inventory selector
	//ImGuiWindowFlags InventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("Inventory", NULL, InventoryWindowFlags);
	//ImGui::SetWindowPos(ImVec2(1210, selectorPos));
	//ImGui::SetWindowSize(ImVec2(100.0f, 25.0f));
	//cInventoryItem = cInventoryManager->GetItem("Selector");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x, cInventoryItem->vec2Size.y),
	//	ImVec2(0, 1), ImVec2(1, 0));
	//ImGui::End();

	ImGui::End();
}	

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_Scene2D::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_Scene2D::Render()
{
	//Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_Scene2D::PostRender(void)
{
}
