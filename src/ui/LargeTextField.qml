import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2

TextField {
    Layout.fillWidth: true
    Layout.preferredWidth: inputPreferWidth
    Layout.minimumWidth: inputMinWidth
    Layout.maximumWidth: inputMaxWidth
    Layout.preferredHeight: inputPreferHeight
    Layout.minimumHeight: inputMinHeight
    Layout.maximumHeight: inputMaxHeight

    smooth: true
    font.pointSize: 13
    font.letterSpacing: font.pixelSize / 7
    font.wordSpacing: 5

    style: TextFieldStyle {
        padding.left: 6
        padding.right: 6
    }
}
