import QtQuick 2.15
import QtQuick.Controls 2.15
import "."

/*! ComboBox restyled for the theme, so the look does not depend on the
 *  active Qt Quick Controls style. */
ComboBox {
    id: cb

    property int controlHeight: 34
    property int controlWidth: 96

    implicitHeight: controlHeight
    implicitWidth: controlWidth
    font.pixelSize: Theme.baseSize

    contentItem: Text {
        leftPadding: 12
        rightPadding: 24
        text: cb.displayText
        color: Theme.text
        font: cb.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        x: cb.width - 20
        y: (cb.height - height) / 2
        text: "▾"
        color: Theme.textDim
        font.pixelSize: 12
    }

    background: Rectangle {
        radius: Theme.radiusSmall
        color: cb.hovered ? Theme.surfaceHi : Theme.surfaceAlt
        border.width: 1
        border.color: cb.activeFocus ? Theme.accent : Theme.border
        Behavior on color { ColorAnimation { duration: Theme.fast } }
    }

    delegate: ItemDelegate {
        id: entry
        width: cb.width
        height: 30
        highlighted: cb.highlightedIndex === index
        contentItem: Text {
            leftPadding: 10
            text: modelData === undefined || modelData === null ? "" : ("" + modelData)
            color: Theme.text
            font.pixelSize: Theme.baseSize
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: 6
            color: entry.highlighted || entry.hovered ? Theme.selection : "transparent"
        }
    }

    popup: Popup {
        y: cb.height + 4
        width: cb.width
        implicitHeight: Math.min(320, contentItem.implicitHeight + 10)
        padding: 5
        background: Rectangle {
            radius: Theme.radiusSmall
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
        }
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: cb.delegateModel
            currentIndex: cb.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds
        }
    }
}
