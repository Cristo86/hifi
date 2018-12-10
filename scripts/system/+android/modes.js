"use strict";
//
//  modes.js
//  scripts/system/
//
//  Created by Gabriel Calero & Cristian Duarte on Jan 23, 2018
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
(function() { // BEGIN LOCAL_SCOPE

var modeButton;
var currentMode;
var barQml;

var SETTING_CURRENT_MODE_KEY = 'Android/Mode';
var MODE_VR = "VR", MODE_RADAR = "RADAR", MODE_MY_VIEW = "MY VIEW";
var DEFAULT_MODE = MODE_MY_VIEW;
var nextMode = {};
nextMode[MODE_MY_VIEW]=MODE_VR;
nextMode[MODE_VR]=MODE_MY_VIEW;
var modeLabel = {};
//modeLabel[MODE_RADAR]="TOP VIEW";
modeLabel[MODE_MY_VIEW]="MY VIEW";
modeLabel[MODE_VR]="VR";

var logEnabled = false;
var radar = Script.require('./radar.js');
var uniqueColor = Script.require('./uniqueColor.js');
var displayNames = Script.require('./displayNames.js');
var clickWeb = Script.require('./clickWeb.js');

function printd(str) {
    if (logEnabled) {       
        print("[modes.js] " + str);
    }
}

function init() {
    radar.setUniqueColor(uniqueColor);
    radar.init();
    
    barQml = new QmlFragment({
        qml: "hifi/modesbar.qml"
    });
    modeButton = barQml.addButton({
        icon: "icons/myview-a.svg",
        activeBgOpacity: 0.0,
        hoverBgOpacity: 0.0,
        activeHoverBgOpacity: 0.0,
        text: "MODE",
        height:240,
        bottomMargin: 16,
        textSize: 38,
        fontFamily: "Raleway",
        fontBold: true
    });
    
    
    if (!HMD.active) {
        connectButton();        
        switchToMode(nextMode[MODE_VR]); // adjust this if radar mode comes back
    } else {
        switchToMode(MODE_VR);
    }

    HMD.displayModeChanged.connect(function (isHMDMode) {
        if (isHMDMode && HMD.active) {
            shutdown();
            switchToMode(MODE_VR);
        } else {
            printd("exit VR?");
            connectButton();
            switchToMode(nextMode[MODE_VR]);
        }
    });

    var deviceID = Controller.findDevice("Daydream");
    print("[DAY] Device ID of Daydream = " + deviceID);

    var daydreamControllerMonitorListener = new DaydreamControllerMonitorListener();
    var daydreamControllerMonitor = new DaydreamControllerMonitor(daydreamControllerMonitorListener);
    daydreamControllerMonitor.startMonitoring();
}

function DaydreamControllerMonitorListener() {

    var daydreamControllerQml = new QmlFragment({
        qml: "hifi/DaydreamControllerWakeUp.qml"
    });

    daydreamControllerQml.setVisible(HMD.active);

    HMD.displayModeChanged.connect(function (isHMDMode) {
        if (daydreamControllerQml) {
            daydreamControllerQml.setVisible(isHMDMode);
        }
    });

    this.onControllerConnected = function () {
        if (daydreamControllerQml) {
            daydreamControllerQml.close();
            daydreamControllerQml = null;
        }
    }

    this.onControllerDisconnected = function() {
        if (!daydreamControllerQml) {
            daydreamControllerQml = new QmlFragment({
                qml: "hifi/DaydreamControllerWakeUp.qml"
            });
        } else {
            // be sure to be showing
            daydreamControllerQml.setVisible(true);
        }
    }

}

// Polls for the connection value (mapping implementation is not working)
function DaydreamControllerMonitor(monitorListener) {
    var _this = this;

    var FAST_DAYDREAM_CONTROLLER_STATUS_POLLING = 500;
    var SLOW_DAYDREAM_CONTROLLER_STATUS_POLLING = 2000;

    var listener = monitorListener;
    var pollingTimer;
    var connectionPrevValue = -1;
    var started = false;

    this.pollDaydreamControllerConnectionStatus = function () {
        if (!started) {
            return;
        }

        var value = Controller.getValue(Controller.Hardware.Daydream.Guide); // Guide is our communicated status
        if (value != connectionPrevValue) {

            if (connectionPrevValue == 0 && value == 1) {
                listener.onControllerConnected();
                // reduce frequency
                Script.clearInterval(pollingTimer);
                pollingTimer = Script.setInterval(_this.pollDaydreamControllerConnectionStatus, SLOW_DAYDREAM_CONTROLLER_STATUS_POLLING);
            } else if (connectionPrevValue == 1 && value == 0) {
                listener.onControllerDisconnected();
                // higher frequency
                Script.clearInterval(pollingTimer);
                pollingTimer = Script.setInterval(_this.pollDaydreamControllerConnectionStatus, FAST_DAYDREAM_CONTROLLER_STATUS_POLLING);
            } // else connectionPrevValue == -1 it's only in the first time

            connectionPrevValue = value;
        }
    }

    this.startMonitoring = function () {
        started = true;
        pollingTimer = Script.setInterval(_this.pollDaydreamControllerConnectionStatus, FAST_DAYDREAM_CONTROLLER_STATUS_POLLING);
    }

    this.stopMonitoring = function() {
        started = false;
        connectionPrevValue = -1;

        Script.clearInterval(pollingTimer);
    }
}

// Mapping implementation not working now, that's why we use that button polling
function DaydreamControllerMonitorWithMapping(monitorListener) {

    var listener = monitorListener;
    var daydreamConnectedMapping;
    var connectionPrevValue = -1;

    this.onDaydreamControllerConnectionStatus = function () {
        if (value != connectionPrevValue) {
            if (connectionPrevValue == 0 && value == 1) {
                listener.onControllerConnected();
            } else if (connectionPrevValue == 1 && value == 0) {
                listener.onControllerDisconnected();
            } // else connectionPrevValue == -1 it's only in the first time

            connectionPrevValue = value;
        }
    }

    this.startMonitoring = function () {
        var mappingName = 'Hifi-Daydream-' + Math.random();
        daydreamConnectedMapping = Controller.newMapping(mappingName);
        daydreamConnectedMapping.from(Controller.Hardware.Daydream.Guide).peek().to(this.onDaydreamControllerConnectionStatus);
        Controller.enableMapping(daydreamConnectedMapping);
    }

    this.stopMonitoring = function() {
        if (daydreamConnectedMapping) {
            Controller.disableMapping(daydreamConnectedMapping);
        }
    }
}

function connectButton() {
    if (modeButton) {
        modeButton.entered.connect(modeButtonPressed);
        modeButton.clicked.connect(modeButtonClicked);
    }
}

function shutdown() {
    modeButton.entered.disconnect(modeButtonPressed);
    modeButton.clicked.disconnect(modeButtonClicked);
}

function modeButtonPressed() {
   Controller.triggerHapticPulseOnDevice(Controller.findDevice("TouchscreenVirtualPad"), 0.1, 40.0, 0);
}

function modeButtonClicked() {
    if (MODE_VR == nextMode[currentMode]) {
        // launch animation to switch to Daydream
        Window.openAndroidActivity("VR", false);
    } else {
        switchToMode(nextMode[currentMode]);
    }
}

function saveCurrentModeSetting(mode) {
    Settings.setValue(SETTING_CURRENT_MODE_KEY, mode);
}

function getCurrentModeSetting() {
    return Settings.getValue(SETTING_CURRENT_MODE_KEY, DEFAULT_MODE);
}

function switchToMode(newMode) {
    // before leaving radar mode
    if (currentMode == MODE_RADAR) {
        radar.endRadarMode();
    }
    currentMode = newMode;
    modeButton.text = modeLabel[currentMode];

    saveCurrentModeSetting(currentMode);

    if (currentMode == MODE_RADAR) {
        radar.startRadarMode();
        displayNames.ending();
        clickWeb.ending();
        barQml.setVisible(true);
        Controller.setVPadEnabled(false);
    } else  if (currentMode == MODE_MY_VIEW) {
        Menu.setIsOptionChecked("Third Person", true);
        // nothing to do yet
        displayNames.init();
        clickWeb.init();
        barQml.setVisible(true);
        Controller.setVPadEnabled(true);
    } else if (currentMode == MODE_VR) {
        Menu.setIsOptionChecked("Daydream", true);
        barQml.setVisible(false);
        Controller.setVPadEnabled(false);
    } else {
        printd("Unknown view mode " + currentMode);
    }
    
}

function sendToQml(o) {
    if(barQml) {
      barQml.sendToQml(o);  
    }
}

Script.scriptEnding.connect(function () {
    shutdown();
});

init();

}()); // END LOCAL_SCOPE