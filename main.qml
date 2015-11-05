import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import org.p2plab.openvaccine 1.0

ApplicationWindow {
    id:appWindow
    property int margin: 11
    width: mainLayout.implicitWidth + 2 * margin
    height: mainLayout.implicitHeight + 2 * margin
    visible: true

    property var aboutkr:qsTr("오픈 백신은 국가정보원이 이용한 해킹팅(Hacking Team)의 RCS를 비롯하여,\n 정부의 사찰과 감시에 이용되는 스파이웨어를 탐지하기 위한 백신입니다.\n\오픈 소스로 배포되고 전 세계 개발자들의 협력을 통한 '국민 백신 프로젝트'를 통해 제작되었습니다.\n스파이웨어 RCS를 이용한 국정원의 국민 사찰 의혹은 그 진실이 투명하게 규명되어야 합니다.\n이용자의 컴퓨터나 스마트폰을 해킹해서 감청하거나 조작하는 것은 법에서 허용하지 않는 불법적인 수사 방식이며,\n인권 침해가 지나치게 크기 때문에 결코 허용되어서는 안됩니다.\n통신사나 메신저 등에 감청 설비를 의무화하는 서상기, 박민식 의원이 발의한 통신비밀보호법 개정안은 폐기되어야 합니다.\n인터넷과 휴대전화의 감청 요건을 강화하고, 당사자가 압수수색 과정에 참여하고 감청 여부를 고지받을 수 있는 등 당사자의 인권을 보호하도록 통신비밀보호법을 개정해야 합니다.\n국가정보원의 선거 개입과 국민 사찰을 근본적으로 방지하기 위해서는 국가정보원이 개혁되어야 합니다. 국정원의 수사권과 국내정보 수집권한을 다른 기관으로 이관해야 합니다!")
    property var about:qsTr("Open Vaccine is a free and open source model anti-malware software designed to detect RCS (Remote Control System), Hacking Team’s spyware used by intelligence agencies in many countries including the National Intelligence Service (NIS) of South Korea. Open Vaccine was, in the first phase, developed for Android smartphones. Open Vaccine does not aim to become a commercial program against all types of malware and spyware. Open Vaccine’s primary goal is to detect the RCS, and hereafter, other malware known to be used to spy on the citizen.")
    MainForm {
          id: mainLayout
          anchors.fill: parent
          scanButton.onClicked: {
              log(qsTr("Searching File..."))
              console.log("scan...")
              state = "scanState"
              appModel.scanDefectFile();
              scanButton.enabled = false;
          }
          cancelButton.onClicked: {
              log(qsTr("Cancel"));
              state = "base state";
              console.log(qsTr("Cancel"));
              appModel.cancelScan();
              scanButton.enabled = true;
          }

          sendButton.onClicked:{
              appModel.email = mainLayout.logger.text;
          }

          function log(msg){
              mainLayout.logger.text = msg;
          }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("About...")
                onTriggered: mainLayout.log(about);
            }
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
    }

    AppModel {
        id: appModel
        onCurrentScanPercentChanged: percentChanged(percent);

        function percentChanged(percent){
            console.log("percent:",percent);
            if(percent === 100){

                mainLayout.scanButton.enabled = true;
                var files = "";
                for(var i in appModel.defectFiles){
                  files += appModel.defectFiles[i] + "\n"
                }
                var info =""
                info += appModel.constants["productOS"] + "\n"
                info += appModel.constants["productVersion"] + "\n"
                info += appModel.constants["productModel"] + "\n"
                info += appModel.constants["productName"] + "\n"
                info += appModel.constants["productManufacturer"] + "\n"
                var report = "[device]\n" + info

                if (appModel.defectCount === 0){
                    messageDialog.show(qsTr("Scan Complete!"), qsTr("RCS File not found")); //해킹팀 감시코드가 검출되지 않았습니다.
                    mainLayout.log(report);
                }else{
                    report = report + "[defect]\n" + files
                    messageDialog.show(qsTr("Detect infected file!"), qsTr("RCS File found:" + files )); //해킹팀 감시 프로그램이 검출되었습니다! 
                    mainLayout.log(report);
                }
            }
        }
        onCurrentScanFileChanged:{
            console.log(qsTr("change File"),currentScanFile);
            mainLayout.log(qsTr("Scan file signature:")+currentScanFile);
        }

        onFileCountStart:{
            mainLayout.state = "countState"
        }
        onFileCountComplete:{
            mainLayout.state = "scanState"
        }
    }

    MessageDialog {
        id:messageDialog
        title:qsTr("alert")
        text:""
        function show(caption,msg) {
            messageDialog.title = caption
            messageDialog.text = msg;
            messageDialog.open();
        }
    }

    Dialog{
        id: aboutDialog
        title: qsTr("About Open Vaccine")
        property alias text: textid.text
        standardButtons: StandardButton.Ok
        TextArea {
            id: textid
            implicitWidth: 250
            implicitHeight: 350
            text:qsTr("Hello blue sky!")
        }
        function show(msg){
            aboutDialog.text = msg;
            aboutDialog.open();
        }
    }
}
