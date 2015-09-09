import QtQuick 2.4
import QtQuick.Layouts 1.1

Text {
    onLinkActivated: Qt.openUrlExternally(link)
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onClicked: parent.color = "red"
        cursorShape: Qt.PointingHandCursor
    }

    Layout.fillWidth: false
    Layout.preferredWidth: implicitWidth
    Layout.minimumWidth: 36
    Layout.maximumWidth: 160
    Layout.preferredHeight: 32
    Layout.minimumHeight: 30
    Layout.maximumHeight: 45
    Layout.rightMargin: 10

    horizontalAlignment: Text.AlignRight
    verticalAlignment: Text.AlignVCenter

    smooth: true
    minimumPixelSize: 12
    fontSizeMode: Text.VerticalFit
    maximumLineCount: 1
    font.letterSpacing: 1
    font.wordSpacing: 2
    font.weight: Font.Normal
    font.underline: true

    linkColor: "blue"
}
