FROM buildpack-deps:stretch

RUN set -ex; \
        apt-get update; \
        apt-get install -y --no-install-recommends \
            clang \
            libboost-all-dev \
            astyle \
            xxd \
        ; \
        rm -rf /var/lib/apt/lists/*
