# syntax=docker/dockerfile:1.7

# STEP1: BUILD BACKEND
FROM golang:1.24-trixie AS builder_backend
WORKDIR /home/filestash/

# 1) OS deps first (cached independently of source changes)
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      ca-certificates curl make \
      libbrotli-dev \
      libjpeg-dev libtiff-dev libpng-dev libwebp-dev libraw-dev libheif-dev libgif-dev

# 2) Copy dependency descriptors first
COPY go.mod go.sum ./
COPY Makefile ./

# 3) Warm Go module cache
RUN --mount=type=cache,target=/go/pkg/mod,sharing=locked \
    --mount=type=cache,target=/root/.cache/go-build,sharing=locked \
    go mod download

# 4) Copy full source
COPY . .

# 5) Build
RUN --mount=type=cache,target=/go/pkg/mod,sharing=locked \
    --mount=type=cache,target=/root/.cache/go-build,sharing=locked \
    make init && \
    make build && \
    mkdir -p ./dist/data/state/config/ && \
    cp config/config.json ./dist/data/state/config/config.json

# STEP3: BUILD PROD IMAGE
FROM debian:stable-slim
WORKDIR /app/

# Install runtime deps once (cached)
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates curl ffmpeg && \
    useradd --system --home /app --shell /usr/sbin/nologin filestash

COPY --from=builder_backend /home/filestash/dist/ .

RUN chown -R filestash:filestash /app/ && \
    find /app/data/ -type d -exec chmod 770 {} \; && \
    find /app/data/ -type f -exec chmod 760 {} \; && \
    chmod 730 /app/filestash

USER filestash
CMD ["/app/filestash"]
EXPOSE 8334
