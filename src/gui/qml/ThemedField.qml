import QtQuick 2.15
import QtQuick.Controls 2.15
import "."

/*! Single line editor matching the theme. */
TextField {
    id: field

    property int fieldHeight: Theme.controlHeight
    /* 0 无判定, 1 正确, -1 错误: 让调用方给输入框加对错边框 */
    property int verdict: 0

    /* 高度按真实行高的 1.35 倍, 留出上下余量 */
    implicitHeight: Math.max(fieldHeight, Math.ceil(contentHeight * 1.35))
    color: Theme.text
    placeholderTextColor: Theme.textMut
    font.pixelSize: Theme.baseSize
    font.family: Theme.fontFamily
    selectByMouse: true
    leftPadding: 12
    rightPadding: 12
    /* 单行 TextInput 不支持垂直居中, 汉字墨迹约占行高 0.63, 这里手动压到中间 */
    topPadding: Math.max(0, Math.round((height - contentHeight * 0.63) / 2))
    bottomPadding: 0
    selectionColor: Theme.accent
    selectedTextColor: "#ffffff"

    background: Rectangle {
        radius: Theme.radiusSmall
        color: Theme.surfaceAlt
        border.width: field.verdict !== 0 ? 2 : 1
        border.color: field.verdict === 1 ? Theme.success
                    : field.verdict === -1 ? Theme.danger
                    : (field.activeFocus ? Theme.accent : Theme.border)
        Behavior on border.color { ColorAnimation { duration: Theme.fast } }
    }
}
