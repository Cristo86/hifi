//
//  WalletSetupLightbox.qml
//  qml/hifi/commerce/wallet
//
//  WalletSetupLightbox
//
//  Created by Zach Fox on 2017-08-17
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import Hifi 1.0 as Hifi
import QtQuick 2.5
import QtQuick.Controls 1.4
import "../../../styles-uit"
import "../../../controls-uit" as HifiControlsUit
import "../../../controls" as HifiControls

// references XXX from root context

Rectangle {
    HifiConstants { id: hifi; }

    id: root;
    property string lastPage: "";
    // Style
    color: "white";

    Hifi.QmlCommerce {
        id: commerce;

        onLoginStatusResult: {
            if (isLoggedIn) {
                securityImageContainer.visible = true;
            } else {
                loginPageContainer.visible = true;
            }
        }
        
        onSecurityImageResult: {
            if (imageID !== 0) { // "If security image is set up"
                passphrasePageSecurityImage.source = securityImageSelection.getImagePathFromImageID(imageID);
                if (root.lastPage === "") {
                    securityImageContainer.visible = false;
                    choosePassphraseContainer.visible = true;
                }
            } else if (root.lastPage === "securityImage") {
                // ERROR! Invalid security image.
                securityImageContainer.visible = true;
                choosePassphraseContainer.visible = false;
            }
        }

        onPassphraseSetupStatusResult: {
            securityImageContainer.visible = false;
            if (passphraseIsSetup) {
                privateKeysReadyContainer.visible = true;
            } else {
                choosePassphraseContainer.visible = true;
            }
        }
    }

    //
    // LOGIN PAGE START
    //
    Item {
        id: loginPageContainer;
        visible: false;
        // Anchors
        anchors.fill: parent;

        Component.onCompleted: {
            commerce.getLoginStatus();
        }

        Item {
            // Size
            width: parent.width;
            height: 50;
            // Anchors
            anchors.left: parent.left;
            anchors.top: parent.top;

            // Title Bar text
            RalewaySemiBold {
                text: "WALLET SETUP - LOGIN";
                // Text size
                size: hifi.fontSizes.overlayTitle;
                // Anchors
                anchors.top: parent.top;
                anchors.left: parent.left;
                anchors.leftMargin: 16;
                anchors.bottom: parent.bottom;
                width: paintedWidth;
                // Style
                color: hifi.colors.darkGray;
                // Alignment
                horizontalAlignment: Text.AlignHLeft;
                verticalAlignment: Text.AlignVCenter;
            }
        }

        // Navigation Bar
        Item {
            // Size
            width: parent.width;
            height: 100;
            // Anchors:
            anchors.left: parent.left;
            anchors.bottom: parent.bottom;

            // "Cancel" button
            HifiControlsUit.Button {
                color: hifi.buttons.black;
                colorScheme: hifi.colorSchemes.dark;
                anchors.top: parent.top;
                anchors.topMargin: 3;
                anchors.bottom: parent.bottom;
                anchors.bottomMargin: 3;
                anchors.left: parent.left;
                anchors.leftMargin: 20;
                width: 100;
                text: "Cancel"
                onClicked: {
                    
                }
            }
        }
    }
    //
    // LOGIN PAGE END
    //

    //
    // SECURITY IMAGE SELECTION START
    //
    Item {
        id: securityImageContainer;
        visible: false;
        // Anchors
        anchors.fill: parent;

        onVisibleChanged: {
            if (visible) {
                commerce.getSecurityImage();
            }
        }

        Item {
            id: securityImageTitle;
            // Size
            width: parent.width;
            height: 50;
            // Anchors
            anchors.left: parent.left;
            anchors.top: parent.top;

            // Title Bar text
            RalewaySemiBold {
                text: "WALLET SETUP - STEP 1 OF 3";
                // Text size
                size: hifi.fontSizes.overlayTitle;
                // Anchors
                anchors.top: parent.top;
                anchors.left: parent.left;
                anchors.leftMargin: 16;
                anchors.bottom: parent.bottom;
                width: paintedWidth;
                // Style
                color: hifi.colors.darkGray;
                // Alignment
                horizontalAlignment: Text.AlignHLeft;
                verticalAlignment: Text.AlignVCenter;
            }
        }

        // Text below title bar
        RalewaySemiBold {
            id: securityImageTitleHelper;
            text: "Choose a Security Picture:";
            // Text size
            size: 24;
            // Anchors
            anchors.top: securityImageTitle.bottom;
            anchors.left: parent.left;
            anchors.leftMargin: 16;
            height: 50;
            width: paintedWidth;
            // Style
            color: hifi.colors.darkGray;
            // Alignment
            horizontalAlignment: Text.AlignHLeft;
            verticalAlignment: Text.AlignVCenter;
        }

        SecurityImageSelection {
            id: securityImageSelection;
            // Anchors
            anchors.top: securityImageTitleHelper.bottom;
            anchors.left: parent.left;
            anchors.leftMargin: 16;
            anchors.right: parent.right;
            anchors.rightMargin: 16;
            height: 280;
        }

        // Text below security images
        RalewaySemiBold {
            text: "<b>Your security picture shows you that the service asking for your passphrase is authorized. You can change your secure picture at any time.</b>";
            // Text size
            size: 18;
            // Anchors
            anchors.top: securityImageSelection.bottom;
            anchors.topMargin: 40;
            anchors.left: parent.left;
            anchors.leftMargin: 16;
            anchors.right: parent.right;
            anchors.rightMargin: 16;
            height: paintedHeight;
            // Style
            color: hifi.colors.darkGray;
            wrapMode: Text.WordWrap;
            // Alignment
            horizontalAlignment: Text.AlignHLeft;
            verticalAlignment: Text.AlignVCenter;
        }

        // Navigation Bar
        Item {
            // Size
            width: parent.width;
            height: 100;
            // Anchors:
            anchors.left: parent.left;
            anchors.bottom: parent.bottom;

            // "Cancel" button
            HifiControlsUit.Button {
                color: hifi.buttons.black;
                colorScheme: hifi.colorSchemes.dark;
                anchors.top: parent.top;
                anchors.topMargin: 3;
                anchors.bottom: parent.bottom;
                anchors.bottomMargin: 3;
                anchors.left: parent.left;
                anchors.leftMargin: 20;
                width: 100;
                text: "Cancel"
                onClicked: {
                    signalSent({method: 'securityImageSelection_cancelClicked'});
                }
            }

            // "Next" button
            HifiControlsUit.Button {
                color: hifi.buttons.black;
                colorScheme: hifi.colorSchemes.dark;
                anchors.top: parent.top;
                anchors.topMargin: 3;
                anchors.bottom: parent.bottom;
                anchors.bottomMargin: 3;
                anchors.right: parent.right;
                anchors.rightMargin: 20;
                width: 100;
                text: "Next";
                onClicked: {
                    root.lastPage = "securityImage";
                    commerce.chooseSecurityImage(securityImageSelection.getSelectedImageIndex());
                    securityImageContainer.visible = false;
                    choosePassphraseContainer.visible = true;
                }
            }
        }
    }
    //
    // SECURITY IMAGE SELECTION END
    //

    //
    // SECURE PASSPHRASE SELECTION START
    //
    Item {
        id: choosePassphraseContainer;
        visible: false;
        // Anchors
        anchors.fill: parent;

        onVisibleChanged: {
            if (visible) {
                commerce.getPassphraseSetupStatus();
            }
        }

        Item {
            // Size
            width: parent.width;
            height: 50;
            // Anchors
            anchors.left: parent.left;
            anchors.top: parent.top;

            // Title Bar text
            RalewaySemiBold {
                text: "WALLET SETUP - STEP 2 OF 3";
                // Text size
                size: hifi.fontSizes.overlayTitle;
                // Anchors
                anchors.top: parent.top;
                anchors.left: parent.left;
                anchors.leftMargin: 16;
                anchors.bottom: parent.bottom;
                width: paintedWidth;
                // Style
                color: hifi.colors.darkGray;
                // Alignment
                horizontalAlignment: Text.AlignHLeft;
                verticalAlignment: Text.AlignVCenter;
            }
        }

        // Navigation Bar
        Item {
            // Size
            width: parent.width;
            height: 100;
            // Anchors:
            anchors.left: parent.left;
            anchors.bottom: parent.bottom;

            // "Back" button
            HifiControlsUit.Button {
                color: hifi.buttons.black;
                colorScheme: hifi.colorSchemes.dark;
                anchors.top: parent.top;
                anchors.topMargin: 3;
                anchors.bottom: parent.bottom;
                anchors.bottomMargin: 3;
                anchors.left: parent.left;
                anchors.leftMargin: 20;
                width: 100;
                text: "Back"
                onClicked: {
                    root.lastPage = "choosePassphrase";
                    choosePassphraseContainer.visible = false;
                    securityImageContainer.visible = true;
                }
            }
            

            // Security Image
            Image {
                id: passphrasePageSecurityImage;
                // Anchors
                anchors.top: parent.top;
                anchors.topMargin: 3;
                anchors.right: passphrasePageNextButton.left;
                height: passphrasePageNextButton.height;
                width: height;
                anchors.verticalCenter: parent.verticalCenter;
                fillMode: Image.PreserveAspectFit;
                mipmap: true;
            }

            // "Next" button
            HifiControlsUit.Button {
                id: passphrasePageNextButton;
                color: hifi.buttons.black;
                colorScheme: hifi.colorSchemes.dark;
                anchors.top: parent.top;
                anchors.topMargin: 3;
                anchors.bottom: parent.bottom;
                anchors.bottomMargin: 3;
                anchors.right: parent.right;
                anchors.rightMargin: 20;
                width: 100;
                text: "Next";
                onClicked: {

                }
            }
        }
    }
    //
    // SECURE PASSPHRASE SELECTION END
    //

    //
    // PRIVATE KEYS READY START
    //
    Item {
        id: privateKeysReadyContainer;
        visible: false;
        // Anchors
        anchors.fill: parent;

        Item {
            // Size
            width: parent.width;
            height: 50;
            // Anchors
            anchors.left: parent.left;
            anchors.top: parent.top;

            // Title Bar text
            RalewaySemiBold {
                text: "WALLET SETUP - STEP 2 OF 3";
                // Text size
                size: hifi.fontSizes.overlayTitle;
                // Anchors
                anchors.top: parent.top;
                anchors.left: parent.left;
                anchors.leftMargin: 16;
                anchors.bottom: parent.bottom;
                width: paintedWidth;
                // Style
                color: hifi.colors.darkGray;
                // Alignment
                horizontalAlignment: Text.AlignHLeft;
                verticalAlignment: Text.AlignVCenter;
            }
        }
    }
    //
    // PRIVATE KEYS READY END
    //

    //
    // FUNCTION DEFINITIONS START
    //
    signal signalSent(var msg);
    //
    // FUNCTION DEFINITIONS END
    //
}
