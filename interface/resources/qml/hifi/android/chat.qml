//
//  Chat.qml
//  interface/resources/qml/android
//
//  Created by Gabriel Calero & Cristian Duarte on 15 Sep 2017
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import "."
import "../../styles"
import "../../styles-uit" as HifiStyles
import "../../controls-uit" as HifiControlsUit
import "../../controls" as HifiControls
import ".."

Item {
	id: top

    property bool shown: true

    property bool typing: false

    signal sendToScript(var message);

	onShownChanged: {
        top.visible = shown;
    }

    function hide() {
        //shown = false;
        sendToScript ({ type: "hide" });
    }

    HifiAndroidConstants { id: android }

    width: parent ? parent.width - android.dimen.windowLessWidth : 0
    height: parent ? parent.height - android.dimen.windowLessHeight : 0
    z: android.dimen.windowZ
    anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }

    HifiConstants { id: hifi }
    HifiStyles.HifiConstants { id: hifiStyleConstants }

	ListModel {
        id: chatContent
        ListElement {
            transmitter: ""
            content: "Type /? or /help for help with chat."
            textColor: "#00FF00"
        }
    }

    Rectangle {
        id: background
        gradient: Gradient {
            GradientStop { position: 0.0; color: android.color.gradientTop }
            GradientStop { position: 1.0; color: android.color.gradientBottom }
        }
        anchors {
            fill: parent
        }

        WindowHeader {
            id: header
            iconSource: "../../../icons/android/chat-i.svg"
            titleText: "CHAT"
        }

        TextField {
            id: input
            Keys.onReturnPressed:{ Qt.inputMethod.hide(); } // sendMessage()
            onTextChanged: textChangedHandler()
            y: 110
            height: 40
            width: 600
            anchors.left: parent.left
            anchors.leftMargin: 25
            font.pointSize: 6
            font.bold: false
            style: TextFieldStyle {
                    textColor: "black"
                    background: Rectangle {
                        radius: 2
                        implicitWidth: 100
                        implicitHeight: 24
                        border.color: "#333"
                        border.width: 1
                    }
                }
        }

        HifiControlsUit.ImageButton {
            width: 140
            height: 35
            text: "SEND"
            source: "../../../images/button.svg"
            hoverSource: "../../../images/button-a.svg"
            fontSize: 18
            fontColor: "#2CD8FF"
            hoverFontColor: "#FFFFFF"
            anchors {
                verticalCenter: input.verticalCenter
                right: parent.right
                rightMargin: android.dimen.headerHideRightMargin
                leftMargin: 10
            }
            onClicked: sendMessage()
        }

        ListView {
            id: chatView
            width: parent.width-5
            height: parent.height - y - 5
            anchors {
                top: input.bottom
                topMargin: 5
                horizontalCenter: parent.horizontalCenter
                left: input.left
            }
            model: chatContent
            clip: true
            delegate: Component {
                Row {
                    Text {
                        font.pointSize: 6
                        text: transmitter?transmitter:""
                        color: textColor
                    }
                    Text {
                        font.pointSize: 6
                        text: content
                        color: "#ffffff"
                    }

                }

            
            }
        }
            
/*
            Text {
                id: otherTyping
                anchors.top: input.bottom
                anchors.left: parent.left
                font.pointSize: 4
                font.bold: false
            }
*/


    }
    Timer {
        id: typingTimer
        interval: 1000; 
        running: false; 
        repeat: false;
        onTriggered: typingTimerTriggered();
    }

    function textChangedHandler()
    {
        //console.log("[CHAT] text changed");
        type();
    }


    function typingTimerTriggered() {
        typing = false;
        handleEndTyping();
    }

    function fromScript(message) {
        //console.log("[CHAT] fromScript " + JSON.stringify(message));
        switch (message.type) {
            case "ReceiveChatMessage":
                showMessage(message.avatarID, 
                            message.displayName, 
                            message.color,
                            message.message, 
                            message.data);
                break;
            case "LogMessage":
                logMessage(message.message);
                //scrollChatLog();
                break;
            case "refreshTyping":
                updateAvatarTypingText ( message.displayNames );
                break;
            case "updateSettings":
                // Update anything that might depend on the settings.
                break;
            case "clearChat":
                chatContent.clear();
                break;
            default:
        }
    }

    function showMessage(avatarID, displayName, color, message, data) {
        chatContent.append({ transmitter: displayName+": ", content: message, textColor: color });
    }

    // Append a log message
    function logMessage(message) {
        chatContent.append({content: message, textColor: "#EEEEEEFF"});
    }

    function sendMessage()
    {
        // toogle focus to force end of input method composer
        var hasFocus = input.focus;
        input.focus = false;

        var data = input.text
        input.text = "";
    
        if (data == '') {
            emptyChatMessage();
        } else {
            handleChatMessage(data, {});
        }

        
        //input.focus = hasFocus;
        Qt.inputMethod.hide();
    }

    // The user entered an empty chat message.
    function emptyChatMessage() {
        sendToScript ({ type: "EmptyChatMessage" });
    }

    // The user entered a non-empty chat message.
    function handleChatMessage(message, data) {
        //console.log("handleChatMessage", message);
        sendToScript({
                type: "HandleChatMessage",
                message: message,
                data: data
            });
    }

    // Call this on every keystroke.
    function type() {
        //console.log("[CHAT] type");
        if (typing && input.text == "") {
            // endTyping
            return;
        }
        beginTyping();
        handleType();
    }

   

    function updateAvatarTypingText(displayNamesTyping) {
        //console.log ("[CHAT] updateAvatarTypingText " + displayNamesTyping + " (" + displayNamesTyping.length+")");
        var str = "";

        for (var i=0; i < displayNamesTyping.length; i++) {
            str += displayNamesTyping[i];
            if (i < displayNamesTyping.length - 1) {
                str += ", ";
            }
        }

        if (displayNamesTyping.length <= 0) {
            //otherTyping.text="";
        } else if (displayNamesTyping.length == 1) {
            //otherTyping.text = str + " is typing";
        } else {
            //otherTyping.text = str + " are typing";
        }

    }

    function beginTyping() {
        //console.log("[CHAT] beginTyping");
        typingTimer.restart();

        if (typing) {
            return;
        }

        typing = true;
        handleBeginTyping();
    }

    // Clear the typing timer and notify if we're finished.
    function endTyping() {
        typingTimer.stop();

        if (!typing) {
            return;
        }

        typing = false;
        handleEndTyping();
    }

    // Notify the interface script when we begin typing.
    function handleBeginTyping() {
        sendToScript({ type: "BeginTyping" });
    }

    // Notify the interface script on every keystroke.
    function handleType() {
        sendToScript({ type: "Type" });
    }

    // Notify the interface script when we end typing.
    function handleEndTyping() {
        sendToScript({ type: "EndTyping" });
    }


	Component.onCompleted: {
        //console.log("[CHAT] qml loaded");
	}
}
