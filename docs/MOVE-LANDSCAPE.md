# Chiappa landscape geometry

The Move physical framebuffer remains 954×1696; its landscape reading view is
1696×954. Compose the reviewed reading-to-physical quarter-turn map with the
physical-to-scene map. Never treat reading pixels as physical pixels or substitute
Ferrari geometry. Frame corners, dimensions and DPR 1 must match exactly.

Schema 4 profiles retain an independent landscape anchor. The supported opposite
turn is derived only as the exact calibrated half-turn, including complete reviewed
native poses; no arbitrary rotation, native-pose acceptance or portrait fallback.
Native-envelope placement needs no template pitch or template-only allowlist.

Numeric normal/zoom/pan/restoration checks were performed historically; they do
not prove real API/capture/insertion in every expanded landscape view. The current
ruled-template owner placement sample is not a universal landscape claim.
See [checkpoint](PROJECT-STATUS.md) and [design](NATIVE-BOUNDS-PLACEMENT.md).
