import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! Preferences: interface, capture word and the dictionary list. */
Item {
    id: page

    /* Dragging breaks the value binding; follow the bridge and Reset. */
    Connections {
        target: bridge
        function onFontSizeChanged() { fontSlider.value = bridge.fontSize }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: Theme.pad
        contentWidth: width
        contentHeight: column.height
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ThemedScrollBar { }

        ColumnLayout {
            id: column
            /* The column owns its height and the viewport only scrolls it;
             * binding this back to the Flickable's contentHeight loops. */
            height: implicitHeight
            /* Leave the scroll bar its eight pixels. */
            width: flick.width - 12
            spacing: Theme.gutter

            /* ---------------- interface ---------------- */
            Card {
                title: qsTr("Interface")
                Layout.fillWidth: true

                ColumnLayout {
                    id: interfaceColumn
                    Layout.fillWidth: true
                    spacing: 10

                    SettingRow {
                        label: qsTr("System tray")
                        hint: qsTr("Hide the window in the tray instead of quitting")
                        ThemedSwitch {
                            checked: bridge.systemTray
                            onToggled: bridge.systemTray = value
                        }
                    }

                    SettingRow {
                        label: qsTr("Interface language")
                        hint: qsTr("Restart the application to apply the change")
                        StyledComboBox {
                            model: bridge.uiLans
                            controlWidth: Math.max(120, Theme.baseSize * 7)
                            currentIndex: bridge.uiLanID
                            onActivated: bridge.setUiLanID(index)
                        }
                    }

                    SettingRow {
                        label: qsTr("Text size")
                        hint: qsTr("Applies to the whole interface right away")
                        RowLayout {
                            spacing: 8
                            Slider {
                                id: fontSlider
                                from: bridge.minFontSize
                                to: bridge.maxFontSize
                                stepSize: 1
                                value: bridge.fontSize
                                implicitWidth: Math.max(170, Theme.baseSize * 9)
                                onMoved: bridge.fontSize = Math.round(value)
                                background: Rectangle {
                                    x: fontSlider.leftPadding
                                    y: fontSlider.topPadding + fontSlider.availableHeight / 2 - height / 2
                                    width: fontSlider.availableWidth
                                    height: 4
                                    radius: 2
                                    color: Theme.track
                                    Rectangle {
                                        width: fontSlider.visualPosition * parent.width
                                        height: parent.height
                                        radius: 2
                                        color: Theme.accent
                                    }
                                }
                                handle: Rectangle {
                                    x: fontSlider.leftPadding + fontSlider.visualPosition * (fontSlider.availableWidth - width)
                                    y: fontSlider.topPadding + fontSlider.availableHeight / 2 - height / 2
                                    width: 16
                                    height: 16
                                    radius: 8
                                    color: Theme.surface
                                    border.width: 2
                                    border.color: Theme.accent
                                }
                            }
                            Text {
                                text: bridge.fontSize + " px"
                                color: Theme.textDim
                                font.pixelSize: Theme.smallSize
                                font.family: Theme.fontFamily
                                Layout.preferredWidth: Math.max(52, Theme.baseSize * 2 + 18)
                            }
                        }
                    }

                    SettingRow {
                        label: qsTr("Interface font")
                        hint: qsTr("Leave empty to use the system default font")
                        ThemedField {
                            id: fontField
                            implicitWidth: Math.max(200, Theme.baseSize * 12)
                            fieldHeight: Math.max(30, Theme.baseSize + 18)
                            text: bridge.fontFamily
                            placeholderText: qsTr("System default")
                            onEditingFinished: bridge.fontFamily = text
                        }
                    }
                }
            }

            /* ---------------- capture word ---------------- */
            Card {
                title: qsTr("Capture word")
                Layout.fillWidth: true

                ColumnLayout {
                    id: cwsColumn
                    Layout.fillWidth: true
                    spacing: 10

                    SettingRow {
                        label: qsTr("Enable capture")
                        hint: qsTr("Show the explanation when text is copied or selected")
                        ThemedSwitch {
                            checked: bridge.cwsEnabled
                            onToggled: bridge.cwsEnabled = value
                        }
                    }

                    SettingRow {
                        label: qsTr("Capture from clipboard")
                        hint: qsTr("Show the explanation when text is copied")
                        ThemedSwitch {
                            checked: bridge.cwsClipboard
                            enabled: bridge.cwsEnabled
                            onToggled: bridge.cwsClipboard = value
                        }
                    }

                    SettingRow {
                        label: qsTr("Capture from selection")
                        hint: qsTr("Read the X11 primary selection")
                        ThemedSwitch {
                            checked: bridge.cwsSelection
                            enabled: bridge.cwsEnabled
                            onToggled: bridge.cwsSelection = value
                        }
                    }

                    SettingRow {
                        label: qsTr("Close the capture window automatically")
                        hint: qsTr("Hide short explanations after a moment")
                        ThemedSwitch {
                            checked: bridge.autoCloseEn
                            enabled: bridge.cwsEnabled
                            onToggled: bridge.autoCloseEn = value
                        }
                    }

                    SettingRow {
                        label: qsTr("Capture shortcut")
                        hint: qsTr("Ctrl+Alt+ the selected letter toggles capture temporarily")
                        RowLayout {
                            spacing: 6
                            Text {
                                text: "Ctrl + Alt +"
                                color: Theme.textDim
                                font.pixelSize: Theme.baseSize - 1
                                font.family: Theme.fontFamily
                            }
                            StyledComboBox {
                                model: bridge.shortcutKeys
                                controlWidth: Math.max(70, Theme.baseSize * 4)
                                currentIndex: bridge.shortcutIndex
                                onActivated: bridge.shortcutIndex = index
                            }
                        }
                    }
                }
            }

            /* ---------------- dictionaries ---------------- */
            Card {
                title: qsTr("Dictionaries")
                subtitle: qsTr("%1 installed").arg(bridge.dictCount)
                Layout.fillWidth: true

                ColumnLayout {
                    id: dictColumn
                    Layout.fillWidth: true
                    spacing: 8

                    /* A Repeater, not a nested list: a handful of rows simply
                     * grow with the font and the page scrolls. */
                    Repeater {
                        model: bridge.dictModel

                        delegate: Rectangle {
                            Layout.fillWidth: true
                            /* Keep the padding, never let the text fill the row. */
                            implicitHeight: Math.max(56, rowBox.implicitHeight + Theme.pad * 2)
                            radius: Theme.radiusSmall
                            color: Theme.surfaceAlt
                            border.width: 1
                            border.color: Theme.border

                            RowLayout {
                                id: rowBox
                                anchors.fill: parent
                                anchors.leftMargin: Theme.pad + 2
                                anchors.rightMargin: Theme.pad
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text {
                                        text: model.name
                                        color: Theme.text
                                        font.pixelSize: Theme.baseSize
                                        font.family: Theme.fontFamily
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        /* One "key: value" per line; flatten it
                                         * into a middot run. */
                                        text: String(model.summary)
                                              .split(/[\r\n]+/)
                                              .map(function (part) { return part.trim(); })
                                              .filter(function (part) { return part.length > 0; })
                                              .join("  ·  ")
                                        color: Theme.textMut
                                        font.pixelSize: Theme.smallSize
                                        font.family: Theme.fontFamily
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                        Layout.fillWidth: true
                                    }
                                }

                                FlatButton {
                                    glyph: "↑"
                                    controlHeight: Theme.controlHeight
                                    horizontalPadding: 8
                                    enabled: index > 0
                                    onClicked: bridge.moveDictRow(index, -1)
                                }
                                FlatButton {
                                    glyph: "↓"
                                    controlHeight: Theme.controlHeight
                                    horizontalPadding: 8
                                    enabled: index < bridge.dictCount - 1
                                    onClicked: bridge.moveDictRow(index, 1)
                                }
                                ThemedSwitch {
                                    checked: model.enabled
                                    onToggled: bridge.setDictEnabled(index, value)
                                }
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        visible: bridge.dictCount === 0
                        text: qsTr("No dictionary found. Put dictionary files into:\n%1").arg(bridge.dataDir)
                        color: Theme.textMut
                        font.pixelSize: Theme.baseSize - 1
                        font.family: Theme.fontFamily
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                /* Keep the button clear of the scroll bar. */
                Layout.rightMargin: 4
                Item { Layout.fillWidth: true }
                FlatButton {
                    text: qsTr("Restore defaults")
                    danger: true
                    onClicked: bridge.resetSettings()
                }
            }

            /* keeps the last row clear of the status bar when scrolled down */
            Item { Layout.preferredHeight: Theme.pad * 2 }
        }
    }
}
