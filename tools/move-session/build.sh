#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
: "${SDK_ENV:?Exact Chiappa SDK required}"
: "${XOVI_REPO:?XOVI source checkout required}"
set +u
source "$SDK_ENV"
set -u
[[ "$(pkg-config --modversion Qt6Core)" == 6.10.3 ]]
[[ "$($CXX -dumpmachine)" == aarch64* ]]
out=build/move-session
trial_flags=()
if [[ "${SINGLE_CAPTURE:-0}" == 1 ]]; then
    out=build/move-session-single-capture
    trial_flags=(-DINK_MOVE_SINGLE_CAPTURE)
fi
if [[ "${TOKEN_BUDGET:-0}" == 1 ]]; then
    out="$out-budget"
    trial_flags+=(-DINK_MOVE_TOKEN_BUDGET)
fi
mkdir -p "$out"
python3 "$XOVI_REPO/util/xovigen.py" -a aarch64 -o "$out"/xovi.cpp -H "$out"/xovi.h tools/move-live/live.xovi
python3 tools/offline-workflow/check-link-table.py "$out"/xovi.cpp
$CXX "${trial_flags[@]}" -std=c++17 -DINK_DIARY_EXPERIMENTAL_MOVE -shared -fPIC -Wl,-Bsymbolic -Wl,--version-script=tools/move-live/live.map tools/move-session/runtime.cpp "$out"/xovi.cpp -I"$out" -o "$out"/ink-diary-move-live.so $(pkg-config --cflags --libs Qt6Quick Qt6Qml Qt6Gui Qt6Core Qt6Network) -ldl
$STRIP --strip-unneeded "$out"/ink-diary-move-live.so
python3 - "$out" "${SINGLE_CAPTURE:-0}" "${TOKEN_BUDGET:-0}" <<'PY'
import hashlib,json,pathlib,struct,sys
p=pathlib.Path(sys.argv[1])/'ink-diary-move-live.so';b=p.read_bytes()
assert b[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',b,18)[0]==183
assert b'/home/root/xovi/move-session-small-templates-v4\0' in b
assert b'/home/root/xovi/move-live-fullframe-v2\0' not in b
m=dict(sha256=hashlib.sha256(b).hexdigest(),architecture='aarch64',qt='6.10.3',run_directory='/home/root/xovi/move-session-small-templates-v4',max_requests=3,max_output_tokens_per_request=800,automatic_retries=False,single_capture=(sys.argv[2]=='1'),deployed=False)
if sys.argv[3]=='1': m.update(max_requests=None,total_budget_usd=5,token_credits=50000000,reservation_credits=1850000,local_budget_guard=True,prompt_caching=False)
p.with_suffix('.json').write_text(json.dumps(m,indent=2)+'\n');print(m)
PY
