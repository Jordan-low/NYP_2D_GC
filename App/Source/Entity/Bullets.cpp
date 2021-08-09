/**
 CBullets
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Bullets.h"
#include "../Scene2D/Player2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"
// Include ImageLoader
#include "System\ImageLoader.h"

CBullets::CBullets(void)
{
}

CBullets::~CBullets(void)
{
}

bool CBullets::Init() {
    cSettings = CSettings::GetInstance();
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    mesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
    if (LoadTexture("Image/Characters/Enemy2.png")) {
        return true;
    }
}

void CBullets::Update(const double dElapsedTime)
{
    //edit the pos based on vel
    f32vec2Index += f32vec2Vel;
    
    vec2UVCoordinate.x = cSettings->ConvertEntityIndexToUVSpace(cSettings->x, f32vec2Index.x, false);
    vec2UVCoordinate.y = cSettings->ConvertEntityIndexToUVSpace(cSettings->y, f32vec2Index.y, false);
}

void CBullets::PreRender(void)
{
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    CShaderManager::GetInstance()->Use("2DShader");
}

void CBullets::Render(void)
{
    glBindVertexArray(VAO);
    unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
    unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtime_color");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
        vec2UVCoordinate.y,
        0.0f));
    transform = glm::rotate(transform, rotation, glm::vec3(0, 0, 1));
    transform = glm::scale(transform, vec3Scale);

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

    glBindTexture(GL_TEXTURE_2D, iTextureID);

    mesh->Render();

    glBindVertexArray(0);
}

void CBullets::PostRender(void)
{
    glDisable(GL_BLEND);
}