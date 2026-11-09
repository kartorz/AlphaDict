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
            Layout.preferredWidth: Math.max(230, Theme.baseSize * 13)
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
                    fieldHeight: Math.max(30, Theme.baseSize + 18)
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
                        height: Math.max(24, Theme.textLine + 4)
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
                            font.family: Theme.fontFamily
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
                        font.pixelSize: Theme.smallSize
                        font.family: Theme.fontFamily
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FlatButton {
                        glyph: "⏮"
                        horizontalPadding: 8
                        controlHeight: Theme.controlHeight
                        onClicked: {
                            var row = bridge.pageIndex(-1);
                            if (row >= 0) indexList.positionViewAtIndex(row, ListView.Beginning);
                        }
                    }
                    Item { Layout.fillWidth: true }
                    FlatButton {
                        glyph: "⏭"
                        horizontalPadding: 8
                        controlHeight: Theme.controlHeight
                        onClicked: {
                            var row = bridge.pageIndex(1);
                            if (row >= 0) indexList.positionViewAtIndex(row, ListView.Beginning);
                        }
                    }
                }
            }
        }

        /* ------------------------- explanation ------------------------- */
        /* Result cards without a header: a lookup usually returns one or two
         * dictionaries, so the headword plus result count only duplicated the
         * entry itself. */
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true

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
                                        font.family: Theme.fontFamily
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
                                        font.family: Theme.fontFamily
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
                                    font.family: Theme.fontFamily
                                    wrapMode: Text.WordWrap
                                    onLinkActivated: Qt.openUrlExternally(link)
                                }

                                /* Dictionary name belongs to the entry, so it sits
                                 * at its own bottom-right line instead of competing
                                 * with the headword for space. */
                                Text {
                                    Layout.alignment: Qt.AlignRight
                                    Layout.maximumWidth: entryColumn.width
                                    visible: modelData.dictname !== ""
                                    text: modelData.dictname
                                    color: Theme.textMut
                                    font.pixelSize: Theme.tinySize
                                    font.family: Theme.fontFamily
                                    elide: Text.ElideRight
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
                        font.family: Theme.fontFamily
                        topPadding: 40
                    }

                    Item { Layout.preferredHeight: 6 }
                }
            }
        }
    }
}
