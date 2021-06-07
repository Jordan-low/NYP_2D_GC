#include "InventoryManager.h"
#include <stdexcept>      // std::invalid_argument

/**
@brief Constructor
*/
CInventoryManager::CInventoryManager(void) 
{
	currentItem = nullptr;
	prevCurrentItem = nullptr;
	for (int i = 0; i < sizeof(inventoryArray) / sizeof(*inventoryArray); i++)
	{
		inventoryArray[i] = nullptr;
	}
}

/**
@brief Destructor
*/
CInventoryManager::~CInventoryManager(void)
{
	// Clear the memory
	Exit();
}


void CInventoryManager::SwitchCurrentItem()
{
	CInventoryItem* oldCurrent = currentItem;
	bool cycled = false;
	for (const auto& p : inventoryMap)
	{
		if (currentItem == nullptr)
		{
			currentItem = GetItem(p.first);
			break;
		}
		if (p.first == "Selector")
			continue;
		if (currentItem == GetItem((--inventoryMap.end())->first))
		{
			currentItem = GetItem((inventoryMap.begin())->first);
			cycled = true;
			break;
		}
		if (cycled)
		{
			currentItem = GetItem(p.first);
			break;
		}
		else if (currentItem->sName == p.first)
		{
			cycled = true;
			continue;
		}
	}
	prevCurrentItem = oldCurrent;
	/*CInventoryItem* oldCurrentItem = currentItem;
	bool cycled = false;
	for (const auto& p : inventoryMap)
	{
		if (currentItem == nullptr)
		{
			currentItem = GetItem(p.first);
			break;
		}
		if (p.first == "Selector")
			continue;
		if (currentItem == GetItem((--inventoryMap.end())->first))
		{
			currentItem = GetItem((inventoryMap.begin())->first);
			cycled = true;
			break;
		}
		if (cycled)
		{
			currentItem = GetItem(p.first);
			break;
		}
		else if (currentItem->sName == p.first)
		{
			cycled = true;
			continue;
		}
	}
	if (cycled)
	{
		for (int i = sizeof(inventoryArray) / sizeof(*inventoryArray) - 1; i > 0; i--)
		{
			inventoryArray[i] = inventoryArray[i - 1];
		}
		inventoryArray[0] = oldCurrentItem;
	}*/



	//CycleThroughInventory();
}

void CInventoryManager::CycleThroughInventory()
{
	
}

/**
@brief Update the active Scene
*/
/*
void CInventoryManager::Update(const double dElapsedTime)
{
	// Check for change of scene
	if (nextScene != inventoryMap)
	{
		if (inventoryMap)
		{
			// Scene is valid, need to call appropriate function to exit
			inventoryMap->Release();
		}
		
		inventoryMap = nextScene;
		inventoryMap->Init();
	}

	if (inventoryMap)
		inventoryMap->Update(dElapsedTime);
}
*/

/**
@brief PreRender the active Scene
*/
/*
void CInventoryManager::PreRender(void)
{
	if (inventoryMap)
		inventoryMap->PreRender();
}
*/

/**
@brief Render the active Scene
*/
/*
void CInventoryManager::Render(void)
{
	if (inventoryMap)
		inventoryMap->Render();
}
*/

/**
@brief PostRender the active Scene
*/
/*
void CInventoryManager::PostRender(void)
{
	if (inventoryMap)
		inventoryMap->PostRender();
}
*/

/**
@brief Exit by deleting the items
*/
void CInventoryManager::Exit(void)
{
	// Delete all scenes stored and empty the entire map
	//inventoryMap->Release();
	//inventoryMap = nullptr;
	std::map<std::string, CInventoryItem*>::iterator it, end;
	end = inventoryMap.end();
	for (it = inventoryMap.begin(); it != end; ++it)
	{
		delete it->second;
		it->second = nullptr;
	}
	inventoryMap.clear();
}

/**
@brief 
a Scene to this Inventory Manager
*/
CInventoryItem* CInventoryManager::Add(	const std::string& _name,
								const char* imagePath,
								const int iItemMaxCount,
								const int iItemCount)
{
	if (Check(_name))
	{
		// Item name already exist here, unable to proceed
		throw std::exception("Duplicate item name provided");
		return NULL;
	}

	CInventoryItem* cNewItem = new CInventoryItem(imagePath);
	cNewItem->iItemMaxCount = iItemMaxCount;
	cNewItem->iItemCount = iItemCount;
	cNewItem->sName = _name;

	// Nothing wrong, add the scene to our map
	inventoryMap[_name] = cNewItem;

	return cNewItem;
}

/**
@brief Remove an item from this Inventory Manager
*/
bool CInventoryManager::Remove(const std::string& _name)
{
	// Does nothing if it does not exist
	if (Check(_name))
	{
		// Item is not available, unable to proceed
		throw std::exception("Unknown item name provided");
		return false;
	}

	CInventoryItem* target = inventoryMap[_name];

	// Delete and remove from our map
	delete target;
	inventoryMap.erase(_name);

	return true;
}

/**
@brief Check if a item exists in this Inventory Manager
*/
bool CInventoryManager::Check(const std::string& _name)
{
	return inventoryMap.count(_name) != 0;
}

/**
@brief Get an item by its name
*/ 
CInventoryItem* CInventoryManager::GetItem(const std::string& _name)
{
	// Does nothing if it does not exist
	if (!Check(_name))
		return NULL;

	// Find and return the item
	return inventoryMap[_name];
}

/**
@brief Get the number of items
*/
int CInventoryManager::GetNumItems(void) const
{
	return inventoryMap.size();
}