import QtQuick 2.2
import QtQuick.Controls 1.2
import QtWebKit 3.0

ScrollView {
    width: 1280
    height: 720
    WebView {
        id: webview
        url: "http://qt-project.org"
        anchors.fill: parent
        onNavigationRequested: {
            // detect URL scheme prefix, most likely an external link
            var schemaRE = /^\w+:/;
            if (schemaRE.test(request.url)) {
                request.action = WebView.AcceptRequest;
            } else {
                request.action = WebView.IgnoreRequest;
                // delegate request.url here
            }
        }
    }
}
