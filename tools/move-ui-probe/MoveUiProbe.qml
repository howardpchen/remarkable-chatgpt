import QtQuick 2.15

// UI-only specimen: deliberately no broker, network, filesystem or insertion API.
Rectangle {
    id: moveUiProbe
    objectName: "moveUiProbe"
    z: 10000
    anchors.right: parent ? parent.right : undefined
    anchors.bottom: parent ? parent.bottom : undefined
    anchors.margins: 16
    width: parent ? Math.max(0, Math.min(360, parent.width - 32)) : 0
    height: 176
    readonly property string diaryCaptureOwner: "ink-diary-owned-control-v1"
    property bool diaryCaptureSuppressed: false
    visible: !diaryCaptureSuppressed && parent !== null && parent.documentLoaded === true && parent.width >= 320 && parent.height >= 352
    color: "white"
    border.color: "black"
    border.width: 2
    radius: 8
    property bool simulatedBusy: false
    property int clicks: 0
    property string localMessage: "UI only: no API or insertion"

    Text {
        id: probeStatus
        objectName: "moveUiProbeStatus"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        height: 60
        font.pixelSize: 22
        wrapMode: Text.WordWrap
        text: moveUiProbe.localMessage
        color: "black"
    }

    Rectangle {
        id: probeButton
        objectName: "moveUiProbeButton"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        height: 76
        color: "white"
        border.color: "black"
        border.width: 2
        radius: 6

        Text {
            anchors.centerIn: parent
            font.pixelSize: 24
            color: "black"
            text: moveUiProbe.simulatedBusy ? "Cancel UI test" : "Start UI test"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                moveUiProbe.clicks += 1;
                moveUiProbe.simulatedBusy = !moveUiProbe.simulatedBusy;
                moveUiProbe.localMessage = moveUiProbe.simulatedBusy
                    ? "Simulated busy: tap Cancel"
                    : "Cancelled locally (" + moveUiProbe.clicks + " taps)";
            }
        }
    }
}
