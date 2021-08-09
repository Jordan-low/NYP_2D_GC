#include <iostream>
using namespace std;
#include "EntityManager.h"

CEntityManager::CEntityManager(void)
{
}

CEntityManager::~CEntityManager(void)
{
}

bool CEntityManager::Init(void)
{
    return false;
}

void CEntityManager::Update(const double dElapsedTime)
{
    if (entityList.size() <= 0)
        return;

    for (std::vector<CEntity2D*>::iterator it = entityList.begin(); it != entityList.end(); it++)
    {
        CEntity2D* entity = (CEntity2D*)*it;
        if (entity->isActive)
            entity->Update(dElapsedTime);
    }
}

void CEntityManager::Render(void)
{
    if (entityList.size() <= 0)
        return;
    
    for (CEntity2D* entity : entityList)
    {
        if (entity->isActive)
        {
            entity->PreRender();
            entity->Render();
            entity->PostRender();
        }
    }
}