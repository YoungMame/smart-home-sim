#!/bin/sh
set -e

echo "[watch] Configuring cmake..."
cmake -B /src/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -S /src

echo "[watch] Building..."
cmake --build /src/build --parallel

echo "[watch] Starting simulator..."
exec /src/build/smart-home-sim
