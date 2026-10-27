#pragma once
// Fresh full-frame one-shot run. Previous v1 markers must remain untouched.
// One shared compile-time path keeps admission and insertion claims together.
namespace moveLive {
#ifdef INK_MOVE_PER_REQUEST
inline constexpr char runDirectory[] = "/home/root/xovi/move-session-small-templates-v4";
#else
inline constexpr char runDirectory[] = "/home/root/xovi/move-live-fullframe-v2";
#endif
}
