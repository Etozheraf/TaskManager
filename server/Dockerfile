FROM alpine:3.20 AS builder

RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    linux-headers \
    boost-dev \
    nlohmann-json \
    postgresql-dev \
    unzip

WORKDIR /app

COPY CMakeLists.txt .
COPY src ./src
COPY tests ./tests

RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    ctest --output-on-failure

# =========================================================================
FROM alpine:3.20

RUN addgroup -S appgroup && adduser -S appuser -G appgroup

RUN apk add --no-cache \
    libstdc++ \
    libgcc \
    boost-libs \
    icu-libs \
    libpq

WORKDIR /app
COPY --from=builder --chown=appuser:appgroup /app/build/src/TaskManagerServer .
USER appuser
CMD ["./TaskManagerServer"]
