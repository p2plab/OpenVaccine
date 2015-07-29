import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.2
import "components"


//ColumnLayout {
//    id: mainLayout
//    anchors.fill: parent
//    anchors.margins: margin

////    GroupBox {
////        id: rowBox
////        title: "Row layout"
////        Layout.fillWidth: true

//        RowLayout {
//            id: rowLayout
//            anchors.fill: parent
//            TextField {
//                placeholderText: "This wants to grow horizontally"
//                Layout.fillWidth: true
//            }
//            Button {
//                text: "Button"
//            }
//        }
////    }

//    GroupBox {
//        id: gridBox
//        title: "Grid layout"
//        Layout.fillWidth: true

//        GridLayout {
//            id: gridLayout
//            rows: 3
//            flow: GridLayout.TopToBottom
//            anchors.fill: parent

//            Label { text: "Line 1" ;font.family: "Times New Roman" }
//            Label { text: "Line 2" }
//            Label { text: "Line 3" }

//            TextField { }
//            TextField { }
//            TextField { }

//            TextArea {
//                text: "This widget spans over three rows in the GridLayout.\n"
//                    + "All items in the GridLayout are implicitly positioned from top to bottom."
//                Layout.rowSpan: 3
//                Layout.fillHeight: true
//                Layout.fillWidth: true
//            }
//        }
//    }
//    TextArea {
//        id: t3
//        text: "This fills the whole cell"
//        Layout.minimumHeight: 30
//        Layout.fillHeight: true
//        Layout.fillWidth: true
//    }
//}


GridLayout {
    id: mainLayout
    anchors.fill: parent
    anchors.margins: margin
    property alias updateButton: updateButton
    property alias watchButton: watchButton
    property alias scanButton: scanButton
    property alias mainLayout: mainLayout
    rows: 10
    columns: 1

    //spacing:2

    RowLayout{
        id: rowlayout1
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.alignment: Qt.AlignCenter
        Layout.fillHeight: true
        Layout.fillWidth: true
        Text {
            id: title
            text: "오픈 백신(베타)"
            anchors.horizontalCenter: parent.horizontalCenter
            style: Text.Normal
            font.pointSize: 28
            font.italic: false
            font.bold: false
            horizontalAlignment: Text.AlignHCenter
        }
    }

    RowLayout {
        id: rowLayout2
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.alignment: Qt.AlignCenter

        Layout.rowSpan: 5
        Layout.fillHeight: true
        Layout.fillWidth: true

        CircularGauge {
            id: circularGauge
            Layout.fillHeight: true
            Layout.fillWidth: true

            visible: true
            style: CircularGaugeStyle {
                id: style
                background:Image {
                    source:"images/background.png"
                }
//                needle:Image {
                //                    source:"images/needle.png"
                //                }
            }
        }
    }

    RowLayout {
        id: rowlayout3
        Layout.alignment: Qt.AlignCenter
        Layout.rowSpan: 1
        Layout.fillHeight: true
        Layout.fillWidth: true

        ProgressBar {
            id: progressBar1
            value: 0.1
            orientation: 1
            Layout.alignment: Qt.AlignCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    RowLayout {
        id: rowlayout4
        Layout.alignment: Qt.AlignCenter
        Layout.rowSpan: 2
        Layout.fillHeight: true
        Layout.fillWidth: true

        DelayButton {
            id: scanButton
            text: qsTr("검사 시작")
            anchors.left: parent.left
            anchors.leftMargin: 0
            delay: 14
        }

        ToggleButton {
            id: watchButton
            text: qsTr("실시간 감시")
            anchors.right: parent.right
            anchors.rightMargin: 0
            clip: false
            checked: false
        }
    }

    RowLayout {
        id: rowlayout5
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true

        Button {
            id: updateButton
            text: qsTr("업데이트")
            anchors.right: parent.right
            anchors.rightMargin: 0
        }

        Label {
            id: versionValue
            text: qsTr("2015.07.06")
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            id: versioinTitle
            text: qsTr("엔진 버전")
            anchors.left: parent.left
            anchors.leftMargin: 0
        }

    }
}
