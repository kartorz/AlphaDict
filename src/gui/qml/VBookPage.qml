import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! The vocabulary book page: word list on the left, trainer and the
 *  explanation of the current word on the right. */
Item {
    id: page

    property int repeatCount: 1
    /* 0 无判定, 1 拼写正确, -1 拼写错误; 用于输入框边框和提示行 */
    property int verdict: 0

    /* 切到这一页时直接聚焦拼写框 */
    onVisibleChanged: if (visible) inputField.forceActiveFocus()

    Timer {
        id: verdictTimer
        interval: 1600
        onTriggered: page.verdict = 0
    }

    function submit() {
        var text = inputField.text.trim();
        if (text.length === 0)
            return;

        var target = bridge.vBookWord;
        page.verdict = (target !== "" && text.toLowerCase() === target.toLowerCase()) ? 1 : -1;
        verdictTimer.restart();

        bridge.vbookInput(text, page.repeatCount);

        if (page.verdict === 1)
            inputField.text = "";
        else
            inputField.selectAll();
        inputField.forceActiveFocus();
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.pad
        spacing: Theme.gutter

        /* ------------------------- word list ------------------------- */
        Card {
            Layout.preferredWidth: Math.max(300, Theme.baseSize * 16)
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
                        controlHeight: Theme.controlHeight
                        onClicked: bridge.vBookMode = 0
                    }
                    FlatButton {
                        text: qsTr("Test")
                        checkable: true
                        checked: bridge.vBookMode === 1
                        horizontalPadding: 10
                        controlHeight: Theme.controlHeight
                        onClicked: bridge.vBookMode = 1
                    }

                    Item { Layout.fillWidth: true }

                    FlatButton {
                        glyph: "✕"
                        danger: true
                        controlHeight: Theme.controlHeight
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
                        height: Math.max(46, Theme.baseSize + 30)
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
                                font.family: Theme.fontFamily
                                elide: Text.ElideRight
                            }
                            Text {
                                width: parent.width
                                text: modelData.complexity + "  " + modelData.date
                                color: Theme.textMut
                                font.pixelSize: Theme.tinySize
                                font.family: Theme.fontFamily
                                elide: Text.ElideRight
                            }
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            text: "✕"
                            color: Theme.textMut
                            font.pixelSize: Theme.smallSize
                            font.family: Theme.fontFamily
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
                        font.family: Theme.fontFamily
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    FlatButton {
                        glyph: "↑"
                        horizontalPadding: 10
                        controlHeight: Theme.controlHeight
                        enabled: bridge.vBookRow > 0
                        onClicked: bridge.vbookStep(-1)
                    }
                    FlatButton {
                        glyph: "↓"
                        horizontalPadding: 10
                        controlHeight: Theme.controlHeight
                        enabled: bridge.vBookRow >= 0 && bridge.vBookRow < bridge.vbook.length - 1
                        onClicked: bridge.vbookStep(1)
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: bridge.vbook.length === 0
                              ? "—"
                              : (bridge.vBookRow + 1) + " / " + bridge.vbook.length
                        color: Theme.textDim
                        font.pixelSize: Theme.smallSize
                        font.family: Theme.fontFamily
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
                spacing: 8

                /* 目标词：学习模式显示单词，测试模式只显示点数，不能泄题 */
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(64, Theme.baseSize * 2 + 28)
                    radius: Theme.radiusSmall
                    color: Theme.surfaceAlt
                    border.width: 1
                    border.color: Theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
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
                            font.pixelSize: Theme.baseSize + 10
                            font.family: Theme.fontFamily
                            font.bold: true
                        }

                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            text: bridge.vBookMode === 0
                                  ? qsTr("Type the word to practise its spelling")
                                  : qsTr("Type the word that matches the explanation")
                            color: Theme.textMut
                            font.pixelSize: Theme.smallSize
                            font.family: Theme.fontFamily
                        }
                    }
                }

                /* 大号拼写框：反复输入，回车即判定 */
                ThemedField {
                    id: inputField
                    Layout.fillWidth: true
                    fieldHeight: Math.round(Theme.controlHeight * 1.9)
                    font.pixelSize: Theme.baseSize + 8
                    horizontalAlignment: TextInput.AlignHCenter
                    verdict: page.verdict
                    placeholderText: qsTr("Spell the word here, then press Enter")
                    onAccepted: page.submit()
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: page.verdict === 1 ? qsTr("✔ Correct")
                            : page.verdict === -1 ? qsTr("✘ Try again")
                            : qsTr("Press Enter to check your spelling")
                        color: page.verdict === 1 ? Theme.success
                             : page.verdict === -1 ? Theme.danger
                             : Theme.textMut
                        font.pixelSize: Theme.smallSize
                        font.family: Theme.fontFamily
                    }

                    FlatButton {
                        text: qsTr("Submit")
                        primary: true
                        controlHeight: Theme.controlHeight
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
                        font.family: Theme.fontFamily
                    }
                    StyledComboBox {
                        controlWidth: Math.max(80, Theme.baseSize * 4)
                        controlHeight: Theme.controlHeight
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
                            font.family: Theme.fontFamily
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
                        font.family: Theme.fontFamily
                    }
                }
            }
        }
    }
}
