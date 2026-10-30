import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! The vocabulary book page: word list on the left, trainer and the
 *  explanation of the current word on the right. */
Item {
    id: page

    property int repeatCount: 1

    function submit() {
        var text = inputField.text;
        if (text.trim().length === 0)
            return;
        bridge.vbookInput(text, page.repeatCount);
        inputField.text = "";
        inputField.forceActiveFocus();
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.pad
        spacing: Theme.gutter

        /* ------------------------- word list ------------------------- */
        Card {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            title: qsTr("Vocabulary")
            subtitle: qsTr("%1 words").arg(bridge.vbook.length)

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    FlatButton {
                        text: qsTr("Study")
                        checkable: true
                        checked: bridge.vBookMode === 0
                        horizontalPadding: 10
                        controlHeight: 28
                        onClicked: bridge.vBookMode = 0
                    }
                    FlatButton {
                        text: qsTr("Test")
                        checkable: true
                        checked: bridge.vBookMode === 1
                        horizontalPadding: 10
                        controlHeight: 28
                        onClicked: bridge.vBookMode = 1
                    }

                    Item { Layout.fillWidth: true }

                    FlatButton {
                        glyph: "✕"
                        danger: true
                        controlHeight: 28
                        horizontalPadding: 8
                        enabled: bridge.vbook.length > 0
                        onClicked: bridge.clearVBook()
                    }
                }

                ListView {
                    id: wordList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 3
                    model: bridge.vbook
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ThemedScrollBar { }
                    currentIndex: bridge.vBookRow
                    onCurrentIndexChanged: if (currentIndex >= 0) positionViewAtIndex(currentIndex, ListView.Contain)

                    delegate: Rectangle {
                        width: wordList.width
                        height: 46
                        radius: Theme.radiusSmall
                        color: index === bridge.vBookRow
                               ? Theme.selection
                               : (entry.containsMouse ? Theme.surfaceHi : "transparent")

                        /* Declared first so the per row buttons stay on top. */
                        MouseArea {
                            id: entry
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: bridge.selectVBookRow(index)
                        }

                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.right: parent.right
                            anchors.rightMargin: 34
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 1
                            Text {
                                width: parent.width
                                text: modelData.word
                                color: Theme.text
                                font.pixelSize: Theme.baseSize
                                elide: Text.ElideRight
                            }
                            Text {
                                width: parent.width
                                text: modelData.complexity + "  " + modelData.date
                                color: Theme.textMut
                                font.pixelSize: Theme.baseSize - 3
                                elide: Text.ElideRight
                            }
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            text: "✕"
                            color: Theme.textMut
                            font.pixelSize: 13
                            opacity: removeArea.containsMouse ? 1.0 : 0.0
                            MouseArea {
                                id: removeArea
                                anchors.fill: parent
                                anchors.margins: -6
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: bridge.removeVBookRow(index)
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 20
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        visible: bridge.vbook.length === 0
                        text: qsTr("The vocabulary book is empty.\nAdd words from an explanation card.")
                        color: Theme.textMut
                        font.pixelSize: Theme.baseSize - 1
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    FlatButton {
                        glyph: "↑"
                        horizontalPadding: 10
                        controlHeight: 30
                        enabled: bridge.vBookRow > 0
                        onClicked: bridge.vbookStep(-1)
                    }
                    FlatButton {
                        glyph: "↓"
                        horizontalPadding: 10
                        controlHeight: 30
                        enabled: bridge.vBookRow >= 0 && bridge.vBookRow < bridge.vbook.length - 1
                        onClicked: bridge.vbookStep(1)
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: bridge.vbook.length === 0
                              ? "—"
                              : (bridge.vBookRow + 1) + " / " + bridge.vbook.length
                        color: Theme.textDim
                        font.pixelSize: Theme.baseSize - 2
                    }
                }
            }
        }

        /* ------------------------- trainer + explanation ------------------------- */
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: bridge.vBookMode === 0 ? qsTr("Study") : qsTr("Test")

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 78
                    radius: Theme.radiusSmall
                    color: Theme.surfaceAlt
                    border.width: 1
                    border.color: Theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 4
                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            /* Review mode must not leak the answer, the word is
                             * replaced by one dot per letter. */
                            text: {
                                if (bridge.vBookWord === "")
                                    return "—";
                                if (bridge.vBookMode === 0)
                                    return bridge.vBookWord;
                                var mask = "";
                                for (var i = 0; i < bridge.vBookWord.length; ++i)
                                    mask += "•";
                                return mask;
                            }
                            color: Theme.text
                            font.pixelSize: Theme.baseSize + 8
                            font.bold: true
                        }
                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            text: bridge.vBookMode === 0
                                  ? qsTr("Read the word and its explanation, then type it")
                                  : qsTr("Spell the word from its explanation, then press Enter")
                            color: Theme.textMut
                            font.pixelSize: Theme.baseSize - 2
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ThemedField {
                        id: inputField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Type the word and press Enter")
                        onAccepted: page.submit()
                    }

                    FlatButton {
                        text: qsTr("Submit")
                        primary: true
                        onClicked: page.submit()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: bridge.vBookMode === 0

                    Text {
                        text: qsTr("Repetitions")
                        color: Theme.textDim
                        font.pixelSize: Theme.baseSize - 1
                    }
                    StyledComboBox {
                        controlWidth: 80
                        controlHeight: 30
                        model: [1, 2, 3, 5]
                        currentIndex: 0
                        onActivated: page.repeatCount = [1, 2, 3, 5][index]
                    }
                    Item { Layout.fillWidth: true }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: Theme.radiusSmall
                    color: Theme.surfaceAlt
                    border.width: 1
                    border.color: Theme.border
                    clip: true

                    Flickable {
                        id: explFlick
                        anchors.fill: parent
                        anchors.margins: 12
                        contentWidth: width
                        contentHeight: explText.implicitHeight
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ThemedScrollBar { }

                        Text {
                            id: explText
                            width: explFlick.width
                            text: bridge.vBookExpl
                            textFormat: text.charAt(0) === "<" ? Text.RichText : Text.PlainText
                            color: Theme.text
                            font.pixelSize: Theme.baseSize
                            wrapMode: Text.WordWrap
                            onLinkActivated: Qt.openUrlExternally(link)
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: bridge.vBookExpl.length === 0
                        text: qsTr("Select a word on the left")
                        color: Theme.textMut
                        font.pixelSize: Theme.baseSize - 1
                    }
                }
            }
        }
    }
}
