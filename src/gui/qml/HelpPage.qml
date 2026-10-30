import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! Help page: renders the plain-text help file shipped with the data
 *  directory. */
Item {
    id: page

    readonly property bool rich: bridge.helpText.charAt(0) === "<"

    /*! Paint anchors with the theme accent; the default is a harsh pure blue
     *  that is unreadable on the dark surfaces. */
    function themedLinks(html) {
        return html.replace(/<a href=/g,
                            "<a style=\"color:" + Theme.accentHex + ";\" href=");
    }

    Card {
        anchors.fill: parent
        anchors.margins: Theme.pad
        title: qsTr("Help")

        Flickable {
            id: helpFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: helpText.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ThemedScrollBar { }

            Text {
                id: helpText
                width: helpFlick.width
                textFormat: page.rich ? Text.RichText : Text.PlainText
                color: Theme.text
                font.pixelSize: Theme.baseSize
                font.family: page.rich ? Theme.fontFamily : "monospace"
                /* QtQuick's Text ignores `linkColor` on Qt 5, so the anchors are
                 * inked by hand before the markup reaches the renderer. */
                text: page.rich ? page.themedLinks(bridge.helpText) : bridge.helpText
                lineHeight: 1.15
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }
}
