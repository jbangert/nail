FROM blang/latex:latest

RUN set -ex; \
        sed -i s/archive.ubuntu.com/mirrors.163.com/g /etc/apt/sources.list; \
        apt-get update; \
        apt-get install -y --no-install-recommends \
            python3 \
        ; \
        rm -rf /var/lib/apt/lists/*
