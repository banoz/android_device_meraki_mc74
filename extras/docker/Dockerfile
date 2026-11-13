FROM ubuntu:16.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    openjdk-8-jdk-headless python python3 git curl ca-certificates bc bison build-essential flex \
    g++-multilib gcc-multilib gnupg gperf imagemagick lib32ncurses5-dev lib32readline6-dev lib32z1-dev \
    libesd0-dev liblz4-tool libncurses5-dev libsdl1.2-dev libssl-dev libwxgtk3.0-dev libxml2 \
    libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc yasm zip unzip zlib1g-dev \
    lzma maven ccache sudo && rm -rf /var/lib/apt/lists/*
ARG UID=1000
RUN useradd -m -u $UID builder && echo "builder ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/builder
USER builder
WORKDIR /src
ENV USER=builder LC_ALL=C
# Jack server talks TLSv1/1.1, which later JDK 8 updates disable
USER root
RUN sed -i 's/^\(jdk.tls.disabledAlgorithms=\)SSLv3, TLSv1, TLSv1.1,/\1SSLv3,/' /etc/java-8-openjdk/security/java.security && grep -A1 '^jdk.tls.disabledAlgorithms' /etc/java-8-openjdk/security/java.security
USER builder
