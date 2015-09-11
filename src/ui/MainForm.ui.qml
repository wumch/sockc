import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2
import "aside.js" as Aside

Item {
    anchors.fill: parent
    anchors.margins: 20
    anchors.horizontalCenter: parent.horizontalCenter
    Layout.preferredWidth: 480
    Layout.minimumWidth: 320
    Layout.maximumWidth: 640
    Layout.preferredHeight: 380
    Layout.minimumHeight: 360
    Layout.maximumHeight: 640
    Layout.fillWidth: true

    property int inputPreferWidth: 200
    property int inputMinWidth: 120
    property int inputMaxWidth: 260
    property int inputPreferHeight: 32
    property int inputMinHeight: 30
    property int inputMaxHeight: 45

    property alias username: username
    property alias password: password
    property alias connect: connect
    property alias server: server
    property alias autoPickServer: autoPickServer

    ColumnLayout {
        anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 12

        LayoutRow {
            Text {
                id: caption
                text: qsTr("Sockc Client")

                Layout.fillWidth: true
                Layout.minimumWidth: 200
                Layout.preferredHeight: 40
                Layout.minimumHeight: 24
                Layout.maximumHeight: 80

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop

                minimumPixelSize: 18
                font.capitalization: Font.Capitalize
                fontSizeMode: Text.VerticalFit
                font.pointSize: 20
                font.weight: Font.bold
                font.letterSpacing: font.pixelSize / 4

                color: "green"
                style: Text.Outline
                styleColor: "red"
            }
        }

        LayoutRow {
            LabelText {
                id: usernameLabel
                text: qsTr("Username") + ":"
            }

            LargeTextField {
                id: username
                placeholderText: qsTr("Username")
                focus: true
                validator: RegExpValidator { regExp: /^\w{5,32}$/ }
            }

            AssistCheckBox {
                id: rememberUsername
                text: qsTr("Remember")
                checked: true
            }
        }

        LayoutRow {
            LabelText {
                id: passwordLabel
                text: qsTr("Password") + ":"
            }

            LargeTextField {
                id: password
                placeholderText: qsTr("Password")
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhHiddenText & Qt.ImhLatinOnly & Qt.ImhPreferLowercase
                maximumLength: 32
                validator: RegExpValidator { regExp: /^\S{5,32}$/ }
            }

            AssistCheckBox {
                id: rememberPassword
                text: qsTr("Remember")
                checked: true
            }
        }

        LayoutRow {
            LabelText {
                id: hostLabel
                text: qsTr("Server") + ":"
            }

            ComboBox {
                id: server
                model: ServerModel {}
                enabled: !autoPickServer.checked

                Layout.preferredWidth: username.width - refreshServerList.width - parent.spacing - 3
                Layout.minimumWidth: inputMinWidth
                Layout.maximumWidth: inputMaxWidth
                Layout.preferredHeight: inputPreferHeight
                Layout.minimumHeight: inputMinHeight
                Layout.maximumHeight: inputMaxHeight

                style: ComboBoxStyle {
                    padding.left: 6
                    padding.right: 6
                }
            }

            AssistCheckBox {
                id: autoPickServer
                text: qsTr("Automate")
                checked: true
            }

            Button {
                id: refreshServerList

                Layout.preferredWidth: Aside.refreshWidth
                Layout.minimumWidth: 40
                Layout.maximumWidth: 120
                Layout.preferredHeight: 32
                Layout.minimumHeight: 24
                Layout.maximumHeight: 48

                style: ButtonStyle {
                    label: LabelText {
                        text: qsTr("Refresh")
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        LayoutRow {
            CheckBox {
                id: autoStart

                checked: true

                Layout.minimumWidth: 80
                Layout.maximumWidth: 360
                Layout.preferredHeight: 32
                Layout.minimumHeight: 24
                Layout.maximumHeight: 48

                style: CheckBoxStyle {
                    label: Text {
                        text: qsTr("Automatically start when system start")
                        verticalAlignment: Text.AlignVCenter
                        color: "#333"
                        font.letterSpacing: 1
                    }
                }
            }
        }

        LayoutRow {
            Button {
                id: connect
                Layout.preferredWidth: 160
                Layout.minimumWidth: 120
                Layout.maximumWidth: 240
                Layout.preferredHeight: 32
                Layout.minimumHeight: 24
                Layout.maximumHeight: 48

                style: ButtonStyle {
                    label: LabelText {
                        text: qsTr("Start")
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        LayoutRow {
            Button {
                id: exit
                Layout.preferredWidth: 60
                Layout.minimumWidth: 40
                Layout.maximumWidth: 120
                Layout.preferredHeight: 32
                Layout.minimumHeight: 24
                Layout.maximumHeight: 48

                style: ButtonStyle {
                    label: LabelText {
                        text: qsTr("Exit")
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Button {
                id: hide
                Layout.preferredWidth: 100
                Layout.minimumWidth: 60
                Layout.maximumWidth: 180
                Layout.preferredHeight: 32
                Layout.minimumHeight: 24
                Layout.maximumHeight: 48

                style: ButtonStyle {
                    label: LabelText {
                        text: qsTr("Hide")
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        LayoutRow {
            Layout.topMargin: 10
            Layout.bottomMargin: 20

            TextLink {
                text: "<a href=\"http://www.baidu.com\">" + qsTr("Register") + "</a>"
            }

            TextLink {
                text: "<a href=\"http://www.baidu.com\">" + qsTr("Retrieve password") + "</a>"
            }

            TextLink {
                text: "<a href=\"http://www.baidu.com\">" + qsTr("Renew") + "</a>"
            }
        }
    }
}
