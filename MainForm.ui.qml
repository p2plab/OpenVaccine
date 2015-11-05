
import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.2

GridLayout {
    property alias scanButton: scanButton
    property alias cancelButton: cancelButton
    property alias sendButton: sendButton

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
        antialiasing: true
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

    TextArea {
        id:logger
        y: 500
        Layout.rowSpan: 2
        Layout.fillHeight: true
        Layout.fillWidth: true
        activeFocusOnPress: false
        text: appWindow.about
        style:TextAreaStyle {
            textColor: "#eee"
            selectionColor: "steelblue"
            selectedTextColor: "#eee"
            backgroundColor: "#888"
        }
    }
    GridLayout {
        id: rowlayout4
        rows: 1
        columns: 3
        Layout.rowSpan: 1
        Layout.fillHeight: true
        Layout.fillWidth: true
        Button {
            id: scanButton
            text: qsTr("Scan")
            Layout.fillHeight: true
            Layout.fillWidth: true
            style: ButtonStyle {
                background: Rectangle {
                    border.width: control.activeFocus ? 2 : 1
                    border.color: "#888"
                    color: "#eee"
                    radius: 10
                    gradient: Gradient {
                        GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
                        GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
                    }
                }
            }
        }
        Button {
            id: cancelButton
            text: qsTr("Cancel")
            Layout.fillHeight: true
            Layout.fillWidth: true
            style: ButtonStyle {
                background: Rectangle {
                    border.width: control.activeFocus ? 2 : 1
                    border.color: "#888"
                    radius: 10
                    gradient: Gradient {
                        GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
                        GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
                    }
                }
            }
        }
        Button {
            id: sendButton
            text: qsTr("Report")
            Layout.fillHeight: true
            Layout.fillWidth: true
            style: ButtonStyle {
                background: Rectangle {
                    border.width: control.activeFocus ? 2 : 1
                    border.color: "#888"
                    radius: 10
                    gradient: Gradient {
                        GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
                        GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
                    }
                }
            }
        }
    }

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
