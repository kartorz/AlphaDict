import QtQuick 2.15
import QtQuick.Controls 2.15
import "."

/*! Single line editor matching the theme. */
TextField {
    id: field

    property int fieldHeight: 34

    implicitHeight: fieldHeight
    color: Theme.text
    placeholderTextColor: Theme.textMut
    font.pixelSize: Theme.baseSize
    selectByMouse: true
    leftPadding: 12
    rightPadding: 12
    verticalAlignment: TextInput.AlignVCenter
    selectionColor: Theme.accent
    selectedTextColor: "#ffffff"

    background: Rectangle {
        radius: Theme.radiusSmall
        color: Theme.surfaceAlt
        border.width: 1
        border.color: field.activeFocus ? Theme.accent : Theme.border
        Behavior on border.color { ColorAnimation { duration: Theme.fast } }
    }
}
