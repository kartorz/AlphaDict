import QtQuick 2.15
import "."

/*! Small switch, drawn from scratch so it follows the theme exactly. */
Item {
    id: sw

    property bool checked: false
    signal toggled(bool value)

    implicitWidth: 42
    implicitHeight: 24
    opacity: enabled ? 1.0 : 0.45

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: sw.checked ? Theme.accent : Theme.track
        border.width: 1
        border.color: sw.checked ? Theme.accent : Theme.border
        Behavior on color { ColorAnimation { duration: Theme.fast } }

        Rectangle {
            width: 18
            height: 18
            radius: 9
            y: 2
            x: sw.checked ? parent.width - width - 3 : 3
            color: "#ffffff"
            Behavior on x {
                NumberAnimation { duration: Theme.fast; easing.type: Easing.OutCubic }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        enabled: sw.enabled
        onClicked: sw.toggled(!sw.checked)
    }
}
