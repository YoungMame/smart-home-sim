#!/bin/sh
set -e

echo "[watch] Configuring cmake..."
cmake -B /dist -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -S /simulator/src

echo "[watch] Building..."
cmake --build /dist --parallel

echo "[watch] Starting simulator..."
exec /dist/smart-home-sim
