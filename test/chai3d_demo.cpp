#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef GLEW_VERSION
#include <GL/glew.h>
#endif

using namespace chai3d;
using namespace std;

//------------------------------------------------------------------------------
// CHAI3D
//------------------------------------------------------------------------------
cWorld* world;
cCamera* camera;
cDirectionalLight* light;
cMesh* sphere;
cMesh* cube;

//------------------------------------------------------------------------------
// GLFW 
//------------------------------------------------------------------------------
GLFWwindow* window = nullptr;
int windowWidth = 800;
int windowHeight = 600;

//------------------------------------------------------------------------------
// CALLBACKS
//------------------------------------------------------------------------------
void errorCallback(int error, const char* description)
{
    cout << "Error: " << description << endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void windowSizeCallback(GLFWwindow* window, int w, int h)
{
    windowWidth = w;
    windowHeight = h;
}

//------------------------------------------------------------------------------
// INITIALIZE CHAI3D
//------------------------------------------------------------------------------
bool initChai3D()
{
    cout << "Initializing CHAI3D..." << endl;
    
    try {
        cout << "Creating world..." << endl;
        // create a new world
        world = new cWorld();
        world->m_backgroundColor.setWhite();
        
        cout << "Creating camera..." << endl;
        // create a camera
        camera = new cCamera(world);
        world->addChild(camera);
        
        cout << "Setting camera position..." << endl;
        // position and orient the camera
        camera->set(cVector3d(0.5, 0.0, 0.0),    // camera position (eye)
                    cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
                    cVector3d(0.0, 0.0, 1.0));   // direction of the "up" vector
        
        cout << "Setting camera clipping planes..." << endl;
        // set the near and far clipping planes of the camera
        camera->setClippingPlanes(0.01, 10.0);
        
        cout << "Creating light source..." << endl;
        // create a light source
        light = new cDirectionalLight(world);
        camera->addChild(light);                   // attach light to camera
        light->setEnabled(true);                   // enable light source
        light->setLocalPos(cVector3d(2.0, 0.5, 1.0)); // position the light source
        
        cout << "Creating sphere..." << endl;
        // create a sphere
        sphere = new cMesh();
        world->addChild(sphere);
        cCreateSphere(sphere, 0.05);
        sphere->setLocalPos(-0.1, 0.0, 0.0);
        sphere->m_material->setRedCrimson();
        
        cout << "Creating cube..." << endl;
        // create a cube
        cube = new cMesh();
        world->addChild(cube);
        cCreateBox(cube, 0.1, 0.1, 0.1);
        cube->setLocalPos(0.1, 0.0, 0.0);
        cube->m_material->setBlueCornflower();
        
        cout << "CHAI3D initialization complete" << endl;
        return true;
    }
    catch (const std::exception& e) {
        cout << "Exception during CHAI3D initialization: " << e.what() << endl;
        return false;
    }
    catch (...) {
        cout << "Unknown exception during CHAI3D initialization" << endl;
        return false;
    }
}

//------------------------------------------------------------------------------
// INITIALIZE GLFW
//------------------------------------------------------------------------------
bool initGLFW()
{
    cout << "Initializing GLFW..." << endl;
    
    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW library" << endl;
        return false;
    }
    
    cout << "GLFW initialized successfully" << endl;
    
    // set error callback
    glfwSetErrorCallback(errorCallback);
    
    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (!mode) {
        cout << "Failed to get video mode" << endl;
        return false;
    }
    windowWidth = 0.8 * mode->height;
    windowHeight = 0.5 * mode->height;
    int x = 0.5 * (mode->width - windowWidth);
    int y = 0.5 * (mode->height - windowHeight);
    
    cout << "Creating window with dimensions: " << windowWidth << "x" << windowHeight << endl;
    
    // create display window
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    
    window = glfwCreateWindow(windowWidth, windowHeight, "CHAI3D Demo", NULL, NULL);
    if (!window)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return false;
    }
    
    cout << "Window created successfully" << endl;
    
    // set window position
    glfwSetWindowPos(window, x, y);
    
    // set key callback
    glfwSetKeyCallback(window, keyCallback);
    
    // set resize callback
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    
    cout << "Making OpenGL context current..." << endl;
    // set current display context
    glfwMakeContextCurrent(window);

#ifdef GLEW_VERSION
    // Initialize GLEW
    cout << "Initializing GLEW..." << endl;
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Failed to initialize GLEW: " << glewGetErrorString(err) << endl;
        return false;
    }
    cout << "GLEW initialized successfully" << endl;
#endif
    
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << endl;
    cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
    
    // set swap interval
    glfwSwapInterval(1);
    
    cout << "GLFW initialization complete" << endl;
    return true;
}

//------------------------------------------------------------------------------
// MAIN RENDERING LOOP
//------------------------------------------------------------------------------
void updateGraphics()
{
    // update shadow maps (not used in this example)
    world->updateShadowMaps(false, false);
    
    // render world
    camera->renderView(windowWidth, windowHeight);
    
    // swap buffers
    glfwSwapBuffers(window);
    
    // process events
    glfwPollEvents();
}

//------------------------------------------------------------------------------
// MAIN ENTRY POINT
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D Demo" << endl;
    cout << "-----------------------------------" << endl << endl;
    
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------
    
    try {
        // initialize GLFW
        if (!initGLFW())
        {
            cout << "Failed to initialize GLFW" << endl;
            return -1;
        }
        
        // initialize CHAI3D
        if (!initChai3D())
        {
            cout << "Failed to initialize CHAI3D" << endl;
            glfwTerminate();
            return -1;
        }
        
        if (!world || !camera || !light || !sphere || !cube) {
            cout << "Critical CHAI3D objects were not properly initialized" << endl;
            glfwTerminate();
            return -1;
        }
        
        cout << "All initialization successful, entering main loop" << endl;
        
        //--------------------------------------------------------------------------
        // MAIN GRAPHIC LOOP
        //--------------------------------------------------------------------------
        
        // main graphic loop
        while (!glfwWindowShouldClose(window))
        {
            // update graphics
            updateGraphics();
            
            // check error
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                cout << "OpenGL Error: " << err << endl;
            }
        }
    }
    catch (const std::exception& e) {
        cout << "Exception in main loop: " << e.what() << endl;
        return -1;
    }
    catch (...) {
        cout << "Unknown exception in main loop" << endl;
        return -1;
    }
    
    //--------------------------------------------------------------------------
    // CLEANUP
    //--------------------------------------------------------------------------
    
    // cleanup
    delete world;
    glfwDestroyWindow(window);
    glfwTerminate();
    
    // exit
    return 0;
} 