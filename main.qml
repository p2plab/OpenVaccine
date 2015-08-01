import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import org.p2plab.openvaccine 1.0

ApplicationWindow {
    property int margin: 11
    width: mainLayout.implicitWidth + 2 * margin
    height: mainLayout.implicitHeight + 2 * margin
    visible: true

    MainForm {
          id: mainLayout
          anchors.fill: parent
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&About")
                onTriggered: messageDialog.show(qsTr("Scan action triggered"));
            }

            MenuItem {
                text: qsTr("&About2")
                onTriggered: messageDialog.show(AppModel.isMobile);
            }
            MenuItem {
                text: qsTr("&Scan")
                onTriggered: appModel.queryScanData("android","2.2","samsung","galuxy 6");
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
    }

    AppModel {
        id: appModel
    }

    MessageDialog {
        id: messageDialog
        title: "알림"
        function show(caption) {
            messageDialog.text = caption;
            messageDialog.open();
        }
    }

    Dialog {
        id: aboutDialog
        title: qsTr("About Open Vaccine")
        Text {
            id: text1
            x: 33
            y: 205
            text: qsTr("-> 경고창 (탐색하기전 필요한 조치들)
'오픈 백신'에 대한 소개(간단히) :

오픈 백신은 국가정보원이 이용한 해킹팅(Hacking Team)의 RCS를 비롯하여, 정부의 사찰과 감시에 이용되는 스파이웨어를 탐지하기 위한 백신입니다. 오픈 소스로 배포되고 전 세계 개발자들의 협력을 통한 '국민 백신 프로젝트'를 통해 제작되었습니다.

-> 탐지(scan) 과정

창의 위 부분에는 탐지 진행 과정을 보여주는 이미지가 나오고, 아래쪽에는 아래와 같은 홍보 메시지 표시. (아래의 홍보 메시지는 현재의 정세에 맞는 문구이며, 향후 업데이트될 때 해당 시기에 맞게 수정될 수 있음.) 뭔가 관련 이미지가 슬라이드처럼 바뀌면 좋을 듯 한데..일단 메시지 내용은 아래처럼하면 어떨까 합니다.

스파이웨어 RCS를 이용한 국정원의 국민 사찰 의혹은 그 진실이 투명하게 규명되어야 합니다.

이용자의 컴퓨터나 스마트폰을 해킹해서 감청하거나 조작하는 것은 법에서 허용하지 않는 불법적인 수사 방식이며, 인권 침해가 지나치게 크기 때문에 결코 허용되어서는 안됩니다

통신사나 메신저 등에 감청 설비를 의무화하는 서상기, 박민식 의원이 발의한 통신비밀보호법 개정안은 폐기되어야 합니다.

인터넷과 휴대전화의 감청 요건을 강화하고, 당사자가 압수수색 과정에 참여하고 감청 여부를 고지받을 수 있는 등 당사자의 인권을 보호하도록 통신비밀보호법을 개정해야 합니다.

국가정보원의 선거 개입과 국민 사찰을 근본적으로 방지하기 위해서는 국가정보원이 개혁되어야 합니다. 국정원의 수사권과 국내정보 수집권한을 다른 기관으로 이관해야 합니다!")
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            font.pixelSize: 12
        }

        function show(caption) {
            messageDialog.text = caption;
            messageDialog.open();
        }
    }


}
