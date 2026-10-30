import QtQuick 2.15
import QtQuick.Controls 2.15
import "."

/*! Scroll bar that follows the theme instead of the platform palette, so it
 *  stays visible on both the dark and the light background. */
ScrollBar {
    id: bar

    /* The platform default is 13px wide and mostly transparent, which is easy
     * to miss on a dark surface. */
    width: 8
    padding: 0
    minimumSize: 0.08
    policy: ScrollBar.AsNeeded

    background: Rectangle {
        implicitWidth: 6
        radius: width / 2
        color: Theme.track
        opacity: bar.size < 1.0 ? 0.5 : 0.0
        Behavior on opacity { NumberAnimation { duration: Theme.fast } }
    }

    contentItem: Rectangle {
        implicitWidth: 6
        radius: width / 2
        color: bar.pressed ? Theme.accent : (bar.hovered ? Theme.textDim : Theme.textMut)
        opacity: bar.size < 1.0 ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: Theme.fast } }
    }
}
