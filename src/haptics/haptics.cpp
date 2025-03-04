#include "haptics.h"
#include "platform_compat.h"
#include "../core/debug.h"
#include <sstream>
#include <iomanip>

/**
 * @file haptics.h 
 * @file haptics.cpp
 *
 * @brief Functions for haptic control.
 *
 * This file contains the initialization of the haptic device, as well as the update function that
 * is called in the haptics thread. 
 */

HapticData hapticsData;
extern GraphicsData graphicsData;
extern ControlData controlData;

/**
 * @brief Initializes the haptic thread. 
 *
 * Contains some custom scale factors depending on which device is
 * used (Falcon or delta.3)
 */
void initHaptics(void)
{
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting haptics initialization");
    try {
        hapticsData.handler = new cHapticDeviceHandler();
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Created haptic device handler");
        
        hapticsData.handler->getDevice(hapticsData.hapticDevice, 0);
        hapticsData.hapticDeviceInfo = hapticsData.hapticDevice->getSpecifications();
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Got haptic device info");
     
        bool open_success = hapticsData.hapticDevice->open();
        debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Opened Device: " + std::to_string(open_success)).c_str());
        
        bool calibrate_success = hapticsData.hapticDevice->calibrate(true);
        debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Calibrate succeeded: " + std::to_string(calibrate_success)).c_str());

        double workspaceScaleFactor;
        double forceScaleFactor;
        if (hapticsData.hapticDeviceInfo.m_model == C_HAPTIC_DEVICE_FALCON) {
            workspaceScaleFactor = 3000;
            forceScaleFactor = 3000;
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Falcon device detected");
        }
        else if (hapticsData.hapticDeviceInfo.m_model == C_HAPTIC_DEVICE_DELTA_3) {
            workspaceScaleFactor = 1000;
            forceScaleFactor = 1000;
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Delta device detected");
        }
        else {
            workspaceScaleFactor = 1000;
            forceScaleFactor = 1000;
            //hapticsData.hapticDevice->close();
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Device not recognized");
        }
      
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Creating haptic tool");
        hapticsData.tool = new cToolCursor(graphicsData.world);
        hapticsData.tool->m_hapticPoint->m_sphereProxy->m_material->setRed();
        graphicsData.world->addChild(hapticsData.tool);
        hapticsData.tool->setHapticDevice(hapticsData.hapticDevice);
        hapticsData.tool->setRadius(HAPTIC_TOOL_RADIUS);
        hapticsData.tool->setWorkspaceScaleFactor(workspaceScaleFactor);
        hapticsData.tool->setWaitForSmallForce(false);
        
        if (hapticsData.hapticDeviceInfo.m_model == C_HAPTIC_DEVICE_DELTA_3) {
            debug_log(__FILE__, __LINE__, __FUNCTION__, "Setting Delta device rotation");
            cMatrix3d rotate = cMatrix3d();
            rotate.set(0, 1, 0, 1, 0, 0, 0, 0, 1);
            hapticsData.tool->setDeviceGlobalRot(rotate);
        }
        
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting haptic tool");
        hapticsData.tool->start();
      
        hapticsData.maxForce = hapticsData.hapticDeviceInfo.m_maxLinearForce;
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Haptics initialization complete");
    } catch (const std::exception& e) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception in initHaptics: " + std::string(e.what())).c_str());
        throw;
    } catch (...) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Unknown exception in initHaptics");
        throw;
    }
}

/**
 * @brief Starts the haptic thread. 
 *
 * Stores a pointer to the haptic thread in the ControlData struct
 */
void startHapticsThread(void)
{
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting haptics thread");
    try {
        hapticsData.hapticsThread = new cThread();
        hapticsData.hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);
        controlData.simulationRunning = true;
        controlData.simulationFinished = false;
        controlData.hapticsUp = true;
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Haptics thread started successfully");
    } catch (const std::exception& e) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception in startHapticsThread: " + std::string(e.what())).c_str());
        throw;
    } catch (...) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Unknown exception in startHapticsThread");
        throw;
    }
}

/**
 * @brief Haptic update function 
 *
 * This function is called on each iteration of the haptic loop. It computes the global and local
 * positions of the device and renders any forces based on objects in the Chai3d world
 */
void updateHaptics(void)
{
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting haptics update loop");
    try {
        cPrecisionClock clock;
        clock.reset();
        cVector3d angVel(0.0, 0.0, 0.1);
        platform::usleep(500); // give some time for other threads to start up
        
        while (controlData.simulationRunning) {
            clock.stop();
            double timeInterval = clock.getCurrentTimeSeconds();
            clock.reset();
            clock.start();
            
            graphicsData.world->computeGlobalPositions(true);
            cVector3d pos = hapticsData.tool->getDeviceLocalPos();
            //cout << pos.x() << ", " << pos.y() << ", " << pos.z() << endl;
            hapticsData.tool->updateFromDevice();
            hapticsData.tool->computeInteractionForces();
            hapticsData.tool->applyToDevice();
        }
        
        controlData.hapticsUp = false;
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Haptics update loop ended");
    } catch (const std::exception& e) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception in updateHaptics: " + std::string(e.what())).c_str());
        throw;
    } catch (...) {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Unknown exception in updateHaptics");
        throw;
    }
}
