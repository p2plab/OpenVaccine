import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.2
import "components"


ColumnLayout {
    id: mainLayout
    anchors.fill: parent
    anchors.margins: margin
    //property alias updateButton: updateButton
    property alias watchButton: watchButton
    property alias scanButton: scanButton
    property alias mainLayout: mainLayout
    //    rows: 10
    //    columns: 1

    //spacing:2


    RowLayout{
        id: rowlayout1
        anchors.horizontalCenter: parent.horizontalCenter

        BorderImage {
            id: title
            width: 300
            source: "images/titleBar_nanumgodic.svg"
            Layout.alignment: Qt.AlignCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    RowLayout {
        id: rowLayout2
        anchors.horizontalCenter: parent.horizontalCenter
        CircularGauge {
            id: circularGauge
            width: 300
            height: 300
            value: 0
            anchors.horizontalCenter: parent.horizontalCenter
            antialiasing: true

            Layout.alignment: Qt.AlignCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
            style: CircularGaugeStyle {
                id: style
                background:Image {
                    source:"images/background.png"
                    antialiasing: true
                }
            }
        }
    }

    RowLayout {
        id: rowlayout4
        width: 261
        anchors.horizontalCenter: parent.horizontalCenter
        DelayButton {
            id: scanButton
            text: qsTr("검사 시작")
            antialiasing: true
            anchors.left: parent.left
            anchors.leftMargin: 0
            delay: 14
        }

        ToggleButton {
            id: watchButton
            text: qsTr("실시간 감시")
            antialiasing: true
            anchors.right: parent.right
            anchors.rightMargin: 0
            clip: false
            checked: false
        }
    }

}
