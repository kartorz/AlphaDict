import QtQuick 2.15
import QtQuick.Layouts 1.15
import "."

/*! A rounded, bordered container with an optional header line.
 *
 *  The card is a plain column layout: whatever is declared inside it becomes a
 *  layout item right below the header.  Its implicit height is the layout's own
 *  implicit height plus the margins, so it never reads back the height it hands
 *  out -- that (a child reporting implicitHeight to the container whose height
 *  decides the child's height) is what used to make Qt Quick report
 *  "recursive rearrange" and "binding loop detected on implicitHeight".
 *
 *  Content that should grow with the card (list views, flickables) simply sets
 *  `Layout.fillHeight: true` on itself. */
Rectangle {
    id: card

    property alias title: titleText.text
    property string subtitle: ""
    property int contentMargins: Theme.pad
    property bool headerVisible: card.title !== ""

    default property alias content: layout.data

    radius: Theme.radius
    color: Theme.surface
    border.width: 1
    border.color: Theme.border
    clip: true
    implicitWidth: 240
    implicitHeight: layout.implicitHeight + contentMargins * 2

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: card.contentMargins
        spacing: 10

        RowLayout {
            id: headerRow
            Layout.fillWidth: true
            spacing: 8
            visible: card.headerVisible
            Text {
                id: titleText
                color: Theme.text
                font.pixelSize: Theme.baseSize + 1
                font.bold: true
                elide: Text.ElideRight
            }
            Item { Layout.fillWidth: true }
            Text {
                text: card.subtitle
                color: Theme.textMut
                font.pixelSize: Theme.baseSize - 1
                elide: Text.ElideRight
                /* Bounded by the card, never by headerRow: a layout item whose
                 * constraint reads its own layout's width feeds back into the
                 * RowLayout implicit width and makes QtQuick.Layouts
                 * rearrange recursively. */
                Layout.maximumWidth: Math.max(0, (card.width - card.contentMargins * 2) * 0.5)
                visible: card.subtitle !== ""
            }
        }
    }
}
