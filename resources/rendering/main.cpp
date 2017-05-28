#include "platform.hpp"

// third-party libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// standard C++ libraries
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <list>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>

// tdogl classes
#include "tdogl/Program.h"
#include "tdogl/Texture.h"
#include "tdogl/Camera.h"
#include "Model.h"

#define NUM_OBJ 32768
#define NUM_TSTEPS 1
#define E pow(NUM_OBJ,-0.3)


using namespace std;

struct Vertex {
    float x;
    float y;
    float z;
    float id;
    float accel;
};

// constants
const glm::vec2 SCREEN_SIZE(1600, 700);

// Globals
GLfloat vertexData[5*NUM_OBJ];
vector<vector<Vertex>> rawDataVec;

/*
vector<vector<float>> diskDataVec;
vector<vector<float>> bulgeDataVec;
vector<vector<float>> haloDataVec;
 */

GLfloat colorData[NUM_OBJ];

GLFWwindow* gWindow = NULL;
double gScrollY = 0.0;
tdogl::Camera gCamera;
ModelAsset gGalaxy;
ModelInstance gInstances;
int timeInt = 0;                                                                // time index to be rendered
int camMode = 0;                                                                // Camera mode (0: free, 1: above fixed, 2: side fixed)
int backColor = 0;                                                              // background color (0, 1, 2)
bool autoRun = false;                                                               // auto running toggle
int revIdx = -1;                                                               // selective rendering of objects
int galIdx = 0;
int mappingMode = 1;                                                            // Color mapping of objects (see fragment shader for detail)
string mapping = "uniform";

bool sortByDist(const Vertex &i, const Vertex &j) {
    
    // Works!!!
    float xi = i.x - gCamera._position.x;
    float yi = i.y - gCamera._position.y;
    float zi = i.z - gCamera._position.z;
    
    float xj = j.x - gCamera._position.x;
    float yj = j.y - gCamera._position.y;
    float zj = j.z - gCamera._position.z;
    
    float cx = sin(M_PI*gCamera._horizontalAngle/180.0);
    float cy = sin(-M_PI*gCamera._verticalAngle/180.0);
    float cz = -cos(M_PI*gCamera._horizontalAngle/180.0);
    
    float iDist = cx*xi + cy*yi + cz*zi;
    float jDist = cx*xj + cy*yj + cz*zj;
    
    return iDist > jDist;
     
    // doesn't work
    //return i.z < j.z;
    
    //return sqrt((i.z+10.0)*(i.z+10.0) + (i.x+15.0)*(i.x+15.0)) < sqrt((j.z+10.0)*(j.z+10.0) + (j.x+15.0)*(j.x+15.0));
    
    // Not quite right, assumes look vector is always in -z direction
    /*
    float x0 = 119.0;
    float m = tan(50.0);
    
    float ix = (i.x * (x0 - m*i.z))/x0;
    float iy = (i.y * (x0 - m*i.z))/x0;
    
    float jx = (j.x * (x0 - m*j.z))/x0;
    float jy = (j.y * (x0 - m*j.z))/x0;
    
    return sqrt((gCamera._position.x-ix)*(gCamera._position.x-ix)+(gCamera._position.y-iy)*(gCamera._position.y-iy)+(gCamera._position.z-i.z)*(gCamera._position.z-i.z)) > sqrt((gCamera._position.x-jx)*(gCamera._position.x-jx)+(gCamera._position.y-jy)*(gCamera._position.y-jy)+(gCamera._position.z-j.z)*(gCamera._position.z-j.z));
     */
    
    // try to sort post-projection
    /*
    glm::vec4 posI = {i.x,i.y,i.z,1};
    glm::vec4 posJ = {j.x,j.y,j.z,1};
    glm::mat4 projection = gCamera.view();
    
    glm::vec4 posIC = projection*posI;
    glm::vec4 posJC = projection*posJ;
    glm::vec3 posICv = {posIC.x,posIC.y,posIC.y};
    glm::vec3 posJCv = {posJC.x,posJC.y,posJC.y};
    
    float iSep = glm::length(gCamera._position-posICv);
    float jSep = glm::length(gCamera._position-posJCv);
    
    return iSep > jSep;
     */
    
    //return sqrt((gCamera._position.x-i.x)*(gCamera._position.x-i.x)+(gCamera._position.y-i.y)*(gCamera._position.y-i.y)+(gCamera._position.z-i.z)*(gCamera._position.z-i.z)) > sqrt((gCamera._position.x-j.x)*(gCamera._position.x-j.x)+(gCamera._position.y-j.y)*(gCamera._position.y-j.y)+(gCamera._position.z-j.z)*(gCamera._position.z-j.z));
}

// returns a new tdogl::Program created from the given vertex and fragment shader filenames
static tdogl::Program* LoadShaders(const char* vertFilename, const char* fragFilename) {
    std::vector<tdogl::Shader> shaders;
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(vertFilename), GL_VERTEX_SHADER));
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(fragFilename), GL_FRAGMENT_SHADER));
    return new tdogl::Program(shaders);
}


// returns a new tdogl::Texture created from the given filename
static tdogl::Texture* LoadTexture(const char* filename) {
    tdogl::Bitmap bmp = tdogl::Bitmap::bitmapFromFile(ResourcePath(filename));
    bmp.flipVertically();
    return new tdogl::Texture(bmp);
}



static void LoadGalaxyAsset() {
    
    gGalaxy.shaders = LoadShaders("vertex-shader.txt", "fragment-shader.txt");
    gGalaxy.drawType = GL_POINTS;
    gGalaxy.drawStart = 0;
    gGalaxy.drawCount = NUM_OBJ;
    // gGalaxy.texture = LoadTexture("");
    glGenBuffers(1, &gGalaxy.vbo);
    glGenVertexArrays(1, &gGalaxy.vao);
    
    // bind the VAO
    glBindVertexArray(gGalaxy.vao);
    
    // bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, gGalaxy.vbo);
    
    sort(rawDataVec.at(0).begin(), rawDataVec.at(0).end(), sortByDist);
    
    for (int i=0; i!=NUM_OBJ; ++i) {
        vertexData[5*i+0] = rawDataVec.at(0).at(i).x;
        vertexData[5*i+1] = rawDataVec.at(0).at(i).y;
        vertexData[5*i+2] = rawDataVec.at(0).at(i).z;
        vertexData[5*i+3] = rawDataVec.at(0).at(i).id;
        vertexData[5*i+4] = rawDataVec.at(0).at(i).accel;
    }

     
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    
    // connect the xyz to the "vert" attribute of the vertex shader
    glEnableVertexAttribArray(gGalaxy.shaders->attrib("vert"));
    glVertexAttribPointer(gGalaxy.shaders->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), NULL);
    
    glEnableVertexAttribArray(gGalaxy.shaders->attrib("ID"));
    glVertexAttribPointer(gGalaxy.shaders->attrib("ID"), 1, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));
    
    glEnableVertexAttribArray(gGalaxy.shaders->attrib("accel"));
    glVertexAttribPointer(gGalaxy.shaders->attrib("accel"), 1, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const GLvoid*)(4 * sizeof(GLfloat)));
    
    //glEnableVertexAttribArray(gGalaxy.shaders->attrib("rand"));
    //glVertexAttribPointer(gGalaxy.shaders->attrib("rand"), 1, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const GLvoid*)(4 * sizeof(GLfloat)));
    
    // connect the uv coords to the "vertTexCoord" attribute of the vertex shader
    //glEnableVertexAttribArray(gGalaxy.shaders->attrib("vertTexCoord"));
    //glVertexAttribPointer(gWoodenCrate.shaders->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,  5*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));
    
    // unbind
    glBindVertexArray(0);
    //glBindBuffer(GL_ARRAY_BUFFER,0);
    
}

static void updateGalaxy() {
    
    sort(rawDataVec.at(timeInt).begin(), rawDataVec.at(timeInt).end(), sortByDist);
    
    for (int i=0; i!=NUM_OBJ; ++i) {
        vertexData[5*i+0] = rawDataVec.at(timeInt).at(i).x;
        vertexData[5*i+1] = rawDataVec.at(timeInt).at(i).y;
        vertexData[5*i+2] = rawDataVec.at(timeInt).at(i).z;
        vertexData[5*i+3] = rawDataVec.at(timeInt).at(i).id;
        vertexData[5*i+4] = rawDataVec.at(timeInt).at(i).accel;
    }
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
}

//create all the `instance` structs for the 3D scene, and add them to `gInstances`
static void CreateInstances() {
    ModelInstance plummer;
    plummer.asset = &gGalaxy;
    gInstances = plummer;
}


//renders a single `ModelInstance`
static void RenderInstance(const ModelInstance& inst) {
    ModelAsset* asset = inst.asset;
    tdogl::Program* shaders = asset->shaders;
    
    //bind the shaders
    shaders->use();
    
    //set the shader uniforms
    shaders->setUniform("camera", gCamera.matrix());
    shaders->setUniform("model", inst.transform);
    shaders->setUniform("mapMode", mappingMode);
    
    //shaders->setUniform("center", {movedPosX,movedPosZ,movedPosY});
    shaders->setUniform("center", {0.0,0.0,0.0});
    shaders->setUniform("revIndex", revIdx);
    shaders->setUniform("lightDir", {0.0,0.0,1.0});
    shaders->setUniform("cameraPos", gCamera._position);
    //shaders->setUniform("time", timeInt);
    
    //shaders->setUniform("tex", 0); //set to 0 because the texture will be bound to GL_TEXTURE0
    
    //bind the texture
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, asset->texture->object());
    
    //bind VAO and draw
    
    glBindVertexArray(asset->vao);
    glDrawArrays(asset->drawType, asset->drawStart, asset->drawCount);
    
    //unbind everything
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    shaders->stopUsing();
}

// draws a single frame
static void Render() {
    // clear everything
    if (backColor == 0)
        glClearColor(0.0, 0.0, 0.0, 1.0); // black
    else if (backColor == 1)
        glClearColor(0.01,0.01,0.2,1.0);
    else if (backColor == 2)
        glClearColor(1.0,1.0,1.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderInstance(gInstances);
    // swap the display buffers (displays what was just drawn)
    glfwSwapBuffers(gWindow);
}

// update the scene based on the time elapsed since last update
static void Update(float secondsElapsed) {
    //move position of camera based on WASD keys, and XZ keys for up and down
    const float moveSpeed = 20.0; //units per second
    if(glfwGetKey(gWindow, 'S')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.forward());
        updateGalaxy();
    } else if(glfwGetKey(gWindow, 'W')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.forward());
        updateGalaxy();
    }
    if(glfwGetKey(gWindow, 'A')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.right());
        updateGalaxy();
    } else if(glfwGetKey(gWindow, 'D')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.right());
        updateGalaxy();
    }
    if(glfwGetKey(gWindow, 'Z')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * -glm::vec3(0,1,0));
        updateGalaxy();
    } else if(glfwGetKey(gWindow, 'X')){
        gCamera.offsetPosition(secondsElapsed * moveSpeed * glm::vec3(0,1,0));
        updateGalaxy();
    }
    
    
    if(glfwGetKey(gWindow, 'L')) {
        
        
        if (timeInt < NUM_TSTEPS-1) {
            timeInt += 1;
            //generatingPoints = true;
            
            
            /*
             if (timeInt < NUM_TSTEPS-1)
             timeInt += 1;
             */
            updateGalaxy();
        }
    } else if(glfwGetKey(gWindow, 'K')) {
        if (timeInt > 0) {
            timeInt -= 1;
            
            updateGalaxy();
        }
    }
    
    
    if(glfwGetKey(gWindow, '1')) {
        mappingMode = 1;
        mapping = "uniform";
        //colorMappingUpdate();
        Render();
    }
    else if(glfwGetKey(gWindow,'2')) {
        mappingMode = 2;
        mapping = "acceleration";
        //colorMappingUpdate();
        Render();
    }
    else if(glfwGetKey(gWindow,'3')) {
        mappingMode = 3;
        mapping = "distance (from center)";
        //colorMappingUpdate();
        Render();
    }
    else if(glfwGetKey(gWindow,'4')) {
        mappingMode = 4;
        mapping = "origin (realistic)";
        //colorMappingUpdate();
        Render();
    }
    else if(glfwGetKey(gWindow, '5')) {
        mappingMode = 5;
        mapping = "origin (hi contrast)";
        //colorMappingUpdate();
        Render();
    } /*else if(glfwGetKey(gWindow, '6')) {
       mappingMode = 6;
       //colorMappingUpdate();
       Render();
       }
       else if(glfwGetKey(gWindow, '7')) {
       //mappingMode = 7;
       //colorMappingUpdate();
       Render();
       }
       */
    
    
    if(glfwGetKey(gWindow, GLFW_KEY_RIGHT)) {
        if (revIdx < 3) {
            ++revIdx;
            double time_0 = glfwGetTime();
            double delay;
            do {
                delay = glfwGetTime() - time_0;
            } while (delay < 0.2);
        }
    } else if (glfwGetKey(gWindow, GLFW_KEY_LEFT)) {
        if (revIdx > -1) {
            --revIdx;
            double time_0 = glfwGetTime();
            double delay;
            do {
                delay = glfwGetTime() - time_0;
            } while (delay < 0.2);
        }
    }
    
    //rotate camera based on mouse movement
    const float mouseSensitivity = 0.1f;
    double mouseX, mouseY;
    glfwGetCursorPos(gWindow, &mouseX, &mouseY);
    if (mouseX!=0 || mouseY!=0) {
        gCamera.offsetOrientation(mouseSensitivity * (float)mouseY, mouseSensitivity * (float)mouseX);
        updateGalaxy();
        glfwSetCursorPos(gWindow, 0, 0); //reset the mouse, so it doesn't go out of the window
    }
    //increase or decrease field of view based on mouse wheel
    const float zoomSensitivity = -0.2f;
    float fieldOfView = gCamera.fieldOfView() + zoomSensitivity * (float)gScrollY;
    if(fieldOfView < 5.0f) fieldOfView = 5.0f;
    if(fieldOfView > 130.0f) fieldOfView = 130.0f;
    gCamera.setFieldOfView(fieldOfView);
    gScrollY = 0;
    
    if(glfwGetKey(gWindow, '0'))
        camMode = 0;
    else if (glfwGetKey(gWindow, '9'))
        camMode = 1;
    else if (glfwGetKey(gWindow, '8'))
        camMode = 2;
    else if (glfwGetKey(gWindow, '7'))
        camMode = 3;
    else if (glfwGetKey(gWindow, '6'))
        camMode = 4;
    
    /*
    if (camMode == 1) {
        gCamera.setPosition(glm::vec3((float)readDataVec.at(timeInt).at(0) / 20.0, (float)readDataVec.at(timeInt).at(1) / 20.0, (float)readDataVec.at(timeInt).at(2) / 20.0 + 7.5));
        gCamera._horizontalAngle = 0.0;
        gCamera._verticalAngle = 0.0;
    } else if (camMode == 2) {
        gCamera.setPosition(glm::vec3((float)readDataVec.at(timeInt).at(4) / 20.0, (float)readDataVec.at(timeInt).at(5) / 20.0, (float)readDataVec.at(timeInt).at(6) / 20.0 + 7.5));
        gCamera._horizontalAngle = 0.0;
        gCamera._verticalAngle = 0.0;
    } else if (camMode == 3) {
        gCamera.setPosition(glm::vec3((float)readDataVec.at(timeInt).at(4) / 20.0 + 5.0, (float)readDataVec.at(timeInt).at(5) / 20.0, (float)readDataVec.at(timeInt).at(6) / 20.0));
        gCamera._horizontalAngle = -90.0;
        gCamera._verticalAngle = 0.0;
    } else if (camMode == 4) {
        gCamera.setPosition(glm::vec3((float)readDataVec.at(timeInt).at(0) / 20.0 - 5.0, (float)readDataVec.at(timeInt).at(1) / 20.0, (float)readDataVec.at(timeInt).at(2) / 20.0));
        gCamera._horizontalAngle = 90.0;
        gCamera._verticalAngle = 0.0;
    }
     */
    
    if(glfwGetKey(gWindow, 'T')) {
        backColor = 0;
        updateGalaxy();
    }
    else if (glfwGetKey(gWindow, 'Y')) {
        backColor = 1;
        updateGalaxy();
    }
    else if (glfwGetKey(gWindow, 'R')) {
        backColor = 2;
        updateGalaxy();
    }
    
    if(glfwGetKey(gWindow, 'C'))
        autoRun = false;
    else if(glfwGetKey(gWindow,'V'))
        autoRun = true;
    
    if (autoRun) {
        if (timeInt < NUM_TSTEPS-1) {
            timeInt +=1;
            updateGalaxy();
        }
        
    }
    if(glfwGetKey(gWindow, '-')) {
        timeInt = 0;
        updateGalaxy();
    }
    
    else if(glfwGetKey(gWindow,'=')) {
        timeInt = NUM_TSTEPS-1;
        updateGalaxy();
    }
}

// records how far the y axis has been scrolled
void OnScroll(GLFWwindow* window, double deltaX, double deltaY) {
    gScrollY += deltaY;
    updateGalaxy();
}

void OnError(int errorCode, const char* msg) {
    throw std::runtime_error(msg);
}

// the program starts here
void AppMain() {
    // initialise GLFW
    glfwSetErrorCallback(OnError);
    if(!glfwInit())
        throw std::runtime_error("glfwInit failed");
    
    // open a window with GLFW
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    gWindow = glfwCreateWindow((int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y, "OpenGL Tutorial", NULL, NULL);
    if(!gWindow)
        throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 3.2?");
    
    // GLFW settings
    glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(gWindow, 0, 0);
    glfwSetScrollCallback(gWindow, OnScroll);
    glfwMakeContextCurrent(gWindow);
    
    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");
    
    // GLEW throws some errors, so discard all the errors so far
    while(glGetError() != GL_NO_ERROR) {}
    
    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // make sure OpenGL version 3.2 API is available
    if(!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");
    
    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    
    //colorMappingUpdate();
    LoadGalaxyAsset();
    
    // setup gCamera
    gCamera.setPosition(glm::vec3(0.0,0.0,10.0));
    gCamera.setViewportAspectRatio(SCREEN_SIZE.x / SCREEN_SIZE.y);
    
    // run while the window is open
    double lastTime = glfwGetTime();
    while(!glfwWindowShouldClose(gWindow)){
        // process pending events
        glfwPollEvents();
        
        CreateInstances();
        
        cout << "Time Index: " << timeInt << '\t' << "Displayed Galaxy: " << revIdx << '\t' << "Color Mapping: " << mapping << '\n';
        //cout << "Vertical Angle: " << gCamera._verticalAngle << '\t' << "Horizontal Angle: " << gCamera._horizontalAngle << '\n';
        
        // update the scene based on the time elapsed since last update
        double thisTime = glfwGetTime();
        Update((float)(thisTime - lastTime));
        lastTime = thisTime;
        
        // draw one frame
        Render();
        
        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR)
            std::cerr << "OpenGL Error " << error << std::endl;
        
        //exit program if escape key is pressed
        if(glfwGetKey(gWindow, GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(gWindow, GL_TRUE);
    }
    
    // clean up and exit
    glfwTerminate();
}


int main(int argc, char *argv[]) {
    /*
    // READ in data file
    std::ifstream rawDataDisk;
    std::ifstream rawDataBulge;
    std::ifstream rawDataHalo;
    rawDataDisk.open("/Users/willan/Documents/PHYS_141/FINAL_DATA/disk");
    rawDataBulge.open("/Users/willan/Documents/PHYS_141/FINAL_DATA/bulge");
    rawDataHalo.open("/Users/willan/Documents/PHYS_141/FINAL_DATA/halo");
    if (rawDataDisk.is_open() && rawDataBulge.is_open() && rawDataHalo.is_open()) {
        std::string in;
        for (int i=0; i!=NUM_TSTEPS; ++i) {
            vector<float> rawDataRow;
            rawDataDisk >> in;                      // ditch first two terms in file
            rawDataDisk >> in;
            
            rawDataBulge >> in;
            rawDataBulge >> in;
            
            rawDataHalo >> in;
            rawDataHalo >> in;
            
            
            for (int j=0; j!=4000; ++j) {           // disk
                rawDataDisk >> in;                      // shed mass term
                
                for (int k=0; k!=3; ++k) {
                    rawDataDisk >> in;                  // keep position terms (3)
                    float val = stof(in);
                    rawDataRow.push_back(val);
                }
                rawDataRow.push_back(0.0);              // Id value
                
                for (int k=0; k!=3; ++k) {              // shed velocity terms (3)
                    rawDataDisk >> in;
                }
            }
            
            for (int j=0; j!=2000; ++j) {           // bulge
                rawDataBulge >> in;
                
                for (int k=0; k!=3; ++k) {
                    rawDataBulge >> in;
                    float val = stof(in);
                    rawDataRow.push_back(val);
                }
                rawDataRow.push_back(1.0);
                
                for (int k=0; k!=3; ++k) {
                    rawDataBulge >> in;
                }
            }
            
            for (int j=0; j!=10000; ++j) {           // halo
                rawDataHalo >> in;
                
                for (int k=0; k!=3; ++k) {
                    rawDataHalo >> in;
                    float val = stof(in);
                    rawDataRow.push_back(val);
                }
                rawDataRow.push_back(2.0);
                
                for (int k=0; k!=3; ++k) {
                    rawDataHalo >> in;
                }
            }
             
            rawDataVec.push_back(rawDataRow);
        }
        rawDataDisk.close();
        rawDataBulge.close();
        rawDataHalo.close();
        
    } else { std::cout << "Unable to open file\n"; }
     */
    
    
    /*
     for (int i=0; i!=NUM_OBJ/2; ++i) {
     int idx = i*5+4;
     int counter = i;
     vertexData[idx] = 0.0;
     }
     
     for (int i=NUM_OBJ/2; i!=NUM_OBJ; ++i) {
     int idx = i*5+4;
     int counter = i;
     vertexData[idx] = 1.0;
     }
     */
    
    /*
    ifstream rawData;
    //rawData.open("/Users/willan/Documents/PHYS_141/system.txt");
    rawData.open("/Users/willan/Documents/PHYS_141/galaxyRun.data");
    if (rawData.is_open()) {
        string in;
        for (int j=0; j!=NUM_TSTEPS; ++j) {
            vector<Vertex> rawDataRow;
            
            for (int l=0; l!= 4000; ++l) {
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                float val_id = 0.0;
                
     
                float a = -0.65;
                float b = 0.31;
                float width = 0.05;
                float theta = atan2(val_y, val_x);
                theta += M_PI;
                float xSpiral = a*cos(theta)*exp(b*theta);
                float ySpiral = a*sin(theta)*exp(b*theta);
                
                if (abs(val_x-xSpiral) < width && abs(val_y-ySpiral) < width) {
                    val_id = 3.0;
                }
                
                
                a = -0.65;
                b = 0.30;
                theta = atan2(val_y, val_x);
                theta += 2.0*M_PI;
                xSpiral = a*cos(theta)*exp(b*theta);
                ySpiral = a*sin(theta)*exp(b*theta);
                if (abs(val_x-xSpiral) < width && abs(val_y-ySpiral) < width) {
                    val_id = 3.0;
                }
     
     
                a = 3.0*M_PI/2.0;
                xSpiral = a*cos(theta)*exp(b*theta);
                ySpiral = a*sin(theta)*exp(b*theta);
                if (abs(val_x-xSpiral) < width && abs(val_y-ySpiral) < width) {
                    val_id = 3.0;
                }
                
                a = 2.3*M_PI;
                xSpiral = a*cos(theta)*exp(b*theta);
                ySpiral = a*sin(theta)*exp(b*theta);
                if (abs(val_x-xSpiral) < width && abs(val_y-ySpiral) < width) {
                    val_id = 3.0;
                }
     
                rawDataRow.push_back({val_x, val_y, val_z, val_id});
                rawData >> in;                                                // ditching accel term
            }
            
            for (int l=0; l!= 2000; ++l) {
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                float val_id;
                if (1)
                    val_id = 1.0;
                else
                    val_id = 1.0;
                
                rawDataRow.push_back({val_x, val_y, val_z, val_id});
                rawData >> in;
            }
            
            
            for (int l=0; l!=10000; ++l) {
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                float val_id = 2.0;
                //rawDataRow.push_back({val_x, val_y, val_z, val_id});
                rawData >> in;
            }
     
            for (int l=0; l!=384; ++l) {
                rawData >> in;
                rawData >> in;
                rawData >> in;
                rawData >> in;
            }
     
            rawDataVec.push_back(rawDataRow);
        }
        rawData.close();
    } else { cout << "Unable to open file\n"; }
     */
    
    //Sort each row of rawDataVec
    /*
    for (int e=0; e!=NUM_TSTEPS; ++e) {
        sort(rawDataVec.at(e).begin(), rawDataVec.at(e).end(), sortByDist);
    }
     */
    
    ifstream rawData;
    rawData.open("/Users/willan/Documents/PHYS_141/FINAL_DATA/output");
    if (rawData.is_open()) {
        string in;
        //rawData >> in;
        //rawData >> in;
        //rawData >> in;
        for (int j=0; j!=NUM_TSTEPS; ++j) {
            vector<Vertex> rawDataRow;
            for (int w=0; w!=2; ++w) {
            for (int l=0; l!= 4000; ++l) {
                rawData >> in;
                //float val_id = stof(in);
                float val_id = 0.0;
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                //rawData >> in;
                //float val_accel = stof(in);
                rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
            }
            
            for (int l=0; l!= 2000; ++l) {
                rawData >> in;
                //float val_id = stof(in);
                float val_id = 1.0;
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                //rawData >> in;
                //float val_accel = stof(in);
                rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
            }
            
            for (int l=0; l!=10000; ++l) {
                rawData >> in;
                //float val_id = stof(in);
                float val_id = 2.0;
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                //rawData >> in;
                //float val_accel = stof(in);
                rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
            }
            
             for (int l=0; l!=384; ++l) {
                 rawData >> in;
                 rawData >> in;
                 rawData >> in;
                 rawData >> in;
             }
            }
        
            rawDataVec.push_back(rawDataRow);
        }
        rawData.close();
    } else { cout << "Unable to open file\n"; }
  
  /*
  ifstream rawData;
  rawData.open("/Users/willan/Documents/PHYS_141/combinedRunFormatted");
  if (rawData.is_open()) {
    string in;
    for (int j=0; j!=NUM_TSTEPS; ++j) {
      vector<Vertex> rawDataRow;
      for (int w=0; w!=2; ++w) {
        for (int l=0; l!= 16384; ++l) {
          rawData >> in;
          float val_id = stof(in);
          rawData >> in;
          float val_x = stof(in);
          rawData >> in;
          float val_y = stof(in);
          rawData >> in;
          float val_z = stof(in);
          rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
        }
      }
      
      rawDataVec.push_back(rawDataRow);
    }
    rawData.close();
  } else { cout << "Unable to open file\n"; }
*/
    /*
    ifstream rawData;
    rawData.open("/Users/willan/Documents/PHYS_141/system2.txt");
    if (rawData.is_open()) {
        string in;
     
        for (int j=0; j!=NUM_TSTEPS; ++j) {
            vector<Vertex> rawDataRow;
            for (int l=0; l!= 4000; ++l) {
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                float val_id = 0.0;
                rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
            }
            
            for (int l=0; l!= 2000; ++l) {
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stof(in);
                float val_id = 1.0;
                rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
            }
            
            for (int l=0; l!=10000; ++l) {
                rawData >> in;
                rawData >> in;
                rawData >> in;
            }
            
            for (int l=0; l!= 4000; ++l) {
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stold(in);
                float val_id = 3.0;
                rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
            }
            
            for (int l=0; l!= 2000; ++l) {
                rawData >> in;
                float val_x = stof(in);
                rawData >> in;
                float val_y = stof(in);
                rawData >> in;
                float val_z = stold(in);
                float val_id = 4.0;
                rawDataRow.push_back({val_x, val_y, val_z, val_id, 0});
            }
            
            for (int l=0; l!=10000; ++l) {
                rawData >> in;
                rawData >> in;
                rawData >> in;
            }
            rawDataVec.push_back(rawDataRow);
        }
        rawData.close();
    } else { cout << "Unable to open file\n"; }
     */
    
    
    // Start Cocoa app
    try {
        AppMain();
    } catch (const std::exception& e){
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
    
}
