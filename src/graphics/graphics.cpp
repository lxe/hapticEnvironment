#include "graphics.h"
#include "../core/debug.h"
#include <sstream>
#include <iomanip>

/**
 * @file graphics.h
 * @file graphics.cpp
 * @brief Functions for setting up and starting the graphics loop.
 *
 * The graphics loop is the main loop of the program. Haptics runs in its own loop and messaging is
 * handled by a separate thread. The functions here are responsible for using GLFW libraries to
 * initialize and update the display.
 */

extern HapticData hapticsData;
extern ControlData controlData;
GraphicsData graphicsData;

/**
 * Creates and initializes a GLFW window, and stores a pointer to this window in the graphicsData
 * struct. This does not initialize the Chai3D graphics information. For that, @see initScene
 */

void initDisplay(void)
{
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting CHAI3D window initialization");
    
    graphicsData.stereoMode = C_STEREO_DISABLED;
    graphicsData.fullscreen = false;
    graphicsData.mirroredDisplay = false;
    if (!glfwInit()) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Failed GLFW initialization");
        cSleepMs(1000);
        return;
    }
    debug_log(__FILE__, __LINE__, __FUNCTION__, "GLFW initialized successfully");
    
    glfwSetErrorCallback(errorCallback);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Got video mode - Width: " + std::to_string(mode->width) + " Height: " + std::to_string(mode->height)).c_str());
    
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width-w);
    int y = 0.5 * (mode->height-h);
    
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting up window hints...");
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    if (graphicsData.stereoMode == C_STEREO_ACTIVE) {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }
    
    graphicsData.width = w;
    graphicsData.height = h;
    graphicsData.xPos = x;
    graphicsData.yPos = y;
    graphicsData.swapInterval = 1;  

    debug_log(__FILE__, __LINE__, __FUNCTION__, "Creating GLFW window...");
    graphicsData.window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!graphicsData.window) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Failed to create window");
        cSleepMs(1000);
        glfwTerminate();
        return;
    }
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Window created successfully");
    
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting up window properties...");
    glfwGetWindowSize(graphicsData.window, &graphicsData.width, &graphicsData.height);
    glfwSetWindowPos(graphicsData.window, graphicsData.xPos, graphicsData.yPos);
    glfwSetKeyCallback(graphicsData.window, keySelectCallback);
    glfwSetWindowSizeCallback(graphicsData.window, resizeWindowCallback);
    glfwMakeContextCurrent(graphicsData.window);
    glfwSwapInterval(graphicsData.swapInterval);
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Window properties set");

#ifdef GLEW_VERSION
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Initializing GLEW...");
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Failed to initialize GLEW library");
        glfwTerminate();
        return;
    }
    debug_log(__FILE__, __LINE__, __FUNCTION__, "GLEW initialized successfully");
#endif

    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("OpenGL Version: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION)))).c_str());
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("OpenGL Vendor: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR)))).c_str());
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("OpenGL Renderer: " + std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))).c_str());

    debug_log(__FILE__, __LINE__, __FUNCTION__, "CHAI3D window initialization complete");
}

/**
 * Initializes the world for Chai3D. Also creates and sets the camera viewing angle and lighting.
 */
void initScene(void)
{
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting scene initialization...");
    
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Creating world...");
    graphicsData.world = new cWorld();
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting world background...");
    graphicsData.world->m_backgroundColor.setBlack();
    debug_log(__FILE__, __LINE__, __FUNCTION__, "World created successfully");
    
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting up camera...");
    graphicsData.camera = new cCamera(graphicsData.world);
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Adding camera to world...");
    graphicsData.world->addChild(graphicsData.camera);
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting camera position...");
    graphicsData.camera->set(cVector3d(400.0, 0.0, 0.0),
                         cVector3d(0.0, 0.0, 0.0),
                         cVector3d(0.0, 0.0, 1.0));
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting camera mirror properties...");                     
    graphicsData.camera->setMirrorVertical(graphicsData.mirroredDisplay);
    graphicsData.camera->setMirrorHorizontal(graphicsData.mirroredDisplay);  
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Camera setup complete");

    debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting up lighting...");
    graphicsData.light = new cDirectionalLight(graphicsData.world);
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Adding light to camera...");
    graphicsData.camera->addChild(graphicsData.light); 
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Configuring light properties...");
    graphicsData.light->setEnabled(true);
    graphicsData.light->setLocalPos(0.0, 500.0, 0.0);
    graphicsData.light->setDir(0.0, -1.0, 0.0);
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Lighting setup complete");
    
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Scene initialization complete");
}

/**
 * @param a_window Pointer to GLFW window.
 * @param a_width Initial window width 
 * @param a_height Initial window height
 *
 * This function is called when the user resizes the window.
 */
void resizeWindowCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    graphicsData.width = a_width;
    graphicsData.height = a_height;
}

/**
 * @param error Error code
 * @param errorDescription Error message
 *
 * This function is automatically called when a GLFW error is thrown
 */
void errorCallback(int error, const char* errorDescription)
{
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Error: " + std::string(errorDescription)).c_str());
}

/**
 * @param window Pointer to GLFW Window object 
 * @param key Name of the key that was pressed. Space key causes errors over message passing
 * @param scancode The scancode assigned to the key that was pressed 
 * @param action Type of action--should be GLFW_PRESS 
 * @param mods Modifier bits
 *
 * Callback function if a keyboard key is pressed. When a key is pressed, a message is sent to the
 * Trial Control module with a string representing the name of the key. 
 */
void keySelectCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Key pressed: " + std::to_string(key)).c_str());
    
    try {
        if ((action != GLFW_PRESS) && (action != GLFW_REPEAT)) {
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Ignoring non-press action");
            return;
        }
        else if ((key == GLFW_KEY_ESCAPE) || (key == GLFW_KEY_Q)) {
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Closing window");
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if(key == GLFW_KEY_F) {
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Toggling fullscreen");
            graphicsData.fullscreen = !graphicsData.fullscreen;
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if (graphicsData.fullscreen) {
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                glfwSwapInterval(graphicsData.swapInterval);
            }
            else {
                int w = 0.8 * mode->height;
                int h = 0.5 * mode->height;
                int x = 0.5 * (mode->width - w);
                int y = 0.5 * (mode->height - h);
                graphicsData.width = w;
                graphicsData.height = h;
                graphicsData.xPos = x;
                graphicsData.yPos = y;
                glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
                glfwSwapInterval(graphicsData.swapInterval);
            }
        }
        else {
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Processing regular key press");
            const char* key_name;
            if (key == 32) {
                key_name = "space";
            }
            else {
                key_name = glfwGetKeyName(key, 0);
            }
            
            if (!key_name) {
                debug_log(__FILE__, __LINE__, __FUNCTION__, "Warning: Could not get key name");
                return;
            }
            
            debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Key name: " + std::string(key_name)).c_str());
            
            M_KEYPRESS keypressEvent;
            memset(&keypressEvent, 0, sizeof(keypressEvent));
            
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Getting message number");
            auto packetNum = controlData.client->call("getMsgNum").as<int>();
            
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Getting timestamp");
            auto currTime = controlData.client->call("getTimestamp").as<double>();
            
            keypressEvent.header.serial_no = packetNum;
            keypressEvent.header.msg_type = KEYPRESS;
            keypressEvent.header.timestamp = currTime;
            memcpy(&(keypressEvent.keyname), key_name, sizeof(keypressEvent.keyname));
            
            char* packet[sizeof(keypressEvent)];
            memcpy(&packet, &keypressEvent, sizeof(keypressEvent));
            
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Sending message");
            auto res = controlData.client->call("sendMessage", (char* const) packet, sizeof(packet), controlData.MODULE_NUM).as<int>();
            
            if (res == 1) {
                debug_log(__FILE__, __LINE__, __FUNCTION__, "Successfully sent KEYPRESS message");
            }
            else {
                debug_log(__FILE__, __LINE__, __FUNCTION__, "Failed to send KEYPRESS message");
            }
        }
    } catch (const std::exception& e) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception in keySelectCallback: " + std::string(e.what())).c_str());
        throw;
    } catch (...) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Unknown exception in keySelectCallback");
        throw;
    }
}

/**
 * updateGraphics is called from the main loop and updates the graphics at each time step. Each
 * update involves updating the shadows and camera view, as well as rendering the updated position
 * of any moving objects. All moving objects must override the graphicsLoopFunction method.
 */
void updateGraphics(void)
{
    try {
        // debug_log(__FILE__, __LINE__, __FUNCTION__, "Updating graphics");
        graphicsData.world->updateShadowMaps(false, graphicsData.mirroredDisplay);
        graphicsData.camera->renderView(graphicsData.width, graphicsData.height);

        for(vector<cGenericMovingObject*>::iterator it = graphicsData.movingObjects.begin(); it != graphicsData.movingObjects.end(); it++)
        {
            double dt = (clock() - graphicsData.graphicsClock)/double(CLOCKS_PER_SEC);
            graphicsData.graphicsClock = clock();

            (*it)->graphicsLoopFunction(dt, hapticsData.tool->getDeviceGlobalPos(), hapticsData.tool->getDeviceGlobalLinVel());
        }
   
        glfwSwapBuffers(graphicsData.window);
        glFinish();
   
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("OpenGL Error: " + std::string((char*)gluErrorString(err))).c_str());
        }
    } catch (const std::exception& e) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception in updateGraphics: " + std::string(e.what())).c_str());
        throw;
    } catch (...) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Unknown exception in updateGraphics");
        throw;
    }
}
