import QtQuick 2.0
import QtQml.Models 2.2
import "aside.js" as Aside

ListModel {
    id: serverModel
    Component.onCompleted: Aside.populateServers(serverModel)
}

