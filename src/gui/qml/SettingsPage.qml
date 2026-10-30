import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "."

/*! Preferences: interface, capture word and the dictionary list. */
Item {
    id: page

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: Theme.pad
        contentWidth: width
        contentHeight: column.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ThemedScrollBar { }

        ColumnLayout {
            id: column
            width: flick.width
            spacing: Theme.gutter

            /* ---------------- interface ---------------- */
            Card {
                title: qsTr("Interface")
                Layout.fillWidth: true

                ColumnLayout {
                    id: interfaceColumn
                    Layout.fillWidth: true
                    spacing: 12

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
                            controlWidth: 120
                            currentIndex: bridge.uiLanID
                            onActivated: bridge.setUiLanID(index)
                        }
                    }

                    SettingRow {
                        label: qsTr("Text size")
                        hint: qsTr("Affects explanations and the vocabulary book")
                        RowLayout {
                            spacing: 8
                            Slider {
                                id: fontSlider
                                from: 9
                                to: 32
                                stepSize: 1
                                value: bridge.fontSize
                                implicitWidth: 170
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
                                text: bridge.fontSize + " pt"
                                color: Theme.textDim
                                font.pixelSize: Theme.baseSize - 2
                                Layout.preferredWidth: 46
                            }
                        }
                    }

                    SettingRow {
                        label: qsTr("Interface font")
                        hint: qsTr("Leave empty to use the system default font")
                        ThemedField {
                            id: fontField
                            implicitWidth: 200
                            fieldHeight: 30
                            text: bridge.fontFamily
                            placeholderText: qsTr("System default")
                            onAccepted: bridge.fontFamily = text
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
                    spacing: 12

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
                            }
                            StyledComboBox {
                                model: bridge.shortcutKeys
                                controlWidth: 70
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

                    ListView {
                        id: dictList
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(360, Math.max(60,
                            bridge.dictCount * 52))
                        clip: true
                        spacing: 4
                        model: bridge.dictModel
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ThemedScrollBar { }

                        delegate: Rectangle {
                            width: dictList.width
                            height: 48
                            radius: Theme.radiusSmall
                            color: Theme.surfaceAlt
                            border.width: 1
                            border.color: Theme.border

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 8
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 1
                                    Text {
                                        text: model.name
                                        color: Theme.text
                                        font.pixelSize: Theme.baseSize
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        /* AldConvert writes the metadata as one
                                         * "key: value" per line; flatten it into a
                                         * single middot separated run. */
                                        text: String(model.summary)
                                              .split(/[\r\n]+/)
                                              .map(function (part) { return part.trim(); })
                                              .filter(function (part) { return part.length > 0; })
                                              .join("  ·  ")
                                        color: Theme.textMut
                                        font.pixelSize: Theme.baseSize - 2
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                        Layout.fillWidth: true
                                    }
                                }

                                FlatButton {
                                    glyph: "↑"
                                    controlHeight: 26
                                    horizontalPadding: 8
                                    enabled: index > 0
                                    onClicked: bridge.moveDictRow(index, -1)
                                }
                                FlatButton {
                                    glyph: "↓"
                                    controlHeight: 26
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

                        Text {
                            anchors.centerIn: parent
                            width: parent.width - 24
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            visible: bridge.dictCount === 0
                            text: qsTr("No dictionary found. Put dictionary files into:\n%1").arg(bridge.dataDir)
                            color: Theme.textMut
                            font.pixelSize: Theme.baseSize - 1
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
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
