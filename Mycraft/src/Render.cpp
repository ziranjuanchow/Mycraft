//
//  initialize.cpp
//  Mycraft
//
//  Created by Clapeysron on 14/11/2017.
//  Copyright © 2017 Clapeysron. All rights reserved.
//

#include "Render.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

bool Render::firstMouse = true;
float Render::yaw   =  -90.0f;
float Render::pitch =  0.0f;
float Render::fov   =  45.0f;
float Render::lastX =  800.0f / 2.0;
float Render::lastY =  600.0 / 2.0;
float Render::deltaTime = 0.0f;
glm::vec3 Render::cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 Render::cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

Render::Render() {
    lastFrame = 0.0f;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mycraft", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Render::initial(Game &game) {
    
    view = glm::lookAt(game.steve_position, game.steve_position + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    

    Block_Shader = Shader("shader/Block.vs","shader/Block.fs");
    texture_init();
}

void Render::texture_init() {
    glGenTextures(1, &texture_pic);
    glBindTexture(GL_TEXTURE_2D, texture_pic);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("picture/texture_big.bmp", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "****** Failed to load texture ******" << std::endl;
    }
    stbi_image_free(data);
}

void Render::render(Game& game) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window, game);
    game.gravity_move();
    projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    view = glm::lookAt(game.steve_position, game.steve_position + cameraFront, cameraUp);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, texture_pic);
    game.visibleChunks.draw(game.steve_position, view, projection, Block_Shader);
    /*char *ret = game.testchunk.readChunk();
    for(int i = 0; i < 256; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            for(int k = 0; k < 16; k++)
            {
                game.block.draw(glm::vec3(0.0f+j,0.0f+i,0.0f+k), view, projection, Block_Shader, ret[256*i+16*j+k]);
            }
        }
    }*/
    //game.block.test(view, projection, Block_Shader);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Render::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Render::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.9f)
        pitch = 89.9f;
    if (pitch < -89.9f)
        pitch = -89.9f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void Render::processInput(GLFWwindow *window, Game &game)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }
    float cameraSpeed = 6.0 * deltaTime;
    if (game.game_mode == GOD_MODE) {
        glm::vec3 cameraFront_XZ = cameraFront;
        cameraFront_XZ.y = 0;
        cameraFront_XZ = glm::normalize(cameraFront_XZ);
        glm::vec3 cameraRight_XZ = glm::cross(cameraFront, cameraUp);
        cameraRight_XZ.y = 0;
        cameraRight_XZ = glm::normalize(cameraRight_XZ);
        glm::vec3 cameraFront_Y = glm::vec3(0.0f, 1.0f, 0.0f);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            game.steve_position += cameraSpeed * cameraFront_XZ;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            game.steve_position -= cameraSpeed * cameraFront_XZ;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            game.steve_position -= cameraSpeed * cameraRight_XZ;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            game.steve_position += cameraSpeed * cameraRight_XZ;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            game.steve_position += cameraSpeed * cameraFront_Y;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            fov -= cameraSpeed*10;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            fov += cameraSpeed*10;
    } else if (game.game_mode == NORMAL_MODE) {
        glm::vec3 cameraFront_XZ = cameraFront;
        glm::vec3 new_position;
        cameraFront_XZ.y = 0;
        cameraFront_XZ = glm::normalize(cameraFront_XZ);
        glm::vec3 cameraRight_XZ = glm::cross(cameraFront, cameraUp);
        cameraRight_XZ.y = 0;
        cameraRight_XZ = glm::normalize(cameraRight_XZ);
        glm::vec3 cameraFront_Y = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            new_position = game.steve_position + cameraSpeed * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position + cameraSpeed * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            new_position = game.steve_position - cameraSpeed * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position - cameraSpeed * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            new_position = game.steve_position - cameraSpeed * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position - cameraSpeed * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            new_position = game.steve_position + cameraSpeed * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position + cameraSpeed * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            new_position = game.steve_position + cameraSpeed/10 * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position + cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            new_position = game.steve_position - cameraSpeed/10 * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position - cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            new_position = game.steve_position - cameraSpeed/10 * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position - cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            new_position = game.steve_position + cameraSpeed/10 * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
            game.move(new_position);
            new_position = game.steve_position + cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (game.vertical_v == 0) {
                game.vertical_v = JUMP_V/16.0f ;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            glm::vec3 new_position = game.steve_position;
            new_position -= cameraSpeed * cameraFront_Y;
            game.move(new_position);
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            fov -= cameraSpeed*10;
        }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            fov += cameraSpeed*10;
        }
    }
}