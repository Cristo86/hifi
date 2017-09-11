import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import "../../styles-uit"
import "../../controls-uit" as HifiControlsUit
import "../../controls" as HifiControls
import ".."

Item {
    id: bar
    x:0
    y:0
    width: 100
    height: 100

    property bool shown: true

    onShownChanged: {
        bar.visible = shown;
    }

	Rectangle {
        anchors.fill : parent
        color: "transparent"
        Flow {
            id: flowMain
            spacing: 10
            flow: Flow.TopToBottom
            layoutDirection: Flow.TopToBottom
            anchors.fill: parent
            anchors.margins: 4
        }
        /*border.width: 1
        border.color: "red"
        radius: 20*/
    }

    Component.onCompleted: {
        // put on bottom
        x = 0;
        y = 0;
        width = 100;
        height = 100;
    }
    
    function addButton(properties) {
        var component = Qt.createComponent("button.qml");
        if (component.status == Component.Ready) {
            var button = component.createObject(flowMain);
            // copy all properites to button
            var keys = Object.keys(properties).forEach(function (key) {
                button[key] = properties[key];
            });
            return button;
        } else if( component.status == Component.Error) {
            console.log("Load button errors " + component.errorString());
        }
    }

    function urlHelper(src) {
            if (src.match(/\bhttp/)) {
                return src;
            } else {
                return "../../../" + src;
            }
        }

}