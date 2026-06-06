# =============================================================================
# Этап 1: Builder
# =============================================================================
FROM gcc:11.5-bullseye AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    libpq-dev \
    postgresql-server-dev-13 \
    libssl-dev \
    ca-certificates \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY CMakeLists.txt ./
COPY src/ ./src/

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=OFF \
    && cmake --build build --config Release -j$(nproc) \
    && strip build/empl_back

# =============================================================================
# Этап 2: Runtime
# =============================================================================
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libpq5 \
    ca-certificates \
    wget \
    && rm -rf /var/lib/apt/lists/* \
    && update-ca-certificates

RUN useradd -r -u 1000 -m appuser
WORKDIR /app

COPY --from=builder /build/build/empl_back /app/empl_back
RUN chown -R appuser:appuser /app
USER appuser

EXPOSE 8080

# Дефолтный порт для контейнера (переопределяется через docker-compose environment)
ENV SERVER_PORT=8080

ENTRYPOINT ["/app/empl_back"]