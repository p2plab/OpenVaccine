import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.2
import "components"


ColumnLayout {
    id: mainLayout
    anchors.fill: parent
    anchors.margins: margin

//    GroupBox {
//        id: rowBox
//        title: "Row layout"
//        Layout.fillWidth: true

        RowLayout {
            id: rowLayout
            anchors.fill: parent
            TextField {
                placeholderText: "This wants to grow horizontally"
                Layout.fillWidth: true
            }
            Button {
                text: "Button"
            }
        }
//    }

    GroupBox {
        id: gridBox
        title: "Grid layout"
        Layout.fillWidth: true

        GridLayout {
            id: gridLayout
            rows: 3
            flow: GridLayout.TopToBottom
            anchors.fill: parent

            Label { text: "Line 1" }
            Label { text: "Line 2" }
            Label { text: "Line 3" }

            TextField { }
            TextField { }
            TextField { }

            TextArea {
                text: "This widget spans over three rows in the GridLayout.\n"
                    + "All items in the GridLayout are implicitly positioned from top to bottom."
                Layout.rowSpan: 3
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
    TextArea {
        id: t3
        text: "This fills the whole cell"
        Layout.minimumHeight: 30
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}


//Item {
//    anchors.fill: parent

//    property alias updateButton: updateButton
//    property alias watchButton: watchButton
//    property alias scanButton: scanButton
//    property alias mainLayout: mainLayout
//    ColumnLayout {
//        id: mainLayout
//        anchors.fill: parent
//        Text {
//            id: title
//            x: 108
//            y: 24
//            text: "오픈 백신(베타)"
//            anchors.horizontalCenter: parent.horizontalCenter
//            style: Text.Normal
//            font.pointSize: 28
//            font.italic: false
//            font.bold: false
//            horizontalAlignment: Text.AlignHCenter
//        }
//        Item {
//            x: 62
//            y: 125
//            width: 197
//            height: 210
//            anchors.horizontalCenter: parent.horizontalCenter

//            Dial {
//                id: dial
//                value: 60
//                anchors.centerIn: parent
////                value: slider.x * 100 / (container.width - 32)
//            }
//        }

//        Row {
//            id: row3
//            x: 0
//            y: 420
//            width: 280
//            height: 120
//            anchors.horizontalCenter: parent.horizontalCenter

//            DelayButton {
//                id: scanButton
//                y: 0
//                width: 126
//                height: 92
//                text: qsTr("검사 시작")
//                anchors.left: parent.left
//                anchors.leftMargin: 0
//                delay: 14
//            }

//            ToggleButton {
//                id: watchButton
//                x: 0
//                y: 0
//                width: 116
//                height: 92
//                text: qsTr("실시간 감시")
//                anchors.right: parent.right
//                anchors.rightMargin: 0
//                clip: false
//                checked: false
//            }
//        }

//        Row {
//            id: row4
//            x: 17
//            y: 519
//            width: 287
//            height: 42
//            anchors.horizontalCenter: parent.horizontalCenter

//            Button {
//                id: updateButton
//                text: qsTr("업데이트")
//                anchors.right: parent.right
//                anchors.rightMargin: 0
//                anchors.verticalCenter: parent.verticalCenter
//            }

//            Label {
//                id: versionValue
//                text: qsTr("2015.07.06")
//                anchors.horizontalCenter: parent.horizontalCenter
//                anchors.verticalCenter: parent.verticalCenter
//            }

//            Label {
//                id: versioinTitle
//                text: qsTr("엔진 버전")
//                anchors.left: parent.left
//                anchors.leftMargin: 0
//                anchors.verticalCenter: parent.verticalCenter
//            }

//        }
//    }
//}
