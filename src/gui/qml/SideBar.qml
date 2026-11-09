import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! Left navigation rail. */
Rectangle {
    id: bar

    property int current: 0
    signal navigate(int index)

    implicitWidth: tile + 16
    color: Theme.surface

    /* Square navigation tiles.  The tile edge is derived from the two text
     * lines so the labels fill it; the rail narrows instead of the buttons
     * growing, which leaves more room for content. */
    readonly property int tile: Math.max(60, Theme.textLine * 2 + 16)

    readonly property var navItems: [
        { glyph: "Aa", label: qsTr("Lookup") },
        { glyph: "★",  label: qsTr("Vocabulary") },
        { glyph: "⚙",  label: qsTr("Settings") },
        { glyph: "?",  label: qsTr("Help") }
    ]

    /* Hidden probe: measures the longest caption so the rail decides once for
     * all tiles.  English labels are wider than a tile, so the whole rail
     * falls back to icon-only with a hover hint instead of eliding text. */
    Text {
        id: labelProbe
        visible: false
        font.pixelSize: Theme.baseSize
        font.family: Theme.fontFamily
        text: {
            var longest = "";
            for (var i = 0; i < navItems.length; i++)
                if (navItems[i].label.length > longest.length)
                    longest = navItems[i].label;
            return longest;
        }
    }

    readonly property bool showLabels: labelProbe.implicitWidth <= tile - 6

    Rectangle {
        width: 1
        height: parent.height
        anchors.right: parent.right
        color: Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 18
        anchors.bottomMargin: 18
        spacing: 6

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: tile
            Layout.preferredHeight: tile
            radius: 14
            gradient: Theme.accentGradient
            Text {
                anchors.centerIn: parent
                text: "A"
                color: "#ffffff"
                font.pixelSize: Theme.baseSize + 14
                font.family: Theme.fontFamily
                font.bold: true
            }
        }

        Item { Layout.preferredHeight: 12 }

        Repeater {
            model: bar.navItems

            delegate: Item {
                id: navTile
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: bar.tile
                Layout.preferredHeight: bar.tile

                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusSmall
                    color: index === bar.current
                           ? Theme.selection
                           : (hover.containsMouse ? Theme.surfaceHi : Theme.surfaceAlt)
                    Behavior on color { ColorAnimation { duration: Theme.fast } }
                }

                Rectangle {
                    width: 3
                    height: 22
                    radius: 1.5
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: Theme.accent
                    opacity: index === bar.current ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: Theme.fast } }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 0
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: modelData.glyph
                        font.pixelSize: Theme.baseSize + 6
                        font.family: Theme.fontFamily
                        font.bold: true
                        color: index === bar.current ? Theme.accent : Theme.text
                    }
                    Text {
                        id: labelText
                        Layout.alignment: Qt.AlignHCenter
                        Layout.maximumWidth: bar.tile - 2
                        visible: bar.showLabels
                        text: modelData.label
                        font.pixelSize: Theme.baseSize
                        font.family: Theme.fontFamily
                        color: index === bar.current ? Theme.accent : Theme.textDim
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                MouseArea {
                    id: hover
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: bar.navigate(index)
                }

                ToolTip.visible: hover.containsMouse && !bar.showLabels
                ToolTip.delay: 600
                ToolTip.text: modelData.label
            }
        }

        Item { Layout.fillHeight: true }

        FlatButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: bar.tile
            glyph: Theme.dark ? "☀" : "☾"
            controlHeight: bar.tile
            horizontalPadding: 0
            onClicked: Theme.dark = !Theme.dark
        }
    }
}
