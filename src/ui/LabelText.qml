import QtQuick 2.4
import QtQuick.Layouts 1.1
import "aside.js" as Aside

Text {
    Layout.fillWidth: true
    Layout.preferredWidth: Aside.captionWidth
    Layout.minimumWidth: 60
    Layout.maximumWidth: 120
    Layout.preferredHeight: 32
    Layout.minimumHeight: 30
    Layout.maximumHeight: 45

    horizontalAlignment: Text.AlignRight
    verticalAlignment: Text.AlignVCenter

    smooth: true
    color: "#333"
    minimumPixelSize: 12
    fontSizeMode: Text.VerticalFit
    maximumLineCount: 1
    font.letterSpacing: 2
    font.wordSpacing: 5
    font.weight: Font.DemiBold
}
