"use strict";


(function() { // BEGIN LOCAL_SCOPE

var logEnabled = true;
function printd(str) {
    if (logEnabled)
        print("teleport.js " + str);
}

var _TARGET_MODEL_DIMENSIONS = [{ x: 1.15, y: 0.5, z: 1.15}, { x: 1.25, y: 0.6, z: 1.25}, { x: 1.45, y: 0.8, z: 1.45}];
var _iter=0;
var _this = this;
_this.updateOverlays = function () {
    printd("[DEBUG-TELEPORT] UPDATE edit overlays " + _this.targetOverlays.length);
    for (i=0;i<_this.targetOverlays.length;i++) {
        printd("[DEBUG-TELEPORT] UPDATE edit overlay " + i);
        var targetOverlay = _this.targetOverlays[i];
        _iter++;
        printd("[DEBUG-TELEPORT] Is loaded " + targetOverlay + ":" +Overlays.isLoaded(targetOverlay));
        Overlays.editOverlay(targetOverlay, { visible: true, dimensions : _TARGET_MODEL_DIMENSIONS[_iter%3]});
        printd("[DEBUG-TELEPORT] Clonning " + targetOverlay+ " -> " + Overlays.cloneOverlay(targetOverlay));


    }

};

_this.targetOverlays = new Array();
//10,0.1,-3.0
_this.createTargetOverlay = function (aPosition) {
        printd ("[DEBUG-TELEPORT] addOverlay debug");

        var TARGET_MODEL_DIMENSIONS = {
            x: 1.15,
            y: 0.5,
            z: 1.15
        };
        var TARGET_MODEL_URL = Script.resolvePath("../assets/models/teleport-destination.fbx");
        var TOO_CLOSE_MODEL_URL = Script.resolvePath("../assets/models/teleport-cancel.fbx");

        printd("[DEBUG-TELEPORT] adding model " + aPosition);
        var position = {
            x: aPosition.x,
            y: 0.25,
            z: aPosition.z
        };
        printd ("[DEBUG-TELEPORT] addOverlay debug 2");
        var rotation = {
            x: 0.0,
            y: 0.0,
            z: 0.0,
            w: 1.0
        }

        var euler = Quat.safeEulerAngles(rotation);
        printd ("[DEBUG-TELEPORT] addOverlay debug 1");
        var towardUs = Quat.fromPitchYawRollDegrees(0, euler.y, 0);

        var targetOverlayProps = {
            //url: "file:///data/user/0/io.highfidelity.hifiinterface/cache/resources/meshes/being_of_light/being_of_light.fbx", 
            url: TARGET_MODEL_URL,
            dimensions: TARGET_MODEL_DIMENSIONS,
            visible: true,
            position: position,
            rotation: towardUs
        };
        var aTargetOverlay = Overlays.addOverlay("model", targetOverlayProps);
        _this.targetOverlays.push(aTargetOverlay);
        printd ("[DEBUG-TELEPORT] addOverlay debug id " + aTargetOverlay);
    

};
Script.setTimeout(function() {
    printd ("[DEBUG-TELEPORT] Adding some teleport overlays");
    _this.createTargetOverlay({x: 0.0, y:0.0   , z: 0.0});
    _this.createTargetOverlay({x: 10.5, y:0.15 , z: -3.5});
    _this.createTargetOverlay({x: -1.2, y:-0.15, z: -1.1});
    _this.createTargetOverlay({x: 1.2, y:0.15 , z: 1.1});
    _this.createTargetOverlay({x: 1.5, y:-1.15, z: 1.0});
    _this.createTargetOverlay({x: 1.2, y:0.5  , z: -0.5});

    _this.createTargetOverlay({x: 12.8, y:0.1 , z:  5.1 });
    _this.createTargetOverlay({x: 12.6, y:0.1 , z: -1.0});

    _this.createTargetOverlay({x: 12.6, y:0.1 , z:  -6.4});
    _this.createTargetOverlay({x:  7.2, y:0.1 , z: -14.2});
    _this.createTargetOverlay({x:  0.5, y:0.1 , z: -17.3});
    _this.createTargetOverlay({x: 10.1, y:0.2 , z:  -3.1});
}, 1000 * 8);

Script.setTimeout(function() {
    // Your code here
    _this.updateOverlays();

}, 1000 * 20);

printd ("[DEBUG-TELEPORT] addOverlay debug invoked");


}());