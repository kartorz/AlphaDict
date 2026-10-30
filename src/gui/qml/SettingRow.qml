import QtQuick 2.15
import QtQuick.Layouts 1.15
import "."

/*! One settings line: label (with optional hint) on the left, control on the
 *  right.  The control is the default property. */
RowLayout {
    id: row

    property string label: ""
    property string hint: ""

    default property alias control: slot.data

    Layout.fillWidth: true
    spacing: 12

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 2
        Text {
            text: row.label
            color: Theme.text
            font.pixelSize: Theme.baseSize
        }
        Text {
            visible: row.hint !== ""
            text: row.hint
            color: Theme.textMut
            font.pixelSize: Theme.baseSize - 2
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    Item {
        id: slot
        /* TextField and friends do not report a usable childrenRect, their
         * implicit size only lives on the child itself. */
        implicitWidth: children.length > 0 ? children[0].implicitWidth : 0
        implicitHeight: children.length > 0 ? children[0].implicitHeight : 0
        Layout.preferredWidth: implicitWidth
        Layout.preferredHeight: implicitHeight
    }
}
