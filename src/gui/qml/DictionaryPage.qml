import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! Lookup page: index on the left, the explanation cards on the right. */
Item {
    id: page

    /*! Ask the bridge for the index of the given prefix. */
    function loadPrefix(prefix) {
        bridge.prefixSearch(prefix);
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.pad
        spacing: Theme.gutter

        /* ------------------------- index ------------------------- */
        Card {
            Layout.preferredWidth: 230
            Layout.fillHeight: true
            title: qsTr("Index")
            subtitle: qsTr("%1 items").arg(bridge.indexCount)

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                ThemedField {
                    id: prefixField
                    Layout.fillWidth: true
                    fieldHeight: 30
                    placeholderText: qsTr("Prefix, e.g. ab")
                    onTextChanged: page.loadPrefix(text)
                }

                ListView {
                    id: indexList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 1
                    model: bridge.indexModel
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ThemedScrollBar { }

                    delegate: Rectangle {
                        width: indexList.width
                        height: 28
                        radius: 6
                        color: entry.containsMouse ? Theme.surfaceHi : "transparent"

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.display
                            color: entry.containsMouse ? Theme.accent : Theme.text
                            font.pixelSize: Theme.baseSize - 1
                            elide: Text.ElideRight
                        }

                        MouseArea {
                            id: entry
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: bridge.clickIndex(index)
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 16
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        visible: bridge.indexCount === 0
                        text: qsTr("No index")
                        color: Theme.textMut
                        font.pixelSize: Theme.baseSize - 2
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FlatButton {
                        glyph: "⏮"
                        horizontalPadding: 8
                        controlHeight: 26
                        onClicked: {
                            var row = bridge.pageIndex(-1);
                            if (row >= 0) indexList.positionViewAtIndex(row, ListView.Beginning);
                        }
                    }
                    Item { Layout.fillWidth: true }
                    FlatButton {
                        glyph: "⏭"
                        horizontalPadding: 8
                        controlHeight: 26
                        onClicked: {
                            var row = bridge.pageIndex(1);
                            if (row >= 0) indexList.positionViewAtIndex(row, ListView.Beginning);
                        }
                    }
                }
            }
        }

        /* ------------------------- explanation ------------------------- */
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: bridge.query !== "" ? bridge.query : qsTr("Explanation")
            subtitle: qsTr("%1 results").arg(bridge.expl.length)

            Flickable {
                id: explFlick
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width
                contentHeight: explColumn.implicitHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ThemedScrollBar { }

                ColumnLayout {
                    id: explColumn
                    width: explFlick.width
                    spacing: 10

                    Repeater {
                        model: bridge.expl

                        delegate: Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: entryColumn.implicitHeight + 22
                            radius: Theme.radiusSmall
                            color: Theme.surfaceAlt
                            border.width: 1
                            border.color: Theme.border

                            ColumnLayout {
                                id: entryColumn
                                anchors.fill: parent
                                anchors.margins: 11
                                spacing: 6

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Text {
                                        text: modelData.word
                                        color: Theme.text
                                        font.pixelSize: Theme.baseSize + 3
                                        font.bold: true
                                    }
                                    Text {
                                        visible: modelData.phonetic !== ""
                                        // AldConvert stores one phonetic per line
                                        // (British / American); render them inline.
                                        text: "/" + String(modelData.phonetic)
                                                  .replace(/[\r\n]+/g, "    ")
                                                  .trim() + "/"
                                        color: Theme.textDim
                                        font.pixelSize: Theme.baseSize - 1
                                    }
                                    Item { Layout.fillWidth: true }
                                    Rectangle {
                                        visible: modelData.dictname !== ""
                                        implicitWidth: dictTag.implicitWidth + 14
                                        implicitHeight: 20
                                        radius: 10
                                        color: Theme.selection
                                        Text {
                                            id: dictTag
                                            anchors.centerIn: parent
                                            text: modelData.dictname
                                            color: Theme.accent
                                            font.pixelSize: Theme.baseSize - 4
                                        }
                                    }
                                    FlatButton {
                                        glyph: "★"
                                        text: qsTr("Vocabulary")
                                        horizontalPadding: 9
                                        controlHeight: 26
                                        enabled: modelData.found
                                        onClicked: bridge.addToVBook(modelData.word)
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.found
                                          ? modelData.expl
                                          : qsTr("Entry not found")
                                    textFormat: text.charAt(0) === "<" ? Text.RichText : Text.PlainText
                                    color: modelData.found ? Theme.text : Theme.textMut
                                    font.pixelSize: Theme.baseSize
                                    wrapMode: Text.WordWrap
                                    onLinkActivated: Qt.openUrlExternally(link)
                                }
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        visible: bridge.expl.length === 0
                        text: bridge.query === ""
                              ? qsTr("Type a word above and press Enter")
                              : (bridge.hasDict ? qsTr("No results")
                                                : qsTr("No dictionary available"))
                        color: Theme.textMut
                        font.pixelSize: Theme.baseSize
                        topPadding: 40
                    }

                    Item { Layout.preferredHeight: 6 }
                }
            }
        }
    }
}
