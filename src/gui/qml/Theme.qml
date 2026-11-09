pragma Singleton
import QtQuick 2.15

/*! Single source of truth for the palette, radii and durations used by the
 *  whole UI.  Flip `dark` and every bound colour follows. */
QtObject {
    id: theme

    property bool dark: true

    /* surfaces */
    property color bg:         dark ? "#0e1015" : "#f4f6fb"
    property color surface:    dark ? "#171a22" : "#ffffff"
    property color surfaceAlt: dark ? "#1f232d" : "#eef1f7"
    property color surfaceHi:  dark ? "#262b37" : "#e2e7f1"
    property color border:     dark ? "#2c3241" : "#dbe1ec"

    /* text */
    property color text:    dark ? "#e7eaf1" : "#161a22"
    property color textDim: dark ? "#98a1b3" : "#5b6474"
    property color textMut: dark ? "#6b7385" : "#8b93a3"

    /* accents */
    /* single source for the accent, also exposed as a string because the rich
     * text renderer needs the literal in an inline `style` attribute */
    readonly property string accentHex: "#4f8cff"
    property color accent:  accentHex
    property color accent2: "#7b5cff"
    property color success: "#33c48d"
    property color danger:  "#ff5f7e"
    property color warn:    "#f2b23c"

    property color selection: dark ? "#2b3a5c" : "#d6e4ff"
    /* unfilled slider/switch track: needs more contrast than surfaceHi in
     * both themes so the remaining range stays visible. */
    property color track: dark ? "#39414f" : "#c6d0e0"

    /* geometry */
    property int radius:      12
    property int radiusSmall: 8
    property int pad:         12
    property int gutter:      10

    /* motion */
    property int fast: 120
    property int slow: 220

    /* baseSize is in logical pixels, the unit QML font.pixelSize uses. */
    property string fontFamily: ""
    property int baseSize: 16

    /* Derived sizes, floored to stay legible at the minimum baseSize. */
    readonly property int smallSize: Math.max(11, baseSize - 2)
    readonly property int tinySize:  Math.max(10, baseSize - 4)

    /* Text should fill ~90% of a control. QML draws a pixelSize font in about
     * 1.3x its size, so size chrome from that line box instead of the font
     * size plus a fixed pad -- text reads big while the control stays small. */
    readonly property real textFill: 0.9
    readonly property int textLine: Math.ceil(baseSize * 1.3)
    readonly property int tinyLine: Math.ceil(tinySize * 1.3)

    readonly property int controlHeight: Math.max(28, Math.round(textLine / textFill))
    readonly property int toolbarHeight: Math.max(46, controlHeight + 18)
    readonly property int statusHeight:  Math.max(24, tinyLine + 8)

    property Gradient accentGradient: Gradient {
        GradientStop { position: 0.0; color: theme.accent }
        GradientStop { position: 1.0; color: theme.accent2 }
    }
}
