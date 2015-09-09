import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import Interact 1.0
import "aside.js" as Aside

ApplicationWindow {
    title: qsTr("Sockc Client") + " - " + qsTr("SOCKS5 Proxy")
    visible: true

    Interact {
        id: interact
    }

    MainForm {
        anchors.fill: parent

        username.onAccepted: {
            if (password.length === 0) {
                password.focus = true
            }
        }

        connect.onClicked: {
            interact.connect(Aside.trim(username.text), Aside.trim(password.text), server.data, "17951");
            server.model.addItem
        }
    }

    MessageDialog {
        id: dialog
        title: qsTr("Note")

        function show(caption) {
            dialog.text = caption;
            dialog.open();
        }
    }
}
