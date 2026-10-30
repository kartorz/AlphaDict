import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! Root object of the QML user interface.
 *
 *  Embedded in a QQuickWidget with SizeRootObjectToView, therefore a plain
 *  Rectangle instead of an ApplicationWindow: the host widget already owns
 *  the window, the icon and the title. */
Rectangle {
    id: root

    property int currentPage: 0

    color: Theme.bg
    focus: true

    /* Follow the configured font size for every themed text. */
    Component.onCompleted: {
        Theme.baseSize = bridge.fontSize;
        searchField.forceActiveFocus();
        bridge.refreshHelpText();
    }

    Connections {
        target: bridge
        function onFontSizeChanged() { Theme.baseSize = bridge.fontSize }
        function onQueryChanged() {
            if (searchField.text !== bridge.query)
                searchField.text = bridge.query;
        }
    }

    /* ------------------------------ top bar ------------------------------ */
    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 58
        color: Theme.surface

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.border
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 10

            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                radius: 10
                gradient: Theme.accentGradient
                Text {
                    anchors.centerIn: parent
                    text: "A"
                    color: "#ffffff"
                    font.pixelSize: 19
                    font.bold: true
                }
            }

            ThemedField {
                id: searchField
                Layout.fillWidth: true
                Layout.maximumWidth: 560
                Layout.alignment: Qt.AlignVCenter
                fieldHeight: 36
                placeholderText: qsTr("Type a word and press Enter")
                onAccepted: bridge.search(text)
            }

            StyledComboBox {
                Layout.alignment: Qt.AlignVCenter
                controlWidth: 120
                model: bridge.srcLans
                currentIndex: Math.max(0, bridge.srcLans.indexOf(bridge.srcLan))
                onActivated: bridge.srcLan = bridge.srcLans[index]
            }

            Text {
                Layout.alignment: Qt.AlignVCenter
                text: "→"
                color: Theme.textMut
                font.pixelSize: Theme.baseSize
            }

            StyledComboBox {
                Layout.alignment: Qt.AlignVCenter
                controlWidth: 120
                model: bridge.detLans
                currentIndex: Math.max(0, bridge.detLans.indexOf(bridge.detLan))
                onActivated: bridge.detLan = bridge.detLans[index]
            }

            Item { Layout.fillWidth: true }

            FlatButton {
                Layout.alignment: Qt.AlignVCenter
                glyph: "🗕"
                horizontalPadding: 10
                onClicked: bridge.minimizeToTray()
            }
            FlatButton {
                Layout.alignment: Qt.AlignVCenter
                glyph: "✕"
                danger: true
                horizontalPadding: 10
                onClicked: bridge.quit()
            }
        }
    }

    /* ------------------------------ body ------------------------------ */
    RowLayout {
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: statusBar.top
        spacing: 0

        SideBar {
            Layout.fillHeight: true
            current: root.currentPage
            onNavigate: root.currentPage = index
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            StackLayout {
                id: stack
                anchors.fill: parent
                currentIndex: root.currentPage

                DictionaryPage { }
                VBookPage { }
                SettingsPage { }
                HelpPage { }
            }

            Connections {
                target: stack
                function onCurrentIndexChanged() { fadeIn.restart() }
            }
        }
    }

    /* Page switches fade in, which keeps the change of content perceptible
     *  without touching the layout of the stack. */
    NumberAnimation {
        id: fadeIn
        target: stack
        property: "opacity"
        from: 0.0
        to: 1.0
        duration: Theme.slow
        easing.type: Easing.OutCubic
        running: false
    }

    /* ------------------------------ status bar ------------------------------ */
    Rectangle {
        id: statusBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 28
        color: Theme.surface

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: Theme.border
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 8

            Rectangle {
                Layout.preferredWidth: 6
                Layout.preferredHeight: 6
                radius: 3
                color: bridge.hasDict ? Theme.success : Theme.warn
            }

            Text {
                Layout.fillWidth: true
                text: bridge.statusText !== ""
                      ? bridge.statusText
                      : (bridge.hasDict ? qsTr("Ready") : qsTr("No dictionary available"))
                color: Theme.textDim
                font.pixelSize: Theme.baseSize - 3
                elide: Text.ElideRight
            }

            Text {
                text: "AlphaDict · Qt 5 · QML"
                color: Theme.textMut
                font.pixelSize: Theme.baseSize - 4
            }
        }
    }
}
