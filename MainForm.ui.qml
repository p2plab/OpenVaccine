import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

GridLayout {
    property alias scanButton: scanButton
    property alias cancelButton: cancelButton
    property alias mainLayout: mainLayout
    property alias logger: logger
    property alias busyIndicator: busyIndicator

    id: mainLayout
    anchors.fill: parent
    anchors.margins: margin
    columns: 1
    rows:9
    flow: GridLayout.TopToBottom

    BorderImage {
        id: title
        source: "images/titleBar_nanumgodic.svg"
        Layout.alignment: Qt.AlignCenter
        Layout.rowSpan: 1
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    CircularGauge {
        id: circularGauge
        y: 53
        value: appModel.currentScanPercent
        antialiasing: true

        Layout.rowSpan: 4
        Layout.fillHeight: true
        Layout.fillWidth: true

        style: CircularGaugeStyle {
            id: style
            background:Image {
                source:"images/background.png"
                antialiasing: true
            }
        }

        BusyIndicator {
            id: busyIndicator
            x: 117
            y: -11
            visible: false
            running: false
            anchors.centerIn: parent
        }
    }

    GroupBox{
        id: groupBox1
        title:qsTr("검사:")
        width:parent.width
        Layout.rowSpan: 2
        Layout.fillHeight: true
        Layout.fillWidth: true
        TextArea {
            id:logger
            text:appModel.currentScanFile
            anchors.fill: parent
            activeFocusOnPress: false
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    GridLayout {
        id: rowlayout4
        rows: 1
        columns: 2
        Layout.rowSpan: 1
        Layout.fillHeight: true
        Layout.fillWidth: true
        Button {
            id: scanButton
            text: qsTr("검사")
            Layout.fillHeight: true
            Layout.fillWidth: true
            //style:btnStyle
        }
        Button {
            id: cancelButton
            text: qsTr("취소")
            Layout.fillHeight: true
            Layout.fillWidth: true
            //style:btnStyle
        }
    }
    //    ButtonStyle {
    //        id: btnStyle
    //        background: Rectangle {
    //            implicitWidth: 100
    //            implicitHeight: 25
    //            border.width: control.activeFocus ? 2 : 1
    //            border.color: "#888"
    //            radius: 4
    //            gradient: Gradient {
    //                GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
    //                GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
    //            }
    //        }
    //    }
    //    ButtonStyle {
    //        id: btnStyle
    //        background: Rectangle {
    //            implicitHeight: 22
    //            implicitWidth: window.width / columnFactor
    //            color: control.pressed ? "darkGray" : control.activeFocus ? "#cdd" : "#ccc"
    //            antialiasing: true
    //            border.color: "gray"
    //            radius: height/2
    //            Rectangle {
    //                anchors.fill: parent
    //                anchors.margins: 1
    //                color: "transparent"
    //                antialiasing: true
    //                visible: !control.pressed
    //                border.color: "#aaffffff"
    //                radius: height/2
    //            }
    //        }
    //    }
    states: [
        State {
            name: "scanState"
            PropertyChanges {
                target: busyIndicator
                visible: false
                running: false
            }
        },
        State {
            name: "countState"
            PropertyChanges {
                target: busyIndicator
                visible: true
                running: true
            }
        }
    ]
}
