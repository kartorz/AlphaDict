import QtQuick 2.15
import QtQuick.Layouts 1.15
import "."

/*! Left navigation rail. */
Rectangle {
    id: bar

    property int current: 0
    signal navigate(int index)

    implicitWidth: Math.max(96, Theme.baseSize * 4 + 40)
    color: Theme.surface

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
            Layout.preferredWidth: Math.max(44, Theme.baseSize + 28)
            Layout.preferredHeight: Math.max(44, Theme.baseSize + 28)
            radius: 14
            gradient: Theme.accentGradient
            Text {
                anchors.centerIn: parent
                text: "A"
                color: "#ffffff"
                font.pixelSize: Math.max(24, Theme.baseSize + 8)
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
                Layout.preferredWidth: Math.max(72, Theme.baseSize * 4 + 16)
                /* Must fit the glyph line plus the label line, so grow with both. */
                Layout.preferredHeight: Math.max(58, Theme.baseSize * 2 + 14)

                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusSmall
                    color: index === bar.current
                           ? Theme.selection
                           : (hover.containsMouse ? Theme.surfaceHi : "transparent")
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
                    spacing: 2
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: modelData.glyph
                        font.pixelSize: Math.max(18, Theme.baseSize + 2)
                        font.family: Theme.fontFamily
                        font.bold: true
                        color: index === bar.current ? Theme.accent : Theme.text
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.maximumWidth: Math.max(60, Theme.baseSize * 4 + 8)
                        text: modelData.label
                        font.pixelSize: Theme.tinySize
                        font.family: Theme.fontFamily
                        color: index === bar.current ? Theme.accent : Theme.textMut
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
            Layout.preferredWidth: 46
            glyph: Theme.dark ? "☀" : "☾"
            controlHeight: Math.max(40, Theme.baseSize + 26)
            horizontalPadding: 0
            onClicked: Theme.dark = !Theme.dark
        }
    }
}
