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
        Theme.fontFamily = bridge.fontFamily;
        bridge.darkTheme = Theme.dark;
        searchField.forceActiveFocus();
        bridge.refreshHelpText();
    }

    Connections {
        target: Theme
        function onDarkChanged() { bridge.darkTheme = Theme.dark }
    }

    Connections {
        target: bridge
        function onFontSizeChanged() { Theme.baseSize = bridge.fontSize }
        function onFontFamilyChanged() { Theme.fontFamily = bridge.fontFamily }
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
        height: Theme.toolbarHeight
        color: Theme.surface

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.border
        }

        /* Drag layer *below* the controls: buttons/fields win the press,
         * only empty bar areas drag the window. */
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onPressed: bridge.beginWindowMove()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: Math.max(32, Theme.textLine + 6)
                Layout.preferredHeight: Math.max(32, Theme.textLine + 6)
                radius: 10
                gradient: Theme.accentGradient
                Text {
                    anchors.centerIn: parent
                    text: "A"
                    color: "#ffffff"
                    font.pixelSize: Theme.baseSize + 6
                    font.family: Theme.fontFamily
                    font.bold: true
                }
            }

            ThemedField {
                id: searchField
                Layout.fillWidth: true
                Layout.maximumWidth: Math.max(900, Theme.baseSize * 40)
                Layout.alignment: Qt.AlignVCenter
                fieldHeight: Math.max(36, Theme.controlHeight + 9)
                placeholderText: qsTr("Type a word and press Enter")
                onAccepted: bridge.search(text)
            }

            /* No language combos: the input decides. */

            FlatButton {
                id: starButton
                /* Acts on what the user typed: the button must work before
                 * Enter is pressed, so it does not use bridge.query. */
                readonly property string word: searchField.text.trim()
                readonly property bool marked: {
                    var list = bridge.vbook;   // dependency: refresh on book change
                    return bridge.inVBook(word);
                }
                Layout.alignment: Qt.AlignVCenter
                glyph: marked ? "★" : "☆"
                primary: marked
                enabled: word !== ""
                controlHeight: Math.max(32, Theme.controlHeight + 4)
                horizontalPadding: 10
                onClicked: bridge.toggleVBook(word)
                ToolTip.visible: hovered
                ToolTip.delay: 600
                ToolTip.text: marked ? qsTr("Remove from vocabulary book")
                                     : qsTr("Add to vocabulary book")
            }
            FlatButton {
                id: minButton
                Layout.alignment: Qt.AlignVCenter
                glyph: "🗕"
                controlHeight: Math.max(32, Theme.controlHeight + 4)
                horizontalPadding: 10
                onClicked: bridge.minimizeToTray()
            }
            FlatButton {
                id: closeButton
                Layout.alignment: Qt.AlignVCenter
                glyph: "✕"
                danger: true
                controlHeight: Math.max(32, Theme.controlHeight + 4)
                horizontalPadding: 10
                onClicked: bridge.closeWindow()
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
        height: Theme.statusHeight
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
                font.pixelSize: Theme.tinySize
                font.family: Theme.fontFamily
                elide: Text.ElideRight
            }

            Text {
                /* version, theme and the dictionary actually loaded: more
                 * useful than the toolkit fingerprint that was here. */
                text: {
                    var parts = ["AlphaDict " + bridge.appVersion,
                                 Theme.dark ? qsTr("Dark") : qsTr("Light")];
                    if (bridge.dictInfo !== "")
                        parts.push(bridge.dictInfo);
                    return parts.join("  ·  ");
                }
                color: Theme.textMut
                font.pixelSize: Theme.tinySize
                font.family: Theme.fontFamily
            }
        }
    }

    /* --------------------------- resize border --------------------------- */
    /* A frameless window loses the WM grip; these strips hand the resize
     * back.  Edges are Qt::LeftEdge=1, TopEdge=2, RightEdge=4, BottomEdge=8. */
    MouseArea {
        width: 6; anchors.left: parent.left; anchors.top: parent.top
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeHorCursor
        onPressed: bridge.beginWindowResize(1)
    }
    MouseArea {
        width: 6; anchors.right: parent.right; anchors.top: parent.top
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeHorCursor
        onPressed: bridge.beginWindowResize(4)
    }
    MouseArea {
        height: 6; anchors.top: parent.top; anchors.left: parent.left
        anchors.right: parent.right
        cursorShape: Qt.SizeVerCursor
        onPressed: bridge.beginWindowResize(2)
    }
    MouseArea {
        height: 6; anchors.bottom: parent.bottom; anchors.left: parent.left
        anchors.right: parent.right
        cursorShape: Qt.SizeVerCursor
        onPressed: bridge.beginWindowResize(8)
    }
    MouseArea {
        width: 12; height: 12; anchors.left: parent.left; anchors.top: parent.top
        cursorShape: Qt.SizeFDiagCursor
        onPressed: bridge.beginWindowResize(3)
    }
    MouseArea {
        width: 12; height: 12; anchors.right: parent.right; anchors.top: parent.top
        cursorShape: Qt.SizeBDiagCursor
        onPressed: bridge.beginWindowResize(6)
    }
    MouseArea {
        width: 12; height: 12; anchors.left: parent.left; anchors.bottom: parent.bottom
        cursorShape: Qt.SizeBDiagCursor
        onPressed: bridge.beginWindowResize(9)
    }
    MouseArea {
        width: 12; height: 12; anchors.right: parent.right; anchors.bottom: parent.bottom
        cursorShape: Qt.SizeFDiagCursor
        onPressed: bridge.beginWindowResize(12)
    }
}
