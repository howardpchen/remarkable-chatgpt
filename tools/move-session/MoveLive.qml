import QtQuick 2.15
import net.asivery.XoviMessageBroker 2.0

// Compact idle launcher; expanded capture uses the tested 360×176 footprint.
// Text answer backend only. No on-load Start.
Rectangle {
    id: moveUiProbe
    objectName: "moveUiProbe"
    z: 10000
    anchors.right: parent ? parent.right : undefined
    anchors.bottom: parent ? parent.bottom : undefined
    anchors.margins: 16
    property bool controlsExpanded: false
    readonly property bool panelExpanded: controlsExpanded || demoBusy
    width: parent ? Math.max(0, Math.min(panelExpanded ? 360 : 72, parent.width - 32)) : 0
    height: panelExpanded ? 176 : 72
    readonly property string diaryCaptureOwner: "ink-diary-owned-control-v1"
    property bool diaryCaptureSuppressed: false
    // Reuse the exact lexical bridge; the capture-only Binding below restores
    // the original native visibility binding/value, not an imperative overwrite.
    // Unknown/missing state must not offer dismissal without a recovery signal.
    readonly property var nativeToolbar: (parent !== null &&
        typeof parent.diaryNativePoseObjects !== "undefined" && parent.diaryNativePoseObjects &&
        typeof parent.diaryNativePoseObjects.toolbar !== "undefined"
        ? parent.diaryNativePoseObjects.toolbar : null)
    readonly property bool toolbarsHidden: (nativeToolbar !== null &&
        typeof nativeToolbar !== "undefined" &&
        typeof nativeToolbar.shown === "boolean" && nativeToolbar.shown === false)
    readonly property bool nativeCaptureReady: (nativeToolbar !== null &&
        typeof nativeToolbar.visible === "boolean" && nativeToolbar.visible === false &&
        typeof nativeToolbar.shown === "boolean" && nativeToolbar.shown === false)
    Binding {
        target: moveUiProbe.nativeToolbar
        property: "visible"
        value: false
        when: moveUiProbe.diaryCaptureSuppressed && moveUiProbe.nativeToolbar !== null
        restoreMode: Binding.RestoreBindingOrValue
    }
    // Owner-intended exception: widget destruction need not restore native
    // visibility. Normal capture lease release still restores the prior state.
    property bool dismissedUntilToolbar: false
    onToolbarsHiddenChanged: {
        if (!toolbarsHidden) dismissedUntilToolbar = false;
    }
    onDemoBusyChanged: {
        if (demoBusy) dismissedUntilToolbar = false;
        else controlsExpanded = true; // Keep completion/error visible until explicitly collapsed.
    }
    visible: !dismissedUntilToolbar && !diaryCaptureSuppressed && parent !== null && parent.documentLoaded === true && parent.width >= 320 && parent.height >= 352
    color: "white"
    border.color: "black"
    border.width: 1
    radius: 12
    property bool demoBusy: false
    property bool demoArmed: false
    property int requestsRemaining: 0
    property bool tokenBudget: false
    property real budgetAvailable: 0
    property real budgetReservation: 0
    property bool budgetReady: false
    clip: true
    XoviMessageBroker { id: demoBroker }
    property int clicks: 0
    property string localMessage: "Text · checking budget"

    function statusLabel() {
        var credits = budgetAvailable >= 1000000 ? (Math.floor(budgetAvailable / 100000) / 10) + "M" : Math.floor(budgetAvailable / 1000) + "k";
        var budget = tokenBudget ? " · " + credits + " credits" : "";
        if (localMessage.indexOf("inserted") >= 0) return "Done" + budget;
        if (localMessage.indexOf("Thinking") === 0) return "Thinking…";
        if (localMessage.indexOf("Live text test") === 0) return "Reading page…";
        if (localMessage.indexOf("Cancelled;") === 0) return "Cancelled" + budget;
        if (localMessage.indexOf("Ready") === 0 || localMessage.indexOf("Text · checking") === 0 || localMessage.indexOf("Normal zoom:") === 0)
            return "AI" + budget;
        return localMessage; // Preserve actionable errors and unsupported notices.
    }

    function refreshStatus() {
        try {
            var result = JSON.parse(demoBroker.sendSimpleSignal("ink-diary-move-live$control", "status"));
            moveUiProbe.requestsRemaining = result.requestsRemaining || 0;
            moveUiProbe.tokenBudget = typeof result.tokenCreditsTotal === "number";
            moveUiProbe.budgetAvailable = result.tokenCreditsAvailable || 0;
            moveUiProbe.budgetReservation = result.tokenCreditsReservedEach || 0;
            moveUiProbe.budgetReady = result.budgetReady === true;
            moveUiProbe.demoBusy = result.busy === true;
            moveUiProbe.demoArmed = result.canStart === true;
            moveUiProbe.localMessage = result.message || result.error || "Text answer unavailable";
        } catch (error) {
            moveUiProbe.localMessage = "Text answer status unavailable";
        }
    }
    Timer {
        interval: 250
        repeat: true
        running: moveUiProbe.visible
        onTriggered: moveUiProbe.refreshStatus()
    }

    Rectangle {
        id: launcher
        objectName: "moveUiProbeLauncher"
        anchors.fill: parent
        color: "white"
        border.color: "black"
        border.width: 2
        radius: 8
        visible: !moveUiProbe.panelExpanded
        Text {
            anchors.centerIn: parent
            text: "AI"
            font.pixelSize: 24
            color: "black"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: moveUiProbe.controlsExpanded = true
        }
    }

    Rectangle {
        id: dismissButton
        objectName: "moveUiProbeDismissButton"
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        width: 40
        height: 40
        color: "white"
        // Keep Cancel reachable while a request is running.
        visible: moveUiProbe.panelExpanded && moveUiProbe.toolbarsHidden && !moveUiProbe.demoBusy
        enabled: visible
        Text {
            anchors.centerIn: parent
            text: "×"
            font.pixelSize: 30
            color: "black"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (moveUiProbe.toolbarsHidden && !moveUiProbe.demoBusy) {
                    moveUiProbe.controlsExpanded = false;
                    moveUiProbe.dismissedUntilToolbar = true;
                }
            }
        }
    }

    Rectangle {
        id: collapseButton
        objectName: "moveUiProbeCollapseButton"
        anchors.top: parent.top
        anchors.right: dismissButton.visible ? dismissButton.left : parent.right
        anchors.margins: 8
        width: 40
        height: 40
        color: "white"
        visible: moveUiProbe.panelExpanded && !moveUiProbe.demoBusy
        Text {
            anchors.centerIn: parent
            text: "−"
            font.pixelSize: 30
            color: "black"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (!moveUiProbe.demoBusy) moveUiProbe.controlsExpanded = false;
            }
        }
    }

    Text {
        id: probeStatus
        visible: moveUiProbe.panelExpanded
        objectName: "moveUiProbeStatus"
        anchors.left: parent.left
        anchors.right: collapseButton.visible ? collapseButton.left : parent.right
        anchors.top: parent.top
        anchors.margins: 12
        height: 60
        font.pixelSize: 22
        wrapMode: Text.WordWrap
        text: moveUiProbe.statusLabel()
        color: "black"
    }

    Rectangle {
        id: probeButton
        visible: moveUiProbe.panelExpanded
        objectName: "moveUiProbeButton"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        height: 68
        color: (moveUiProbe.demoBusy || moveUiProbe.demoArmed) ? "black" : "white"
        border.color: "black"
        border.width: 1
        radius: 10

        Text {
            anchors.centerIn: parent
            font.pixelSize: 24
            color: (moveUiProbe.demoBusy || moveUiProbe.demoArmed) ? "white" : "black"
            text: moveUiProbe.demoBusy ? "Cancel" : (moveUiProbe.demoArmed ? "Start" : (moveUiProbe.tokenBudget && moveUiProbe.budgetReady && moveUiProbe.budgetAvailable < moveUiProbe.budgetReservation ? "Budget reached" : "Setup needed"))
        }

        MouseArea {
            anchors.fill: parent
            enabled: moveUiProbe.demoBusy || moveUiProbe.demoArmed
            onClicked: {
                moveUiProbe.clicks += 1;
                demoBroker.sendSimpleSignal("ink-diary-move-live$control", moveUiProbe.demoBusy ? "cancel" : "start");
                moveUiProbe.refreshStatus();
            }
        }
    }
}
