import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Extras 1.4
import QtWebView 1.1

Rectangle {
    property alias mouseArea: mouseArea

    width: 360
    height: 360

    MouseArea {
        id: mouseArea
        visible: true
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
        clip: false
        anchors.fill: parent
    }

    Text {
        id: text1
        anchors.centerIn: parent
        text: "Hello World"
    }
    states: [
        State {
            name: "State1"

            PropertyChanges {
                target: mouseArea
                anchors.bottomMargin: 0
                anchors.rightMargin: 0
                anchors.leftMargin: 1
                anchors.topMargin: 0
            }

            PropertyChanges {
                target: text1
                anchors.verticalCenterOffset: 0
                anchors.horizontalCenterOffset: 0
            }
        }
    ]
}
