import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Extras 1.4
import QtWebView 1.1
import "components"

Rectangle {
    id: rectangle1
    //property alias mouseArea: mouseArea

    width: 320
    height: 640

    Text {
        id: title
        anchors.centerIn: parent
        text: "오픈 백신"
        style: Text.Normal
        font.pointSize: 28
        font.italic: false
        font.bold: false
        horizontalAlignment: Text.AlignHCenter
        anchors.verticalCenterOffset: -271
        anchors.horizontalCenterOffset: 1
    }

    Dial {
        id: dial
        x: 63
        y: 123
        width: 197
        height: 210
        value: 60
    }

    DelayButton {
        id: scan
        x: 22
        y: 380
        width: 126
        height: 92
        text: qsTr("검사 시작")
        delay: 20
    }

    Grid {
        id: grid1
        x: 0
        y: 523
        width: 320
        height: 77
        spacing: 2
        rows: 3
        columns: 3

        Label {
            id: versioinTitle
            text: qsTr("엔진 버전")
        }

        Label {
            id: versionValue
            text: qsTr("2015.07.06")
        }

        Button {
            id: button1
            text: qsTr("업데이트")
        }

        Label {
            id: realtimeScan
            text: qsTr("실시간 감시")
        }

        Label {
            id: label1
            text: qsTr("사용안함")
        }
    }

    ToggleButton {
        id: toggleButton1
        x: 185
        y: 380
        width: 111
        height: 92
        text: qsTr("실시간 감시")
    }



}
