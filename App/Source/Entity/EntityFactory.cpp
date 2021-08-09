/**
 CBullets
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "EntityFactory.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"
// Include ImageLoader
#include "System\ImageLoader.h"

CEntityFactory::CEntityFactory(void)
{
}

CEntityFactory::~CEntityFactory(void)
{
}

CBullets* CEntityFactory::SpawnBullet(glm::f32vec2 _f32vec2Index, glm::f32vec2 _f32vec2Vel, glm::vec3 _vec3Scale, float _rotation, CEntity2D::ENTITY_TYPE _type)
{
    CBullets* newBullet = new CBullets;
    newBullet->Init();
    newBullet->f32vec2Index = _f32vec2Index;
    newBullet->f32vec2Vel = _f32vec2Vel;
    newBullet->vec3Scale = _vec3Scale;
    newBullet->rotation = _rotation;
    newBullet->type = _type;
    newBullet->isActive = true;

    return newBullet;
}