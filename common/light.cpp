#include <common/light.hpp>
#include <common/maths.hpp>

bool active = false;
int upOrDown = 5;
int reached = 0;

void Light::addPointLight(const glm::vec3 position, const glm::vec3 colour,
    const float constant, const float linear,
    const float quadratic)
{
    LightSource light;
    light.position = position;
    light.colour = colour;
    light.constant = constant;
    light.linear = linear;
    light.quadratic = quadratic;
    light.type = 1;
    lightSources.push_back(light);
}

void Light::addSpotLight(const glm::vec3 position, const glm::vec3 direction,
    const glm::vec3 colour, const float constant,
    const float linear, const float quadratic,
    const float cosPhi)
{
    LightSource light;
    light.position = position;
    light.direction = direction;
    light.colour = colour;
    light.constant = constant;
    light.linear = linear;
    light.quadratic = quadratic;
    light.cosPhi = cosPhi;
    light.type = 2;
    lightSources.push_back(light);
}

void Light::addDirectionalLight(const glm::vec3 direction, const glm::vec3 colour)
{
    LightSource light;
    light.direction = direction;
    light.colour = colour;
    light.type = 3;
    lightSources.push_back(light);
}

void Light::toShader(unsigned int shaderID, glm::mat4 view)
{
    unsigned int numLights = static_cast<unsigned int>(lightSources.size());
    glUniform1i(glGetUniformLocation(shaderID, "numLights"), numLights);

    for (unsigned int i = 0; i < numLights; i++)
    {
        std::string idx = std::to_string(i);
        glm::vec3 VSLightPosition = glm::vec3(view * glm::vec4(lightSources[i].position, 1.0f));
        glm::vec3 VSLightDirection = glm::vec3(view * glm::vec4(lightSources[i].direction, 0.0f));
        glUniform3fv(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].position").c_str()), 1, &VSLightPosition[0]);
        glUniform3fv(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].direction").c_str()), 1, &VSLightDirection[0]);
        glUniform3fv(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].colour").c_str()), 1, &lightSources[i].colour[0]);
        glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].constant").c_str()), lightSources[i].constant);
        glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].linear").c_str()), lightSources[i].linear);
        glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].quadratic").c_str()), lightSources[i].quadratic);
        glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].cosPhi").c_str()), lightSources[i].cosPhi);
        glUniform1i(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].type").c_str()), lightSources[i].type);
    }
}

void Light::draw(unsigned int shaderID, glm::mat4 view, glm::mat4 projection, Model lightModel)
{
    glUseProgram(shaderID);
    for (unsigned int i = 0; i < static_cast<unsigned int>(lightSources.size()); i++)
    {
            // Ignore directional lights
            if (lightSources[i].type == 3)
                continue;

            // Calculate model matrix
            glm::mat4 translate = Maths::translate(lightSources[i].position); //CHANGED
            glm::mat4 scale = Maths::scale(glm::vec3(0.1f)); //CHANGED
            glm::mat4 model = translate * scale;

            // Send the MVP and MV matrices to the vertex shader
            glm::mat4 MVP = projection * view * model;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);

            // Send model, view, projection matrices and light colour to light shader
            glUniform3fv(glGetUniformLocation(shaderID, "lightColour"), 1, &lightSources[i].colour[0]);

            // Draw light source
            lightModel.draw(shaderID);
    }
}

void Light::activated() {
    active = true;
    for (unsigned int i = 0; i < static_cast<unsigned int>(lightSources.size()); i++)
    {
        if (lightSources[i].type == 2) {
            lightSources[i].colour = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        else if (lightSources[i].position.y < 3) {
            lightSources[i].position = glm::vec3(lightSources[i].position.x, lightSources[i].position.y + 0.025f, lightSources[i].position.z);
            lightSources[i].linear = 5.8f;
        }
    }
}

void Light::deactivated() {
    active = false;
    for (unsigned int i = 0; i < static_cast<unsigned int>(lightSources.size()); i++)
    {
        if (lightSources[i].type == 2) {
            lightSources[i].colour = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        if (lightSources[i].type == 1) {
            if (lightSources[i].position.y > -10) {
                lightSources[i].position = glm::vec3(lightSources[i].position.x, lightSources[i].position.y - 0.05f, lightSources[i].position.z);
                //lightSources[i].constant = 1.0f;
                lightSources[i].linear = 20.8f;
                //lightSources[i].quadratic = 0.02f;
            }
            //if (lightSources[i].position.x)
        }
    }
}

//void Light::pointLightCirculate(glm::vec3 position) {
//    switch (position) {
//        case (0.0f, 3.0f, 4.0f) {
//
//        }
//    }
//
//}
//
//std::array<int, 6> xPos = { 0.0f, -4.0f, -4.0f, 0.0f, 4.0f, 4.0f };
//std::array<int, 6> zPos = { 4.0f, 2.0f, -2.0f, -4.0f, -2.0f, 2.0f };

//NO GLM LEFT

