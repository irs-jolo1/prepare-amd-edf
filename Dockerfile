FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    chrpath \
    cpio \
    curl \
    diffstat \
    file \
    gawk \
    git \
    libiio-dev \
    libx11-6 \
    locales \
    lz4 \
    python3 \
    python3-pip \
    python3-yaml \
    tmux \
    wget \
    zstd \
    && locale-gen en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y locales \
    && sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen \
    && locale-gen \
    && update-locale LANG=en_US.UTF-8

ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

# libtinfo5 is required by meta-xilinx-tools (xsct) but is no longer packaged
# in Ubuntu 24.04 (noble ships libtinfo6 only), so fetch it from the older
# focal archive and install it directly.
RUN curl -sSLo /tmp/libtinfo5.deb \
    http://security.ubuntu.com/ubuntu/pool/universe/n/ncurses/libtinfo5_6.2-0ubuntu2.1_amd64.deb \
    && dpkg -i /tmp/libtinfo5.deb \
    && rm -f /tmp/libtinfo5.deb

# Install the Android repo tool
RUN curl -o /usr/local/bin/repo https://storage.googleapis.com/git-repo-downloads/repo \
    && chmod a+x /usr/local/bin/repo

# bb: thin bitbake wrapper that pretty-prints long variable dumps
# (RDEPENDS, ...) for humans. See scripts/bb.py.
COPY scripts/bb.py /usr/local/bin/bb
RUN chmod a+x /usr/local/bin/bb

ARG USER_UID=1000
ARG USER_GID=1000

RUN existing_user="$(getent passwd "$USER_UID" | cut -d: -f1 || true)"; \
    if [ -n "$existing_user" ] && [ "$existing_user" != "yocto" ]; then \
        userdel -r "$existing_user" 2>/dev/null || true; \
    fi; \
    (getent group "$USER_GID" >/dev/null || groupadd --gid "$USER_GID" yocto) \
    && useradd --create-home --shell /bin/bash --uid "$USER_UID" --gid "$USER_GID" yocto \
    && mkdir -p /workspace \
    && chown -R yocto:yocto /workspace


WORKDIR /workspace

COPY entrypoint.sh /entrypoint.sh
RUN chmod a+x /entrypoint.sh

USER yocto

# Simple colored prompt: bold green user@host, bold blue cwd.
RUN echo "PS1='\[\033[1;32m\]\u@\h\[\033[0m\]:\[\033[1;34m\]\w\[\033[0m\]\$ '" >> /home/yocto/.bashrc

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
