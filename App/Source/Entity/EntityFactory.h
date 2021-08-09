#pragma once

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"
#include "Bullets.h"

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

class CEntityFactory : public CSingletonTemplate<CEntityFactory>
{
	friend CSingletonTemplate<CEntityFactory>;
public:
	// Constructor
	CEntityFactory(void);

	// Destructor
	~CEntityFactory(void);

	CBullets* SpawnBullet(glm::f32vec2 f32vec2Index, glm::f32vec2 f32vec2Vel, glm::vec3 vec3Scale, float rotation, CEntity2D::ENTITY_TYPE type);
};

