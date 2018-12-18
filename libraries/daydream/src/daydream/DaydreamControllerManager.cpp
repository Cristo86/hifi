//
//  ViveControllerManager.cpp
//  input-plugins/src/input-plugins
//
//  Created by Sam Gondelman on 6/29/15.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "DaydreamControllerManager.h"

#include <PerfStat.h>
#include <PathUtils.h>
#include <gpu/Batch.h>
#include <gpu/Context.h>
#include <DeferredLightingEffect.h>
#include <NumericalConstants.h>
#include <ui-plugins/PluginContainer.h>
#include <UserActivityLogger.h>
#include <OffscreenUi.h>


#include <controllers/UserInputMapper.h>

#include <controllers/StandardControls.h>


#include <glm/gtx/matrix_decompose.hpp>

const QString DaydreamControllerManager::NAME = "Daydream";
static const char* MENU_PATH = "Avatar" ">" "Daydream Controllers";

bool DaydreamControllerManager::isSupported() const {
  return true;
}

bool DaydreamControllerManager::activate() {
    _container->addMenu(MENU_PATH);
    InputPlugin::activate();

    // register with UserInputMapper
    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();

    GvrState::init(__gvr_context);
    GvrState *gvrState = GvrState::getInstance();

    if (gvrState->_gvr_api && !gvrState->_controller_api) {
        gvrState->_controller_api.reset(new gvr::ControllerApi);
        gvrState->_controller_api->Init(gvr::ControllerApi::DefaultOptions(), gvrState->_gvr_context);
    }

    gvrState->_controller_api->Resume();
    // TODO: retrieve state from daydream API
    unsigned int controllerConnected = true;


    _controller = std::make_shared<DaydreamControllerDevice>(*this);
    userInputMapper->registerDevice(_controller);
    _registeredWithInputMapper = true;
    return true;
}

void DaydreamControllerManager::deactivate() {

    // unregister with UserInputMapper
    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
    if (_controller) {
        userInputMapper->removeDevice(_controller->getDeviceID());
        _registeredWithInputMapper = false;
    }

}

void DaydreamControllerManager::pluginUpdate(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) {

    // TODO: check state and deactivate if needed

    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();

    // because update mutates the internal state we need to lock
    userInputMapper->withLock([&, this]() {
        _controller->update(deltaTime, inputCalibrationData);
    });

    /*if (!_registeredWithInputMapper && _inputDevice->_trackedControllers > 0) {
        userInputMapper->registerDevice(_inputDevice);
        _registeredWithInputMapper = true;
    }*/

}

void DaydreamControllerManager::pluginFocusOutEvent() {
  qDebug() << "pluginFocusOutEvent pluginFocusOutEvent pluginFocusOutEvent";
    if (_controller) {
        _controller->focusOutEvent();
    }
}


// An enum for buttons which do not exist in the StandardControls enum
enum DaydreamButtonChannel {
    APP_BUTTON = controller::StandardButtonChannel::NUM_STANDARD_BUTTONS,
    CLICK_BUTTON
};

QString DaydreamControllerManager::DaydreamControllerDevice::nvlButn(int w) {
    if (_buttonPressedMap.find(w) != _buttonPressedMap.end()) {
        return "YES";
    } else {
        return "null";
    }
}

QString DaydreamControllerManager::DaydreamControllerDevice::nvlAxis(int w) {
    if (_axisStateMap.find(w) != _axisStateMap.end()) {
        return QString::number(_axisStateMap[w]);
    } else {
        return "null";
    }
}

void DaydreamControllerManager::DaydreamControllerDevice::update(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) {
    _poseStateMap.clear();
    _buttonPressedMap.clear();

    PerformanceTimer perfTimer("DaydreamControllerManager::update");

    GvrState *gvrState = GvrState::getInstance();

    _isLeftHanded = gvrState->_gvr_api->GetUserPrefs().GetControllerHandedness() == gvr::kControllerLeftHanded;

    // Read current controller state. This must be done once per frame
    gvrState->_controller_state.Update(*gvrState->_controller_api);

    int32_t currentApiStatus = gvrState->_controller_state.GetApiStatus();
    int32_t currentConnectionState = gvrState->_controller_state.GetConnectionState();

    // Print new API status and connection state, if they changed.
    if (currentApiStatus != gvrState->_last_controller_api_status ||
          currentConnectionState != gvrState->_last_controller_connection_state) {
            qDebug() << "[DAY] controller API status " <<
                gvr_controller_api_status_to_string(currentApiStatus) << ", connection state: " <<
                gvr_controller_connection_state_to_string(currentConnectionState);

            gvrState->_last_controller_api_status = currentApiStatus;
            gvrState->_last_controller_connection_state = currentConnectionState;
    }

    // Will update this 'button' status in every update
    if (currentConnectionState == GVR_CONTROLLER_CONNECTED) {
        _buttonPressedMap.insert(controller::GUIDE);
    }

    handleController(gvrState, deltaTime, inputCalibrationData);

    handleHeadPose(gvrState, deltaTime, inputCalibrationData);
}

void DaydreamControllerManager::DaydreamControllerDevice::handleController(GvrState *gvrState, float deltaTime, const controller::InputCalibrationData& inputCalibrationData) {

      gvr::ControllerQuat orientation = gvrState->_controller_state.GetOrientation();
      if (gvrState->_controller_state.GetRecentered()) {
        //qDebug("[DAYDREAM-CONTROLLER] just recentered! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
        _justRecentered = true;
    }
      //qDebug() << "[DAYDREAM-CONTROLLER]: gvr::ControllerQuat orientation: " << orientation.qx << "," << orientation.qy << "," << orientation.qz << "," << orientation.qw;
      handlePoseEvent(deltaTime, inputCalibrationData, orientation);

      bool trackpadClicked = false;

      if (gvrState->_last_controller_api_status == gvr_controller_api_status::GVR_CONTROLLER_API_OK &&
          gvrState->_last_controller_connection_state == gvr_controller_connection_state::GVR_CONTROLLER_CONNECTED) {
        for (int k = gvr_controller_button::GVR_CONTROLLER_BUTTON_NONE; k < gvr_controller_button::GVR_CONTROLLER_BUTTON_COUNT ;k++) {
          bool justPressed = gvrState->_controller_state.GetButtonDown(static_cast<gvr::ControllerButton>(k)); // Returns whether the given button was just justPressed (transient).
          bool currentlyPressed = gvrState->_controller_state.GetButtonState(static_cast<gvr::ControllerButton>(k)); // Returns whether the given button is currently justPressed.
          bool justReleased = gvrState->_controller_state.GetButtonUp(static_cast<gvr::ControllerButton>(k)); // Returns whether the given button was just released (transient).
          //if ((justPressed || justReleased || currentlyPressed) || rand() % 100 > 98)
              //qDebug() << "[DAYDREAM-CONTROLLER]: call handleButtonEvent(deltaTime: " << deltaTime << ", k: " << k <<
              //        ", justPressed: " << justPressed << ", justReleased: " << justReleased << ",  currentlyPressed: " <<  currentlyPressed;

          trackpadClicked = trackpadClicked || ( gvr_controller_button::GVR_CONTROLLER_BUTTON_CLICK && (justPressed || justReleased || currentlyPressed) );
          handleButtonEvent(deltaTime, k, justPressed, justReleased, currentlyPressed);

        }
      }

    if (trackpadClicked) {
        bool isTouching = gvrState->_controller_state.IsTouching();
        gvr_vec2f touchPos = gvrState->_controller_state.GetTouchPos();
        if (_isLeftHanded) {
            handleAxisEvent(deltaTime, isTouching, touchPos, controller::LX, controller::LY);
            partitionTouchpad(controller::LS, controller::LX, controller::LY, controller::LT_CLICK, controller::LS_X, controller::LS_Y, controller::LT);
        } else {
            handleAxisEvent(deltaTime, isTouching, touchPos, controller::RX, controller::RY);
            partitionTouchpad(controller::RS, controller::RX, controller::RY, controller::RT_CLICK, controller::RS_X, controller::RS_Y, controller::RT);
        }
    } else {
        _rtClickStarted = false;
    }

}

void DaydreamControllerManager::DaydreamControllerDevice::handleHeadPose(GvrState *gvrState, 
                                                                        float deltaTime, 
                                                                        const controller::InputCalibrationData& inputCalibrationData) {
    gvr::ClockTimePoint pred_time = gvr::GvrApi::GetTimePointNow();
    pred_time.monotonic_system_time_nanos += 50000000; // 50ms

    gvr::Mat4f head_view = gvrState->_gvr_api->GetHeadSpaceFromStartSpaceTransform(pred_time);
    glm::mat4 glmHeadView = glm::inverse(GvrMat4fToGlmMat4(head_view));

    //perform a 180 flip to make the HMD face the +z instead of -z, beacuse the head faces +z
    glm::mat4 matYFlip = glmHeadView * Matrices::Y_180;

    glm::vec3 headTranslation = extractTranslation(matYFlip);
    glm::quat headRotation = glmExtractRotation(matYFlip);
    glm::vec3 angles = glm::eulerAngles(headRotation);
    controller::Pose pose(headTranslation, headRotation);

    glm::mat4 worldToAvatar = glm::inverse(inputCalibrationData.avatarMat);
    glm::mat4 sensorToAvatar = worldToAvatar * inputCalibrationData.sensorToWorldMat;
    
    glm::mat4 defaultHeadOffset = glm::inverse(inputCalibrationData.defaultCenterEyeMat) *
                                                        inputCalibrationData.defaultHeadMat;
    pose.valid = true;
    controller::Pose result = pose.postTransform(defaultHeadOffset).transform(sensorToAvatar);
    _poseStateMap[controller::HEAD] = result;
}

void DaydreamControllerManager::DaydreamControllerDevice::handlePoseEvent(float deltaTime, const controller::InputCalibrationData& inputCalibrationData, gvr::ControllerQuat gvrOrientation) {
    glm::quat orientation = toGlm(gvrOrientation);
    // transform into avatar frame
    glm::mat4 controllerToAvatar = glm::inverse(inputCalibrationData.avatarMat) * inputCalibrationData.sensorToWorldMat;

    if (_justRecentered) {
        _adjustmentRotationMatrix = genNegativeAdjustmentRotationAroundYMatrix(controllerToAvatar);

        _justRecentered = false;
    }

    auto pose = daydreamControllerPoseToHandPose(_isLeftHanded, orientation);
    _poseStateMap[_isLeftHanded ? controller::LEFT_HAND : controller::RIGHT_HAND] = pose.transform(_adjustmentRotationMatrix).transform(controllerToAvatar);
}

// These functions do translation from the Steam IDs to the standard controller IDs
void DaydreamControllerManager::DaydreamControllerDevice::handleButtonEvent(float deltaTime, uint32_t button, bool justPressed, bool justReleased, bool currentlyPressed) {

    using namespace controller;
    // gvr_controller_button::GVR_CONTROLLER_BUTTON_CLICK = 1,  ///< Touchpad Click.
    // gvr_controller_button::GVR_CONTROLLER_BUTTON_HOME = 2,
    // gvr_controller_button::GVR_CONTROLLER_BUTTON_APP = 3,
    // gvr_controller_button::GVR_CONTROLLER_BUTTON_VOLUME_UP = 4,
    // gvr_controller_button::GVR_CONTROLLER_BUTTON_VOLUME_DOWN = 5,
    if (justPressed) {
        if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_CLICK) {
            //qDebug() << "[DAYDREAM-CONTROLLER]: RT_CLICK inserted";
            //_buttonPressedMap.insert(RT_CLICK);
            //_buttonPressedMap.insert(RT);
            _buttonPressedMap.insert(_isLeftHanded ? LS : RS);
            _buttonPressedMap.insert(_isLeftHanded ? LS_TOUCH : RS_TOUCH);
        } else if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_APP) {
            _buttonPressedMap.insert(_isLeftHanded ? LEFT_PRIMARY_THUMB : RIGHT_PRIMARY_THUMB);
          //_buttonPressedMap.insert(LS);
        } else if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_HOME) {
            // TODO: we must not use this home button, check the desired mapping
            //_axisStateMap[LEFT_GRIP] = 1.0f;
        }
    } else {
        if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_HOME) {
            //_axisStateMap[LEFT_GRIP] = 0.0f;
        }
    }

    if (currentlyPressed) {
        if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_CLICK) {
            //qDebug() << "[DAYDREAM-CONTROLLER]: RT_CLICK inserted (continues)";
            //_buttonPressedMap.insert(RT_CLICK);
            //_buttonPressedMap.insert(RT);
            _buttonPressedMap.insert(_isLeftHanded ? LS : RS);
            _buttonPressedMap.insert(_isLeftHanded ? LS_TOUCH : RS_TOUCH);
        } else if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_APP) {
            _buttonPressedMap.insert(_isLeftHanded ? LEFT_PRIMARY_THUMB : RIGHT_PRIMARY_THUMB);
          //_buttonPressedMap.insert(LS);
        }
    }

    if (justReleased) {
          // TODO: this is also duplicated. Perhaps we discard some feature later
         if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_CLICK) {
          // TODO: this is also duplicated. Perhaps we discard some feature later
            //_buttonPressedMap.insert(LS_TOUCH);
            //qDebug() << "[DAYDREAM-CONTROLLER]: RT_CLICK inserted";
            //_buttonPressedMap.insert(RT_CLICK);
        } else if (button == gvr_controller_button::GVR_CONTROLLER_BUTTON_APP) {
            _buttonPressedMap.insert(_isLeftHanded ? LEFT_PRIMARY_THUMB : RIGHT_PRIMARY_THUMB);
          //_buttonPressedMap.insert(LS);
        }
    }
}

// These functions do translation from the Steam IDs to the standard controller IDs
void DaydreamControllerManager::DaydreamControllerDevice::handleAxisEvent(float deltaTime, bool isTouching, gvr_vec2f touchPos, int xAxis, int yAxis) {
    using namespace controller;
    if (isTouching) {
        glm::vec2 stick(2*(touchPos.x-0.5f), 2*(touchPos.y-0.5f));
        stick = _filteredRightStick.process(deltaTime, stick);
        //qDebug() << "[DAYDREAM-CONTROLLER]: Touching x:" << stick.x << " y:" << stick.y;
        _axisStateMap[xAxis] = stick.x;
        _axisStateMap[yAxis] = stick.y;
    } else {
      _axisStateMap.clear();
    }
}

void DaydreamControllerManager::DaydreamControllerDevice::partitionTouchpad(int sButton, int xAxis, int yAxis, int centerPseudoButton, int xPseudoButton, int yPseudoButton, int triggerButton) {
    // Populate the L/RS_CENTER/OUTER pseudo buttons, corresponding to a partition of the L/RS space based on the X/Y values.
    const float CENTER_DEADBAND = 0.6f;
    const float DIAGONAL_DIVIDE_IN_RADIANS = PI / 4.0f;
    if (_buttonPressedMap.find(sButton) != _buttonPressedMap.end()) {
        float absX = fabs(_axisStateMap[xAxis]);
        float absY = fabs(_axisStateMap[yAxis]);
        glm::vec2 cartesianQuadrantI(absX, absY);
        float angle = glm::atan(cartesianQuadrantI.y / cartesianQuadrantI.x);
        float radius = glm::length(cartesianQuadrantI);
        bool isCenter = radius < CENTER_DEADBAND;
        int toInsert = isCenter ? centerPseudoButton : ((angle < DIAGONAL_DIVIDE_IN_RADIANS) ? xPseudoButton :yPseudoButton);
        //_buttonPressedMap.insert(isCenter ? centerPseudoButton : ((angle < DIAGONAL_DIVIDE_IN_RADIANS) ? xPseudoButton :yPseudoButton));
        if (!isCenter) {
            _buttonPressedMap.insert(toInsert);
        }
        if (isCenter || _rtClickStarted) {
            // RT_CLICK
            _buttonPressedMap.insert(toInsert);
            // extra RT
            _buttonPressedMap.insert(triggerButton);
            _axisStateMap[triggerButton] = 1;
            // save so we keep sending the click message
            _rtClickStarted = true;
        }
    }
}

void DaydreamControllerManager::DaydreamControllerDevice::focusOutEvent() {
  qDebug() << "DaydreamControllerDevice::focusOutEvent DaydreamControllerDevice::focusOutEvent DaydreamControllerDevice::focusOutEvent ";
    _axisStateMap.clear();
    _buttonPressedMap.clear();
};

glm::mat4 DaydreamControllerManager::DaydreamControllerDevice::genNegativeAdjustmentRotationAroundYMatrix(glm::mat4 original) {
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(original, scale, rotation, translation, skew, perspective);

    glm::vec3 eulerAngles = glm::eulerAngles(rotation);

    glm::quat negativeYRotation = glm::quat(glm::vec3(eulerAngles.x, -eulerAngles.y, eulerAngles.z));

    controller::Pose pose;
    pose.translation = glm::vec3(0.0f);
    pose.rotation = negativeYRotation;
    pose.angularVelocity = glm::vec3(0.0f);
    pose.velocity = glm::vec3(0.0f);
    pose.valid = true;

    return pose.getMatrix();
}

controller::Input::NamedVector DaydreamControllerManager::DaydreamControllerDevice::getAvailableInputs() const {
    using namespace controller;
    QVector<Input::NamedPair> availableInputs{

        // touch pad press
        makePair(RS, "RS"),
        makePair(LS, "LS"),

        makePair(RX, "RX"),
        makePair(RY, "RY"),
        makePair(LX, "LX"),
        makePair(LY, "LY"),
        
        makePair(RS_TOUCH, "RSTouch"),
        makePair(LS_TOUCH, "LSTouch"),

        // triggers
        makePair(RT, "RT"),
        makePair(LT, "LT"),

        // Trigger clicks
        makePair(RT_CLICK, "RTClick"),
        makePair(LT_CLICK, "LTClick"),

        // Differentiate where we are in the touch pad click
        makePair(RS_CENTER, "RSCenter"),
        makePair(RS_X, "RSX"),
        makePair(RS_Y, "RSY"),
        makePair(LS_CENTER, "LSCenter"),
        makePair(LS_X, "LSX"),
        makePair(LS_Y, "LSY"),

        makePair(RIGHT_PRIMARY_THUMB, "RightPrimaryThumb"),
        makePair(LEFT_PRIMARY_THUMB, "LeftPrimaryThumb"),

        // 3d location of controller
        makePair(RIGHT_HAND, "RightHand"),
        makePair(LEFT_HAND, "LeftHand"),

        makePair(HEAD, "Head"),

        // This will tell us if the controller is connected
        makePair(GUIDE, "Guide"),
    };

    return availableInputs;
}



QString DaydreamControllerManager::DaydreamControllerDevice::getDefaultMappingConfig() const {
    static const QString MAPPING_JSON = PathUtils::resourcesPath() + "/controllers/daydream.json";
    return MAPPING_JSON;
}



