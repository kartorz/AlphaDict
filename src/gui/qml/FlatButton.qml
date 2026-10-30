import QtQuick 2.15
import "."

/*! Flat, theme aware button.  `primary` uses the accent colour, `danger` the
 *  red one, everything else sits on the surface. */
Rectangle {
    id: btn

    property string text: ""
    property string glyph: ""
    property bool primary: false
    property bool danger: false
    property bool checkable: false
    property bool checked: false
    property int horizontalPadding: 14
    property int controlHeight: 34

    signal clicked()

    readonly property bool hovered: mouse.containsMouse
    readonly property color fg: !enabled ? Theme.textMut
                                : ((primary || danger) ? "#ffffff" : Theme.text)

    implicitHeight: controlHeight
    implicitWidth: Math.max(58, inner.implicitWidth + horizontalPadding * 2)
    radius: Theme.radiusSmall
    /* Disabled buttons must still read as buttons: dim the fill and the label a
     * little, but keep the outline and enough contrast to stay legible. */
    opacity: enabled ? 1.0 : 0.7

    color: {
        if (danger)
            return hovered ? Qt.lighter(Theme.danger, 1.12) : Theme.danger;
        if (primary)
            return hovered ? Qt.lighter(Theme.accent, 1.12) : Theme.accent;
        if (checkable && checked)
            return Theme.selection;
        return hovered ? Theme.surfaceHi : Theme.surfaceAlt;
    }
    border.width: 1
    border.color: (primary || danger) ? "transparent" : Theme.border
    Behavior on color { ColorAnimation { duration: Theme.fast } }

    Row {
        id: inner
        anchors.centerIn: parent
        spacing: 6
        Text {
            text: btn.glyph
            visible: btn.glyph !== ""
            color: btn.fg
            font.pixelSize: Theme.baseSize + 1
        }
        Text {
            text: btn.text
            visible: btn.text !== ""
            color: btn.fg
            font.pixelSize: Theme.baseSize
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        enabled: btn.enabled
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (btn.checkable)
                btn.checked = !btn.checked;
            btn.clicked();
        }
    }
}
