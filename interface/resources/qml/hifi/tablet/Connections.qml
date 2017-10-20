//
//  Connections.qml
//  interface/resources/qml/tablet
//
//  Created by Gabriel Calero & Cristian Duarte on 30 Aug 2017
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
import "../../styles-uit"
import "../../controls-uit" as HifiControlsUit
import "../../controls" as HifiControls
import ".."

Rectangle {
    property var modelData: []; // This simple list is essentially a mirror of the connectionModel without all the extra complexities.
    anchors.fill : parent


    property int gap: 14

    color: "#00000000"

    HifiControlsUit.Table {
        id: table;
        flickableItem.interactive: true;
        customBackgroundColor: "#00000000"//"#7fff0000"
        customBorderColor : "#00000000"
        customRadius : 0
        height: 900
        width: 400;
        // Anchors
        anchors.fill: parent
        // Properties
        sortIndicatorVisible: false;
        headerVisible: false;

        TableViewColumn {
            id: displayNameHeader;
            role: "displayName";
            title: table.rowCount + (table.rowCount === 1 ? " NAME" : " NAMES");
            width: table.width;
            movable: false;
            resizable: false;
        }
        model: ListModel {
            id: connectionModel;
        }

        // This Rectangle refers to each Row in the table.
        rowDelegate: Rectangle { // The only way I know to specify a row height.
            // Size
            height: 91//rowHeight ;
            color: "#00000000" // hifi.colors.tableRowLightEven
            //color: "#55558800"
        }

        // This Item refers to the contents of each Cell
        itemDelegate: Item {
            id: itemCell;
            // This NameCard refers to the cell that contains an avatar's
            // DisplayName and UserName
            SimpleNameCard {
                objectName: (model && model.sessionId) || "";
                uuid: (model && model.sessionId) || "";
                // Properties
                visible: true
                profileUrl: (model && model.profileUrl) || "";
                displayName: "";
                userName: model ? model.userName : "";
                placeName: "Placename here"; // model ? model.placeName : ""
                connectionStatus : model && model.connection ? model.connection : "";
                selected: styleData.selected;
                // Size
                width: table.width
                height: parent.height
                // Anchors
                anchors.left: parent.left
            }
        }       

    }

    function loadConnections() {
        connectionModel.clear();
        var userIndex = 0;
        modelData.forEach(function (datum) {
            datum.userIndex = userIndex++;
            // console.log("[NEARBY] load " + datum.userIndex + " " + datum.userName);
            connectionModel.append(datum);
        });
    }

    function updateProperty(index, name, value) {
        //console.log("[NEARBY] updated property " + index + " ["+name+"] ="+value);
        var datum = modelData[index];
        datum[name] = value;
        connectionModel.setProperty(index, name, value);
        //console.log("[NEARBY] updated property " + index + " ["+name+"] ="+value + " (DONE)");
    }
}