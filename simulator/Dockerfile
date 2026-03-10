# ────────────────────────────────────────────────────────────────────────────
# Stage 1 — Build
# ────────────────────────────────────────────────────────────────────────────
FROM alpine:3.21 AS builder

RUN apk add --no-cache \
        build-base \
        cmake \
        ninja \
        git \
        ca-certificates \
        mosquitto-dev \
        openssl-dev

WORKDIR /src

# Copy only the manifest files first to benefit from layer caching.
COPY CMakeLists.txt .

# Copy the rest of the source tree.
COPY . .

# Configure and build in Release mode.
RUN cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/install \
    && cmake --build build --parallel \
    && cmake --install build

# ────────────────────────────────────────────────────────────────────────────
# Stage 2 — Runtime
# ────────────────────────────────────────────────────────────────────────────
FROM alpine:3.21 AS runtime

# Only the runtime shared libraries (no dev headers).
RUN apk add --no-cache \
        mosquitto-libs \
        libssl3

# Copy the installed binary and data files from the build stage.
COPY --from=builder /install/bin/smart-home-sim  /usr/local/bin/smart-home-sim
COPY --from=builder /install/share/smart-home-sim/data /data

# Simulator API — logs, simulation data, event triggers (user-facing).
EXPOSE 4000

ENV MQTT_BROKER_URL=mqtt://mqtt-broker:1883
ENV CORE_PORT=4000

ENTRYPOINT ["smart-home-sim"]
