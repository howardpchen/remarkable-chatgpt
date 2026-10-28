#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
broker_test_dir=$(mktemp -d "$PWD/build-host/broker-lifecycle.XXXXXX")
trap 'rm -rf -- "$broker_test_dir"' EXIT
cat > "$broker_test_dir/pipes.h" <<HEADER
#pragma once
#define IN_CMD_PIPE "$broker_test_dir/in"
#define OUT_PIPE "$broker_test_dir/out"
#define MAX_COMMAND_SIZE 1024
void initializePipes();
void stopPipes();
void shutdownPipes();
HEADER
cat > "$broker_test_dir/messagebroker.h" <<'HEADER'
#pragma once
extern "C" char *broadcastToNative(const char *,const char *,int *);
extern "C" char *broadcast(const char *,const char *);
HEADER
c++ -std=c++17 -Wall -Wextra -pthread tools/broker-lifecycle/pipes.cpp tools/broker-lifecycle/test-worker.cpp -I"$broker_test_dir" -o "$broker_test_dir/worker"
timeout 15 "$broker_test_dir/worker"
