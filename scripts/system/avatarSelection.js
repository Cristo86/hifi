"use strict";
//
//  avatarSelection.js
//  scripts/system/
//
//  Created by Gabriel Calero & Cristian Duarte on 21 Sep 2017
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var window;

var logEnabled = true;
var isVisible = false;

function printd(str) {
    if (logEnabled)
        print("[avatarSelection.js] " + str);
}

function fromQml(message) { // messages are {method, params}, like json-rpc. See also sendToQml.
    var data;
    printd("fromQml " + JSON.stringify(message));
    switch (message.method) {
    case 'selectAvatar':
        // use this message.params.avatarUrl
        App.askBeforeSetAvatarUrl(message.params.avatarUrl);
        break;
    case 'openAvatarMarket':
        // good
        App.openUrl("https://metaverse.highfidelity.com/marketplace?category=avatars");
        break;
    case 'hide':
        module.exports.onHidden();
        break;
    default:
        print('[avatarSelection.js] Unrecognized message from avatarSelection.qml:', JSON.stringify(message));
    }
}

function sendToQml(message) {
    if (!window) {
        print("[avatarSelection.js] There is no window object");
        return;
    }
    window.sendToQml(message);
}

function refreshSelected(currentAvatarURL) {
    sendToQml({
        type: "refreshSelected",
        selectedAvatarUrl: currentAvatarURL
    });

    sendToQml({
        type: "showAvatars"
    });
}

function init() {
    if (!window) {
        print("[avatarSelection.js] There is no window object for init()");
        return;
    }
    var DEFAULT_AVATAR_URL = "http://mpassets.highfidelity.com/f14bf7c9-49a1-4249-988a-0a577ed78957-v1/beingOfLight.fst";
    sendToQml({
        type: "addAvatar",
        name: "Being of Light Avatar",
        thumbnailUrl: "https://hifi-metaverse.s3-us-west-1.amazonaws.com/marketplace/previews/f14bf7c9-49a1-4249-988a-0a577ed78957/thumbnail/hifi-mp-f14bf7c9-49a1-4249-988a-0a577ed78957.jpg",
        avatarUrl: DEFAULT_AVATAR_URL
    });
    sendToQml({
        type: "addAvatar",
        name: "Cody",
        thumbnailUrl: "https://hifi-metaverse.s3-us-west-1.amazonaws.com/marketplace/previews/8c859fca-4cbd-4e82-aad1-5f4cb0ca5d53/thumbnail/hifi-mp-8c859fca-4cbd-4e82-aad1-5f4cb0ca5d53.jpg",
        avatarUrl: "http://mpassets.highfidelity.com/8c859fca-4cbd-4e82-aad1-5f4cb0ca5d53-v1/cody.fst"
    });
    sendToQml({
        type: "addAvatar",
        name: "Mixamo Will",
        thumbnailUrl: "https://hifi-metaverse.s3-us-west-1.amazonaws.com/marketplace/previews/d029ae8d-2905-4eb7-ba46-4bd1b8cb9d73/thumbnail/hifi-mp-d029ae8d-2905-4eb7-ba46-4bd1b8cb9d73.jpg",
        avatarUrl: "http://mpassets.highfidelity.com/d029ae8d-2905-4eb7-ba46-4bd1b8cb9d73-v1/4618d52e711fbb34df442b414da767bb.fst"
    });
    sendToQml({
        type: "addAvatar",
        name: "Albert",
        thumbnailUrl: "https://hifi-metaverse.s3-us-west-1.amazonaws.com/marketplace/previews/1e57c395-612e-4acd-9561-e79dbda0bc49/thumbnail/hifi-mp-1e57c395-612e-4acd-9561-e79dbda0bc49.jpg",
        avatarUrl: "http://mpassets.highfidelity.com/1e57c395-612e-4acd-9561-e79dbda0bc49-v1/albert.fst"
    });

    sendToQml({
        type: "addExtraOption",
        showName: "More choices",
        thumbnailUrl: "../../../images/moreAvatars.png",
        methodNameWhenClicked: "openAvatarMarket",
        actionText: "MARKETPLACE"
    });

    var currentAvatarURL = Settings.getValue('Avatar/fullAvatarURL', DEFAULT_AVATAR_URL);
    refreshSelected(currentAvatarURL);
}

module.exports = {
    init: function() {
        window = new QmlFragment({
            menuId: "hifi/android/avatarSelection",
            visible: false
        });
        if (window) {
            window.fromQml.connect(fromQml);
        }
        init();
    },
    show: function() {
        if (window) {
            window.setVisible(true);
            isVisible = true;
        }
    },
    hide: function() {
        if (window) {
            window.setVisible(false);
        }
        isVisible = false;
    },
    destroy: function() {
        if (window) {
            window.fromQml.disconnect(fromQml);   
            window.close();
            window = null;
        }
    },
    isVisible: function() {
        return isVisible;
    },
    width: function() {
        return window ? window.size.x : 0;
    },
    height: function() {
        return window ? window.size.y : 0;
    },
    position: function() {
        return window && isVisible ? window.position : null;
    },
    refreshSelectedAvatar: function(currentAvatarURL) {
        refreshSelected(currentAvatarURL);
    },
    onHidden: function() { }
};
