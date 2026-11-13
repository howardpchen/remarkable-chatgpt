# Chiappa bounded zoom/pan

At Start, compare uniform live scale with the immutable reviewed orientation
anchor. Admit 50–400% zoom and finite translation only; refuse shear, reflection,
DPR changes, unsupported dimensions/turns or missing calibration. Freeze the full
live transform for the request and reject changes through insertion.

Native envelope corners are projected through the frozen reading-to-scene inverse;
no grid/dot pitch or template pixel mask is needed. Existing native poses,
resource fingerprints, capture-owner boundary and visible answer-fit guards stay.

Historical numeric checks passed for both orientations and baseline restoration.
Real request evidence is narrower than numeric coverage; see
[checkpoint](PROJECT-STATUS.md) and [placement design](NATIVE-BOUNDS-PLACEMENT.md).
