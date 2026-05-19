FROM debian:13-slim AS base

RUN apt-get update && apt-get install -y \
    make \
    g++-mingw-w64-x86-64 \
    g++-mingw-w64-i686 \
    zip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists /var/cache/apt/* \
    && mkdir /app

WORKDIR /app

FROM base AS builder

ENTRYPOINT ["make"]

FROM base AS dev

RUN apt-get update && apt-get install -y \
    clang-format \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists /var/cache/apt/*

