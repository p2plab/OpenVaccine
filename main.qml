import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

ApplicationWindow {
    property int margin: 11
    width: mainLayout.implicitWidth + 2 * margin
    height: mainLayout.implicitHeight + 2 * margin
    minimumWidth: mainLayout.Layout.minimumWidth + 2 * margin
    minimumHeight: mainLayout.Layout.minimumHeight + 2 * margin
    visible: true

    MainForm {
          id: mainLayout
          anchors.fill: parent
//        mouseArea.onClicked: {
//            Qt.quit();
//        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&Scan")
                onTriggered: messageDialog.show(qsTr("Scan action triggered"));
            }
            MenuItem {
                text: qsTr("&Watch")
                onTriggered: messageDialog.show(qsTr("Watch action triggered"));
            }
            MenuItem {
                text: qsTr("&Update")
                onTriggered: messageDialog.show(qsTr("Update action triggered"));
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
    }

    MessageDialog {
        id: messageDialog
        title: qsTr("May I have your attention, please?")

        function show(caption) {
            messageDialog.text = caption;
            messageDialog.open();
        }
    }
}
