import QtQuick 2.15
import QtQuick.Layouts 1.15
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
            model: [
                { glyph: "Aa", label: qsTr("Lookup") },
                { glyph: "★",  label: qsTr("Vocabulary") },
                { glyph: "⚙",  label: qsTr("Settings") },
                { glyph: "?",  label: qsTr("Help") }
            ]

            delegate: Item {
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
                        Layout.alignment: Qt.AlignHCenter
                        Layout.maximumWidth: bar.tile - 2
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
