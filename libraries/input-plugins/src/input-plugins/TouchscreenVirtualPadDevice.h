//
//  TouchscreenVirtualPadDevice.h
//  input-plugins/src/input-plugins
//
//  Created by Triplelexx on 1/31/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_TouchscreenVirtualPadDevice_h
#define hifi_TouchscreenVirtualPadDevice_h

#include <controllers/InputDevice.h>
#include "InputPlugin.h"
#include <QtGui/qtouchdevice.h>

class QTouchEvent;
class QGestureEvent;

const float STICK_RADIUS_INCHES = .3f;

extern bool __touchVPadLeft;

class TouchscreenVirtualPadDevice : public InputPlugin {
    Q_OBJECT
public:

    /*enum TouchAxisChannel {
        TOUCH_AXIS_X_POS = 0,
        TOUCH_AXIS_X_NEG,
        TOUCH_AXIS_Y_POS,
        TOUCH_AXIS_Y_NEG,
    };

	enum TouchGestureAxisChannel {
        TOUCH_GESTURE_PINCH_POS = TOUCH_AXIS_Y_NEG + 1,
        TOUCH_GESTURE_PINCH_NEG,
    };*/

    // Plugin functions
    virtual bool isSupported() const override;
    virtual const QString getName() const override { return NAME; }

    bool isHandController() const override { return false; }

    virtual void pluginFocusOutEvent() override { _inputDevice->focusOutEvent(); }
    virtual void pluginUpdate(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) override;

    void touchBeginEvent(const QTouchEvent* event);
    void touchEndEvent(const QTouchEvent* event);
    void touchUpdateEvent(const QTouchEvent* event);
    void touchGestureEvent(const QGestureEvent* event);

    static const char* NAME;

protected:

    class InputDevice : public controller::InputDevice {
    public:
        InputDevice() : controller::InputDevice("TouchscreenVirtualPad") {}
    private:
        // Device functions
        virtual controller::Input::NamedVector getAvailableInputs() const override;
        virtual QString getDefaultMappingConfig() const override;
        virtual void update(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) override;
        virtual void focusOutEvent() override;

        //controller::Input makeInput(TouchAxisChannel axis) const;
        //controller::Input makeInput(TouchGestureAxisChannel gesture) const;

        friend class TouchscreenVirtualPadDevice;
    };

public:
    const std::shared_ptr<InputDevice>& getInputDevice() const { return _inputDevice; }

protected:
    qreal _lastPinchScale;
    qreal _pinchScale;
    qreal _screenDPI;
    glm::vec2 _screenDPIScale;
    //glm::vec2 _firstTouchVec;
    //glm::vec2 _currentTouchVec;
    bool _validTouchLeft;
    glm::vec2 _firstTouchLeftPoint;
    glm::vec2 _currentTouchLeftPoint;
    bool _validTouchRight;
    glm::vec2 _firstTouchRightPoint;
    glm::vec2 _currentTouchRightPoint;
    int _touchPointCount;
    int _screenWidthCenter;
    std::shared_ptr<InputDevice> _inputDevice { std::make_shared<InputDevice>() };

    void touchLeftBegin(glm::vec2 touchPoint);
    void touchLeftUpdate(glm::vec2 touchPoint);
    void touchLeftEnd();
    void touchRightBegin(glm::vec2 touchPoint);
    void touchRightUpdate(glm::vec2 touchPoint);
    void touchRightEnd();
// just for debug
private:
    void debugPoints(const QTouchEvent* event, QString who);

};

bool _touchVPadLeft();
glm::vec2 _touchVPadFirstLeft();
glm::vec2 _touchVPadCurrentLeft();

#endif // hifi_TouchscreenVirtualPadDevice_h