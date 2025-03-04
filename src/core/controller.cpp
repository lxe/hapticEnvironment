#include "controller.h"
#include "platform_compat.h"
#include "debug.h"
#include <csignal>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <dbghelp.h>

/**
 * @file controller.h
 * @file controller.cpp 
 * @brief This file contains the main function that runs the haptic, graphics, and message listening
 * loops.
 *
 * This file is in the global namespace. It accesses haptic data and graphics data through external
 * structs, and keeps track of all the messaging sockets and Chai3D threads through the ControlData
 * extern, which is accessible in other files. 
 */

extern HapticData hapticsData;
extern GraphicsData graphicsData;
ControlData controlData;

// Signal handler
void signal_handler(int sig) {
    std::stringstream ss;
    ss << "Received signal " << sig;
    debug_log(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str());
    print_stack_trace();
    exit(1);
}

// Set up signal handlers
void setup_signal_handlers() {
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGILL, signal_handler);
}

int main(int argc, char* argv[])
{
  setup_signal_handlers();
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting application");

  const char* MODULE_IP;
  int MODULE_PORT;
  const char* MH_IP;
  int MH_PORT;

  debug_log(__FILE__, __LINE__, __FUNCTION__, "\n-----------------------------------\nCHAI3D\n-----------------------------------\n\n");
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Keyboard Options:\n\n[f] - Enable/Disable full screen mode\n[q] - Exit application\n\n");
  
  controlData.simulationRunning = false;
  controlData.simulationFinished = true;
  controlData.hapticsUp = false;
  controlData.listenerUp = false;
  controlData.streamerUp = false;
  controlData.loggingData = false;

  // TODO: Set these IP addresses from a config file
  controlData.MODULE_NUM = 1;
  if (argc < 2) {
    controlData.IPADDR = "127.0.0.1";
    controlData.PORT = 7000;
  }
  else {
    controlData.IPADDR = argv[1];
    controlData.PORT = atoi(argv[2]);
  }
  if (argc <= 3) {
    controlData.MH_IP = "127.0.0.1";
    controlData.MH_PORT = 8080;
  }
  else {
    controlData.MH_IP = argv[3];
    controlData.MH_PORT = atoi(argv[4]);
  }
  controlData.client = new rpc::client(controlData.MH_IP, controlData.MH_PORT);
  controlData.hapticsOnly = false;
  
  if (controlData.hapticsOnly == false) {
    initDisplay();
    initScene();
  }

  debug_log(__FILE__, __LINE__, __FUNCTION__, "Display initialized");

  debug_log(__FILE__, __LINE__, __FUNCTION__, "*** Initializing Haptics ***");
  initHaptics();
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Haptics initialized");

  debug_log(__FILE__, __LINE__, __FUNCTION__, "*** Starting Haptics Thread ***");
  startHapticsThread(); 
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Haptics thread started");

  debug_log(__FILE__, __LINE__, __FUNCTION__, "*** Initializing Messaging ***");
  atexit(close);
  resizeWindowCallback(graphicsData.window, graphicsData.width, graphicsData.height);
  platform::sleep(2); 
  openMessagingSocket();
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Messaging socket opened");

  debug_log(__FILE__, __LINE__, __FUNCTION__, "*** Adding Message Handler Module ***");
  int addSuccess = addMessageHandlerModule();
  if (addSuccess == 0) {
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Module addition failed");
    close();
    exit(1);
  }
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Module addition successful");

  debug_log(__FILE__, __LINE__, __FUNCTION__, "*** Subscribing to Trial Control ***");
  platform::sleep(1);
  int subscribeSuccess = subscribeToTrialControl();
  if (subscribeSuccess == 0) {
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Subscribe to Trial Control failed");
    close();
    exit(1);
  }
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Subscribe to Trial Control successful");

  debug_log(__FILE__, __LINE__, __FUNCTION__, "*** Starting Streamer and Listener ***");
  platform::sleep(2);
  startStreamer(); 
  startListener();
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Streamer and listener started");

  while (!glfwWindowShouldClose(graphicsData.window)) {
    // debug_log(__FILE__, __LINE__, __FUNCTION__, "Main loop iteration");
    try {
      glfwGetWindowSize(graphicsData.window, &graphicsData.width, &graphicsData.height);
      graphicsData.graphicsClock = clock();
      updateGraphics();
      glfwPollEvents();
      graphicsData.freqCounterGraphics.signal(1);
    } catch (const std::exception& e) {
      debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception in main loop: " + std::string(e.what())).c_str());
      print_stack_trace();
      throw;
    } catch (...) {
      debug_log(__FILE__, __LINE__, __FUNCTION__, "Unknown exception in main loop");
      print_stack_trace();
      throw;
    }
  }
  
  glfwDestroyWindow(graphicsData.window);
  glfwTerminate();
  return(0);
}

/**
 * Checks if the haptics and messaging threads have exited yet. The graphics loop is in main, and
 * exits when all other threads are down, so it is not checked here.
 */
bool allThreadsDown()
{
  return (controlData.hapticsUp && controlData.listenerUp && controlData.streamerUp);  
}

/**
 * Ends the program. This method does so by setting the "simulationRunning" boolean to false. When
 * false, other threads will exit. To exit gracefully, this method waits until all threads have
 * returned before stopping the haptic tool and exiting the graphic interface.
 */
void close()
{
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Starting application close");
  controlData.simulationRunning = false;
  while (!controlData.simulationFinished) {
    controlData.simulationFinished = allThreadsDown();
    platform::sleep(100);
  }
  try {
    hapticsData.tool->stop();
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Haptic tool stopped");
    delete hapticsData.hapticsThread;
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Deleted haptics thread");
    delete graphicsData.world;
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Deleted world");
    delete hapticsData.handler;
    debug_log(__FILE__, __LINE__, __FUNCTION__, "Deleted handler");
    closeMessagingSocket();
    graphicsData.world->deleteAllChildren();
  } catch (const std::exception& e) {
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception during close: " + std::string(e.what())).c_str());
    print_stack_trace();
    throw;
  }
}

/**
 * This function receives packets from the listener threads and updates the haptic environment
 * variables accordingly.
 * @param packet is a pointer to a char array of bytes
 */
void parsePacket(char* packet)
{
  debug_log(__FILE__, __LINE__, __FUNCTION__, "Parsing packet");
  try {
    MSG_HEADER header;
    memcpy(&header, packet, sizeof(header));
    int msgType = header.msg_type;
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Message type: " + std::to_string(msgType)).c_str());
    switch (msgType)
    {
      case SESSION_START:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received SESSION_START Message");
        break;
      }

      case SESSION_END:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received SESSION_END Message");
        controlData.simulationRunning = false;
        close();
        break;
      }

      case TRIAL_START:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received TRIAL_START Message");
        break;
      }

      case TRIAL_END:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received TRIAL_END Message");
        break;
      }

      case START_RECORDING:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received START_RECORDING Message");
        M_START_RECORDING recInfo;
        memcpy(&recInfo, packet, sizeof(recInfo));
        char* fileName;
        fileName = recInfo.filename;
        controlData.dataFile.open(fileName, ofstream::binary);
        controlData.dataFile.flush();
        controlData.loggingData = true;
        break; 
      }

      case STOP_RECORDING:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received STOP_RECORDING Message");
        controlData.dataFile.close();
        controlData.loggingData = false;
        break;
      }
      
      case REMOVE_OBJECT:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received REMOVE_OBJECT Message");
        M_REMOVE_OBJECT rmObj;
        memcpy(&rmObj, packet, sizeof(rmObj));
        if (controlData.objectMap.find(rmObj.objectName) == controlData.objectMap.end()) {
          std::stringstream ss;
          ss << rmObj.objectName << " not found";
          debug_log(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str());
        }
        else {
          cGenericObject* objPtr = controlData.objectMap[rmObj.objectName];
          graphicsData.world->deleteChild(objPtr);
          controlData.objectMap.erase(rmObj.objectName);
        }
        break;
      }
      
      case RESET_WORLD:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received RESET_WORLD Message");
        unordered_map<string, cGenericObject*>::iterator objIt = controlData.objectMap.begin();
        while (objIt != controlData.objectMap.end()) {
          bool removedObj = graphicsData.world->deleteChild(objIt->second);
          objIt++;
        }
        unordered_map<string, cGenericEffect*>::iterator effIt = controlData.worldEffects.begin();
        while (effIt != controlData.worldEffects.end()) {
          bool removedEffect = graphicsData.world->removeEffect(effIt->second);
          effIt++;
        }
        controlData.objectMap.clear();
        controlData.objectEffects.clear();
        controlData.worldEffects.clear();
        break;
      }

      case CST_CREATE:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CST_CREATE Message");
        M_CST_CREATE cstObj;
        memcpy(&cstObj, packet, sizeof(cstObj));
        cCST* cst = new cCST(graphicsData.world, cstObj.lambdaVal, 
            cstObj.forceMagnitude, cstObj.visionEnabled, cstObj.hapticEnabled);
        char* cstName = cstObj.cstName;
        controlData.objectMap[cstName] = cst;
        graphicsData.movingObjects.push_back(cst);
        graphicsData.world->addEffect(cst);
        controlData.worldEffects[cstName] = cst;
        break;
      }
      case CST_DESTRUCT:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CST_DESTRUCT Message");
        M_CST_DESTRUCT cstObj;
        memcpy(&cstObj, packet, sizeof(cstObj));
        if (controlData.objectMap.find(cstObj.cstName) == controlData.objectMap.end()) {
          std::stringstream ss;
          ss << cstObj.cstName << " not found";
          debug_log(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str());
        }
        else {
          cCST* cst = dynamic_cast<cCST*>(controlData.objectMap[cstObj.cstName]);
          cst->stopCST();
          cst->destructCST();
          remove(graphicsData.movingObjects.begin(), graphicsData.movingObjects.end(), cst);
          controlData.worldEffects.erase(cstObj.cstName);
          bool removedCST = graphicsData.world->removeEffect(cst);
        }
        break;
      }
      case CST_START:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CST_START Message");
        M_CST_START cstObj;
        memcpy(&cstObj, packet, sizeof(cstObj));
        cCST* cst = dynamic_cast<cCST*>(controlData.objectMap[cstObj.cstName]);
        hapticsData.tool->setShowEnabled(false);
        cst->startCST();
        break;
      }
      case CST_STOP:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CST_STOP Message");
        M_CST_STOP cstObj;
        memcpy(&cstObj, packet, sizeof(cstObj));
        cCST* cst = dynamic_cast<cCST*>(controlData.objectMap[cstObj.cstName]);
        cst->stopCST();
        hapticsData.tool->setShowEnabled(true);
        break;
      }
      case CST_SET_VISUAL:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CST_SET_VISUAL Message");
        M_CST_SET_VISUAL cstObj;
        memcpy(&cstObj, packet, sizeof(cstObj));
        bool visual = cstObj.visionEnabled;
        cCST* cst = dynamic_cast<cCST*>(controlData.objectMap[cstObj.cstName]);
        cst->setVisionEnabled(visual);
        break;
      }
      case CST_SET_HAPTIC:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CST_SET_HAPTIC Message");
        M_CST_SET_HAPTIC cstObj;
        memcpy(&cstObj, packet, sizeof(cstObj));
        bool haptic = cstObj.hapticEnabled;
        cCST* cst = dynamic_cast<cCST*>(controlData.objectMap[cstObj.cstName]);
        cst->setHapticEnabled(haptic);
        break;
      }
      case CST_SET_LAMBDA:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CST_SET_LAMBDA Message");
        M_CST_SET_LAMBDA cstObj;
        memcpy(&cstObj, packet, sizeof(cstObj));
        double lambda = cstObj.lambdaVal;
        cCST* cst = dynamic_cast<cCST*>(controlData.objectMap[cstObj.cstName]);
        cst->setLambda(lambda);
        break;
      }
      case CUPS_CREATE:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CUPS_CREATE Message");
        M_CUPS_CREATE createCups;
        memcpy(&createCups, packet, sizeof(createCups));
        cCups* cups = new cCups(graphicsData.world, createCups.escapeAngle, 
            createCups.pendulumLength, createCups.ballMass, createCups.cartMass);
        char* cupsName = createCups.cupsName;
        controlData.objectMap[cupsName] = cups;
        graphicsData.movingObjects.push_back(cups);
        graphicsData.world->addEffect(cups);
        controlData.worldEffects[cupsName] = cups;
        break;
      }
      case CUPS_DESTRUCT:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CUPS_DESTRUCT Message");
        M_CUPS_DESTRUCT cupsObj;
        memcpy(&cupsObj, packet, sizeof(cupsObj));
        if (controlData.objectMap.find(cupsObj.cupsName) == controlData.objectMap.end()) {
          std::stringstream ss;
          ss << cupsObj.cupsName << " not found";
          debug_log(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str());
        }
        else {
          cCups* cups = dynamic_cast<cCups*>(controlData.objectMap[cupsObj.cupsName]);
          cups->stopCups();
          cups->destructCups();
          remove(graphicsData.movingObjects.begin(), graphicsData.movingObjects.end(), cups);
          controlData.worldEffects.erase(cupsObj.cupsName);
          bool removedCups = graphicsData.world->removeEffect(cups);
        }
        break;
      }
      case CUPS_START:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CUPS_START Message");
        M_CUPS_START cupsObj;
        memcpy(&cupsObj, packet, sizeof(cupsObj));
        cCups* cups = dynamic_cast<cCups*>(controlData.objectMap[cupsObj.cupsName]);
        hapticsData.tool->setShowEnabled(false);
        cups->startCups();
        break;
      }
      case CUPS_STOP:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received CUPS_STOP Message");
        M_CUPS_STOP cupsObj;
        memcpy(&cupsObj, packet, sizeof(cupsObj));
        cCups* cups = dynamic_cast<cCups*>(controlData.objectMap[cupsObj.cupsName]);
        cups->stopCups();
        hapticsData.tool->setShowEnabled(true);
        break;
      }
      case HAPTICS_SET_ENABLED:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received HAPTICS_SET_ENABLED Message");
        M_HAPTICS_SET_ENABLED hapticsEnabled;
        memcpy(&hapticsEnabled, packet, sizeof(hapticsEnabled));
        char* objectName;
        objectName = hapticsEnabled.objectName;
        if (controlData.objectMap.find(objectName) == controlData.objectMap.end()) {
          std::stringstream ss;
          ss << objectName << " not found";
          debug_log(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str());
        }
        else {
          if (hapticsEnabled.enabled == 1) {
            controlData.objectMap[objectName]->setHapticEnabled(true);
          }
          else if (hapticsEnabled.enabled == 0) {
            controlData.objectMap[objectName]->setHapticEnabled(false);
          }
        }
        break;
      }

      case HAPTICS_SET_ENABLED_WORLD:
      {
        M_HAPTICS_SET_ENABLED_WORLD worldEnabled;
        memcpy(&worldEnabled, packet, sizeof(worldEnabled));
        char* effectName;
        effectName = worldEnabled.effectName;
        cGenericEffect* fieldEffect = controlData.worldEffects[effectName];
        fieldEffect->setEnabled(worldEnabled.enabled);
        break; 
      }

      case HAPTICS_SET_STIFFNESS:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received HAPTICS_SET_STIFFNESS Message");
        M_HAPTICS_SET_STIFFNESS stiffness;
        memcpy(&stiffness, packet, sizeof(stiffness));
        char* objectName;
        objectName = stiffness.objectName;
        if (controlData.objectMap.find(objectName) == controlData.objectMap.end()) {
          std::stringstream ss;
          ss << objectName << " not found";
          debug_log(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str());
        }
        else {
          controlData.objectMap[objectName]->m_material->setStiffness(stiffness.stiffness);
        }
        break;
      }

      case HAPTICS_BOUNDING_PLANE:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received HAPTICS_BOUNDING_PLANE Message");
        M_HAPTICS_BOUNDING_PLANE bpMsg;
        memcpy(&bpMsg, packet, sizeof(bpMsg));
        double bWidth = bpMsg.bWidth;
        double bHeight = bpMsg.bHeight;
        int stiffness = hapticsData.hapticDeviceInfo.m_maxLinearStiffness;
        double toolRadius = hapticsData.toolRadius;
        cBoundingPlane* bp = new cBoundingPlane(stiffness, toolRadius, bWidth, bHeight);
        graphicsData.world->addChild(bp->getLowerBoundingPlane());
        graphicsData.world->addChild(bp->getUpperBoundingPlane());
        graphicsData.world->addChild(bp->getTopBoundingPlane());
        graphicsData.world->addChild(bp->getBottomBoundingPlane());
        graphicsData.world->addChild(bp->getLeftBoundingPlane());
        graphicsData.world->addChild(bp->getRightBoundingPlane());
        controlData.objectMap["boundingPlane"] = bp;
        break;
      }
      
      case HAPTICS_CONSTANT_FORCE_FIELD:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received HAPTICS_CONSTANT_FORCE_FIELD Message");
        M_HAPTICS_CONSTANT_FORCE_FIELD cffInfo;
        memcpy(&cffInfo, packet, sizeof(cffInfo));
        double d = cffInfo.direction;
        double m = cffInfo.magnitude;
        cConstantForceFieldEffect* cFF = new cConstantForceFieldEffect(graphicsData.world, d, m);
        graphicsData.world->addEffect(cFF);
        controlData.worldEffects[cffInfo.effectName] = cFF;
        break;
      }

      case HAPTICS_VISCOSITY_FIELD:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received HAPTICS_VISCOSITY_FIELD Message");
        M_HAPTICS_VISCOSITY_FIELD vF;
        memcpy(&vF, packet, sizeof(vF));
        cMatrix3d* B = new cMatrix3d(vF.viscosityMatrix[0], vF.viscosityMatrix[1], vF.viscosityMatrix[2],
                                     vF.viscosityMatrix[3], vF.viscosityMatrix[4], vF.viscosityMatrix[5],
                                     vF.viscosityMatrix[6], vF.viscosityMatrix[7], vF.viscosityMatrix[8]);
        cViscosityEffect* vFF = new cViscosityEffect(graphicsData.world, B);
        graphicsData.world->addEffect(vFF);
        controlData.worldEffects[vF.effectName] = vFF;
        break;
      }
      
      case HAPTICS_FREEZE_EFFECT:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received HAPTICS_FREEZE_EFFECT Message");
        M_HAPTICS_FREEZE_EFFECT freeze;
        memcpy(&freeze, packet, sizeof(freeze));
        double workspaceScaleFactor = hapticsData.tool->getWorkspaceScaleFactor();
        double maxStiffness = 1.5*hapticsData.hapticDeviceInfo.m_maxLinearStiffness/workspaceScaleFactor;
        cVector3d currentPos = hapticsData.tool->getDeviceGlobalPos();
        cFreezeEffect* freezeEff = new cFreezeEffect(graphicsData.world, maxStiffness, currentPos);
        graphicsData.world->addEffect(freezeEff);
        controlData.worldEffects[freeze.effectName] = freezeEff;
        break;  
      }

      case HAPTICS_REMOVE_WORLD_EFFECT:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received HAPTICS_REMOVE_FIELD_EFFECT Message");
        M_HAPTICS_REMOVE_WORLD_EFFECT rmField;
        memcpy(&rmField, packet, sizeof(rmField));
        cGenericEffect* fieldEffect = controlData.worldEffects[rmField.effectName];
        graphicsData.world->removeEffect(fieldEffect);
        controlData.worldEffects.erase(rmField.effectName);
        break;
      }

      case GRAPHICS_SET_ENABLED:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_SET_ENABLED Message");
        M_GRAPHICS_SET_ENABLED graphicsEnabled;
        memcpy(&graphicsEnabled, packet, sizeof(graphicsEnabled));
        char* objectName;
        objectName = graphicsEnabled.objectName;
        int enabled = graphicsEnabled.enabled;
        if (controlData.objectMap.find(objectName) == controlData.objectMap.end()) {
          std::stringstream ss;
          ss << objectName << " not found";
          debug_log(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str());
        }
        else {
          if (graphicsEnabled.enabled == 1) {
            controlData.objectMap[objectName]->setShowEnabled(true);
          }
          else if (graphicsEnabled.enabled == 0) {
            controlData.objectMap[objectName]->setShowEnabled(false);
          }
        }
        break;
      }
      
      case GRAPHICS_CHANGE_BG_COLOR:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_CHANGE_BG_COLOR Message");
        M_GRAPHICS_CHANGE_BG_COLOR bgColor;
        memcpy(&bgColor, packet, sizeof(bgColor));
        float red = bgColor.color[0]/250.0;
        float green = bgColor.color[1]/250.0;
        float blue = bgColor.color[2]/250.0;
        graphicsData.world->setBackgroundColor(red, green, blue);
        break;
      }
      
      case GRAPHICS_PIPE:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_PIPE Message");
        M_GRAPHICS_PIPE pipe;
        memcpy(&pipe, packet, sizeof(pipe));
        cVector3d* position = new cVector3d(pipe.position[0], pipe.position[1], pipe.position[2]);
        cMatrix3d* rotation = new cMatrix3d(pipe.rotation[0], pipe.rotation[1], pipe.rotation[2],
                                            pipe.rotation[3], pipe.rotation[4], pipe.rotation[5],
                                            pipe.rotation[6], pipe.rotation[7], pipe.rotation[8]);
        cColorf* color = new cColorf(pipe.color[0], pipe.color[1], pipe.color[2], pipe.color[3]);
        cPipe* myPipe = new cPipe(pipe.height, pipe.innerRadius, pipe.outerRadius, pipe.numSides, 
                                  pipe.numHeightSegments, position, rotation, color);
        controlData.objectMap[pipe.objectName] = myPipe->getPipeObj();
        graphicsData.world->addChild(myPipe->getPipeObj());
        break;
      }

      case GRAPHICS_ARROW:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_ARROW Message");
        M_GRAPHICS_ARROW arrow;
        memcpy(&arrow, packet, sizeof(arrow));
        cVector3d* direction = new cVector3d(arrow.direction[0], arrow.direction[1], arrow.direction[2]);
        cVector3d* position = new cVector3d(arrow.position[0], arrow.position[1], arrow.position[2]);
        cColorf* color = new cColorf(arrow.color[0], arrow.color[1], arrow.color[2], arrow.color[3]);
        cArrow* myArrow = new cArrow(arrow.aLength, arrow.shaftRadius, arrow.lengthTip, arrow.radiusTip,
                                      arrow.bidirectional, arrow.numSides, direction, position, color);
        controlData.objectMap[arrow.objectName] = myArrow->getArrowObj();
        graphicsData.world->addChild(myArrow->getArrowObj());
        break;
      }
      
      case GRAPHICS_CHANGE_OBJECT_COLOR:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_CHANGE_OBJECT_COLOR Message");
        M_GRAPHICS_CHANGE_OBJECT_COLOR color;
        memcpy(&color, packet, sizeof(color));
        cGenericObject* obj = controlData.objectMap[color.objectName];
        obj->m_material->setColorf(color.color[0], color.color[1], color.color[2], color.color[3]);
        break;
      }
      case GRAPHICS_MOVING_DOTS:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_MOVING_DOTS Message");
        cMultiPoint* test = new cMultiPoint();
        M_GRAPHICS_MOVING_DOTS dots;
        memcpy(&dots, packet, sizeof(dots));
        char* objectName;
        objectName = dots.objectName;
        cMovingDots* md = new cMovingDots(dots.numDots, dots.coherence, dots.direction, dots.magnitude);
        controlData.objectMap[objectName] = md;
        graphicsData.movingObjects.push_back(md);
        graphicsData.world->addChild(md->getMovingPoints());
        graphicsData.world->addChild(md->getRandomPoints());
        break;
      }
      case GRAPHICS_SHAPE_BOX:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_SHAPE_BOX Message");
        M_GRAPHICS_SHAPE_BOX box;
        memcpy(&box, packet, sizeof(box));
        cShapeBox* boxObj = new cShapeBox(box.sizeX, box.sizeY, box.sizeZ);
        boxObj->setLocalPos(box.localPosition[0], box.localPosition[1], box.localPosition[2]);
        boxObj->m_material->setColorf(box.color[0], box.color[1], box.color[2], box.color[3]);
        controlData.objectMap[box.objectName] = boxObj;
        graphicsData.world->addChild(boxObj);
        break;
      }
      case GRAPHICS_SHAPE_SPHERE: 
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_SHAPE_SPHERE Message");
        M_GRAPHICS_SHAPE_SPHERE sphere;
        memcpy(&sphere, packet, sizeof(sphere));
        cShapeSphere* sphereObj = new cShapeSphere(sphere.radius);
        sphereObj->setLocalPos(sphere.localPosition[0], sphere.localPosition[1], sphere.localPosition[2]);
        sphereObj->m_material->setColorf(sphere.color[0], sphere.color[1], sphere.color[2], sphere.color[3]);
        controlData.objectMap[sphere.objectName] = sphereObj;
        graphicsData.world->addChild(sphereObj);
        break;
      }
      case GRAPHICS_SHAPE_TORUS:
      {
        debug_log(__FILE__, __LINE__, __FUNCTION__, "Received GRAPHICS_SHAPE_TORUS Message");
        M_GRAPHICS_SHAPE_TORUS torus;
        memcpy(&torus, packet, sizeof(torus));
        cShapeTorus* torusObj = new cShapeTorus(torus.innerRadius, torus.outerRadius);
        graphicsData.world->addChild(torusObj);
        torusObj->setLocalPos(0.0, 0.0, 0.0);
        torusObj->m_material->setStiffness(1.0);
        torusObj->m_material->setColorf(255.0, 255.0, 255.0, 1.0);
        cEffectSurface* torusEffect = new cEffectSurface(torusObj);
        torusObj->addEffect(torusEffect);
        controlData.objectMap[torus.objectName] = torusObj;
        break; 
      }
    }
  } catch (const std::exception& e) {
    debug_log(__FILE__, __LINE__, __FUNCTION__, std::string("Exception in parsePacket: " + std::string(e.what())).c_str());
    print_stack_trace();
    throw;
  }
}
