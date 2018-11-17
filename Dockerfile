# FROM jupyter/base-notebook
# USER root
# ENV DEBIAN_FRONTEND=noninteractive
FROM neurodebian:xenial
# Prepare environment
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
                    curl \
                    bzip2 \
                    ca-certificates \
                    xvfb \
                    cython3 \
                    build-essential \
                    autoconf \
                    libtool \
                    ncurses-dev \
                    vim \
                    git \
                    libmotif-dev \
                    libnetcdf-dev \
                    tcsh \
                    r-base \
                    python-qt4 \
                    wget \
                    pkg-config 
#     curl -sSL http://neuro.debian.net/lists/xenial.us-ca.full >> /etc/apt/sources.list.d/neurodebian.sources.list && \
#     apt-key add /root/.neurodebian.gpg && \
#     (apt-key adv --refresh-keys --keyserver hkp://ha.pool.sks-keyservers.net 0xA5D32F012649A5A9 || true)

# RUN echo deb-src http://neuro.debian.net/debian bionic main contrib> /etc/apt/sources.list.d/neurodebian.list

RUN wget https://launchpad.net/ubuntu/+archive/primary/+sourcefiles/cmake/3.12.1-1/cmake_3.12.1.orig.tar.gz \
  ; tar xzvf cmake_3.12.1.orig.tar.gz \
  ; cd cmake-3.12.1 \
  ; ./bootstrap && make \
  ; make install \
  ; cd .. \
  ;rm -fr cmake* 

# RUN apt-get update & apt-get -y build-dep afni
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    freeglut3-dev \
    libgsl-dev \
    libglew-dev \
    libglib2.0-dev \
    libglw-dev \
    libinsighttoolkit3-dev \
    libopenjpeg-dev 
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libxmu-dev \
    libxpm-dev \
    libxt-dev  \
    libjpeg-dev


# #######################################
ADD src /afni/src/
ADD cmake /afni/cmake/
ADD CMakeLists.txt /afni/
ADD Dockerfile /afni/
RUN  mkdir -p /build
WORKDIR /build

RUN  cmake /afni -DBUILD_SHARED_LIBS=ON
RUN make -j 20 install
# RUN apsearch -update_all_afni_help

