#include <iostream>
#include <cmath>
#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

//Function prototypes
void keyboardInput(GLFWwindow* window);
void mouseInput(GLFWwindow* window);

//Frame timer floats
float previousTime = 0.0f;  // time of previous iteration of the loop
float deltaTime = 0.0f;  // time elapsed since the previous frame

// Create camera object
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));

//Object struct
struct Object
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    float angle = 0.0f;
    std::string name;
};

//Position vector
glm::vec3 positionVector;

//Bools
bool centralised = false;
bool hasJumped = false;
bool loggedYPos;
bool startJumpHeight;

//Ints
int currentNum = 0;
int targetNum = 10;
int useThirdPerson = 0;
int upPressed = 0;
int downPressed = 0;
int leftPressed = 0;
int rightPressed = 0;
int lastPressed = 0;

//Floats
float jumpLength;
float jumpPower;


int main(void)
{
//--->          WINDOW CREATION         <---
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(1024, 768, "Obelisks", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
//--->          END WINDOW CREATION         <---

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Use back face culling
    glEnable(GL_CULL_FACE);

    // Ensure we can capture keyboard inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Capture mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Compile shader program
    unsigned int shaderID, lightShaderID;
    shaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    lightShaderID = LoadShaders("lightVertexShader.glsl", "lightFragmentShader.glsl");

    // Activate shader
    glUseProgram(shaderID);

    // Load models
    Model obelisk("../assets/cube.obj");
    Model sphere("../assets/sphere.obj");
    Model collisionBox("../assets/cube.obj");
    Model platform("../assets/cube.obj");

    //Platform textures/light properties
    platform.addTexture("../assets/neutral_specular.png", "specular");
    platform.addTexture("../assets/bricks_diffuse.png", "diffuse");
    platform.addTexture("../assets/bricks_normal.png", "normal");

    //Define platform props
    platform.ka = 0.2f;
    platform.kd = 1.0f;
    platform.ks = 1.0f;
    platform.Ns = 20.0f;

    //Obelisk textures, light properties and positions
    obelisk.addTexture("../assets/stones_diffuse.png", "diffuse");

    obelisk.ka = 0.2f;
    obelisk.kd = 1.0f;
    obelisk.ks = 1.0f;
    obelisk.Ns = 20.0f;

    glm::vec3 positions[] = { //X, Y, Z
    glm::vec3(0.0f,  0.0f, 4.0f),
    glm::vec3(-4.0f,  0.0f,  2.0f),
    glm::vec3(-4.0f,  0.0f,  -2.0f),
    glm::vec3(0.0f,  0.0f,  -4.0f),
    glm::vec3(4.0f,  0.0f,  -2.0f),
    glm::vec3(4.0f,  0.0f,  2.0f),
    };

    //Collision box textures/light properties
    collisionBox.ka = 0.2f;
    collisionBox.kd = 1.0f;
    collisionBox.ks = 1.0f;
    collisionBox.Ns = 20.0f;

    collisionBox.addTexture("../assets/stones_diffuse.png", "diffuse");

    //Floor textures/light properties
    Model floor("../assets/plane.obj");
    floor.addTexture("../assets/stones_diffuse.png", "diffuse");
    floor.addTexture("../assets/stones_normal.png", "normal");
    floor.addTexture("../assets/stones_specular.png", "specular");

    floor.ka = 0.2f;
    floor.kd = 1.0f;
    floor.ks = 1.0f;
    floor.Ns = 20.0f;

    // Add light sources
    Light lightSources;

    //Spotlight
    lightSources.addSpotLight(glm::vec3(0.0f, 3.0f, 0.0f),          // position
        glm::vec3(0.0f, -1.0f, 0.0f),                               // direction
        glm::vec3(1.0f, 1.0f, 1.0f),                                // colour
        1.0f, 0.8f, 0.02f,                                          // attenuation
        std::cos(Maths::radians(60.0f)));                           // cos(phi)

    //Pointlight from array
    std::array<int, 6> xPos = { 0.0f, -4.0f, -4.0f, 0.0f, 4.0f, 4.0f };
    std::array<int, 6> zPos = { 4.0f, 2.0f, -2.0f, -4.0f, -2.0f, 2.0f };

    for (int i = 0; i < 6; i++) {
        lightSources.addPointLight(glm::vec3((xPos[i]), -30, (zPos[i])),      // position
            glm::vec3(0.0f, 1.0f, 1.0f),                                      // colour
            0.002f, 20.0f, 0.002f);                                           // attenuation
    }

    //Establish object vector
    std::vector<Object> objects;
    Object object;

    //Platform
    object.name = "platform";
    object.position = glm::vec3(0, -0.8f, 0);
    object.scale = glm::vec3(1.0f, 0.2f, 1.0f);
    objects.push_back(object);

    //Collision Box
    object.name = "collisionBox";
    object.position = camera.eye;
    object.scale = glm::vec3(0.2f, 0.2f, 0.2f);
    objects.push_back(object);

    //Obelisks
    object.name = "obelisk";
    for (unsigned int i = 0; i < 6; i++)
    {
        object.position = positions[i];
        object.rotation = glm::vec3(0.0f, 60.0f * (i), 1.0f);
        object.scale = glm::vec3(0.2f, 2.0f, 0.2f);
        object.angle = Maths::radians(60.0f * i);
        objects.push_back(object);
    }
    
    //Floor
    object.name = "floor";
    for (unsigned int i = 0; i < 1; i++)
    {
        // Add floor model to objects vector
        object.position = glm::vec3(0.0f, -0.85f, 0.0f);
        object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
        object.angle = 0.0f;
        object.name = "floor";
        objects.push_back(object);
    }

    //--->          RENDER LOOP         <---
    while (!glfwWindowShouldClose(window))
    {
        //Ensure player can't float
        camera.eye.y = 0.0f;

        //Update timer
        float time = glfwGetTime();
        deltaTime = time - previousTime;
        previousTime = time;

        //Get inputs
        keyboardInput(window);
        mouseInput(window);

        //Clear the window
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Calculate view and projection matrices
        camera.target = camera.eye + camera.front;

        //Determine if first or third person camera is needed
        if (useThirdPerson == 0)
        {
            camera.quaternionCamera();
        }
        else
        {
            camera.thirdPersonCamera();
        }

        //Activate shader
        glUseProgram(shaderID);

        //Send light source properties to the shader
        lightSources.toShader(shaderID, camera.view);

        //Loop through objects
        for (unsigned int i = 0; i < static_cast<unsigned int>(objects.size()); i++)
        {
            //Calculate model matrix
            glm::mat4 translate = Maths::translate(objects[i].position);
            glm::mat4 scale = Maths::scale(objects[i].scale);
            glm::mat4 rotate = Maths::rotate(objects[i].angle, objects[i].rotation);
            glm::mat4 model = translate * rotate * scale;

            //Send the MVP and MV matrices to the vertex shader
            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

            //Draw the models
            if (objects[i].name == "collisionBox")
            {
                objects[i].position = glm::vec3(camera.eye.x, camera.eye.y - 0.6f, camera.eye.z); //Check if player is centralised
                positionVector = objects[i].position;
                if (camera.eye.x >= -0.9f && camera.eye.x <= 0.9f &&
                    camera.eye.z >= -0.9f && camera.eye.z <= 0.9f)
                {
                    centralised = true;
                }
                else
                {
                    centralised = false;
                }
                loggedYPos = objects[i].position.y * jumpPower; //Grab current Y position

                if (useThirdPerson == true)
                {
                    collisionBox.draw(shaderID);
                }
            }
            if (objects[i].name == "obelisk")
            {
                if (objects[i].position.x + 2.5f > camera.eye.x && //Check if player is close
                    objects[i].position.x - 2.5f < camera.eye.x &&
                    objects[i].position.z + 2.5f > camera.eye.z &&
                    objects[i].position.z - 2.5f < camera.eye.z)
                {

                    if (objects[i].position.y < 3)
                    {
                        objects[i].position = glm::vec3(objects[i].position.x, objects[i].position.y += 0.005f, objects[i].position.z);
                    }
                }
                else if (objects[i].position.y > 0)
                {
                    objects[i].position.y = objects[i].position.y - 0.005f;
                }
                obelisk.draw(shaderID);
            }
            if (objects[i].name == "floor")
            {
                floor.draw(shaderID);
            }
            if (objects[i].name == "platform")
            {
                if ((objects[i].position.x + 1.2f > camera.eye.x && //Check if player is colliding with platform
                    objects[i].position.x - 1.2f < camera.eye.x &&
                    objects[i].position.z + 1.2f > camera.eye.z &&
                    objects[i].position.z - 1.2f < camera.eye.z) && camera.eye.y <= 1.2f)
                {
                    if (upPressed == 1) {
                        camera.eye -= camera.front * 0.01f, camera.up - 1.0f;
                    }
                    if (downPressed == 1) {
                        camera.eye += camera.front * 0.01f, camera.up - 1.0f;
                    }
                    if (leftPressed == 1) {
                        camera.eye += camera.right * 0.01f, camera.up - 1.0f;
                    }
                    if (rightPressed == 1) {
                        camera.eye -= camera.right * 0.01f, camera.up - 1.0f;
                    }
                }
                platform.draw(shaderID);
            }
        }

        if (centralised == true) {
            lightSources.activated();
        }
        else
        {
            lightSources.deactivated();
        }

        //Draw light sources
        lightSources.draw(lightShaderID, camera.view, camera.projection, sphere);

        if (camera.pitch > 1.20f) {
            camera.pitch = 1.20f;
        }
        else if (camera.pitch < -0.5f) {
            camera.pitch = -0.5f;
        }

        //Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Cleanup
    floor.deleteBuffers();
    collisionBox.deleteBuffers();
    obelisk.deleteBuffers();
    platform.deleteBuffers();

    glDeleteProgram(shaderID);

    //Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}

//Check for keyboard input
void keyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float speed = 1;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speed = 5;
    else speed = 1;

    //Move the camera using WSAD keys

    //W - FORWARDS
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (!downPressed) {
            camera.eye += 2.0f * deltaTime * camera.front * speed; //value controls speed of camera
            upPressed = 1;
        }
        lastPressed = 0;
    }
    else {
        upPressed = 0;
    }

    //S - BACKWARDS
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (!upPressed) {
            camera.eye -= 2.0f * deltaTime * camera.front * speed;
            downPressed = 1;
        }
        lastPressed = 1;
    }
    else {
        downPressed = 0;
    }

    //A - LEFT
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.eye -= 2.0f * deltaTime * camera.right * speed;
        leftPressed = 1;
        lastPressed = 2;
    }
    else {
        leftPressed = 0;
    }

    //D - RIGHT
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.eye += 2.0f * deltaTime * camera.right * speed;
        rightPressed = 1;
        lastPressed = 3;
    }
    else {
        rightPressed = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        useThirdPerson = 0;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        useThirdPerson = 1;

    //SPACE - JUMP
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        if (hasJumped == false) {
            startJumpHeight = camera.eye.y;
            hasJumped = true;
        }

    if (hasJumped == true)
    {
        jumpLength = jumpLength + 0.0025f;
        jumpPower = sin(jumpLength);
        camera.eye.y = 0.8f + jumpPower;

        if (camera.eye.y <= startJumpHeight)
        {
            hasJumped = false;
            jumpLength = 0;
        }
    }
}

//Check for mouse input
void mouseInput(GLFWwindow* window)
{
    //Get mouse cursor position and reset to centre
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    //Update yaw and pitch angles
    camera.yaw += 0.005f * float(xPos - 1024 / 2);
    camera.pitch += 0.005f * float(768 / 2 - yPos);

    //Limit camera pitch amount
    if (camera.pitch > 0.5f) {
        camera.pitch == 0.5f;
    }
    else if (camera.pitch < -0.4f) {
        camera.pitch == -0.4f;
    }

    //Calculate camera vectors from the yaw and pitch angles
    camera.quaternionCamera();
}

