# Build
FROM alpine:3.21 AS builder

RUN apk add --no-cache \
        build-base \
        cmake \
        ninja \
        git \
        ca-certificates \
        sqlite-dev \
        mosquitto-dev \
        openssl-dev

WORKDIR /build

# Copy only the manifest first to benefit from layer caching.
COPY src/CMakeLists.txt .

# Copy the full source tree.
COPY src/ .

# Configure and build in Release mode.
RUN cmake -B dist -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/install \
    && cmake --build dist --parallel \
    && cmake --install dist

# Runtime
FROM alpine:3.21 AS runtime

# Only the runtime shared libraries (no dev headers).
RUN apk add --no-cache \
        sqlite-libs \
        mosquitto-libs \
        libssl3

# Copy the installed binary and data files from the build stage.
COPY --from=builder /install/bin/smart-home-sim  /usr/local/bin/smart-home-sim
COPY --from=builder /install/share/smart-home-sim/data /data

# Simulator API — logs, simulation data, event triggers (user-facing).
EXPOSE 4000

ENV MQTT_BROKER_URL=mqtt://mqtt-broker:1883
ENV CORE_PORT=4000
# Paths to SQLite DB + SQL seed file inside the container.
ENV SMART_HOME_DATA_DIR=/data
ENV SMART_HOME_DB_PATH=/data/simulator.db
ENV SMART_HOME_DB_SEED=/data/seed.sql

ENTRYPOINT ["smart-home-sim"]

# Dev (watch mode)
# docker compose watch

FROM alpine:3.21 AS dev

RUN apk add --no-cache \
        build-base \
        cmake \
        ninja \
        git \
        ca-certificates \
        sqlite-dev \
        mosquitto-dev \
        openssl-dev

WORKDIR /simulator/src

ENV MQTT_BROKER_URL=mqtt://mqtt-broker:1883
ENV CORE_PORT=4000
ENV SMART_HOME_DATA_DIR=/simulator/src/data
ENV SMART_HOME_DB_PATH=/simulator/src/data/simulator.db
ENV SMART_HOME_DB_SEED=/simulator/src/data/seed.sql

EXPOSE 4000

COPY watch.sh /watch.sh
RUN chmod +x /watch.sh

COPY src/ /simulator/src/

ENTRYPOINT ["/watch.sh"]

# Test
FROM alpine:3.21 AS test

RUN apk add --no-cache \
        build-base \
        cmake \
        ninja \
        git \
        ca-certificates \
        sqlite-dev \
        mosquitto-dev \
        openssl-dev

WORKDIR /build
COPY src/ .
COPY tests/ ../tests/

RUN cmake -B dist -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
    && cmake --build dist --parallel

CMD ["ctest", "--test-dir", "dist", "--output-on-failure"]
