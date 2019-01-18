FROM ubuntu:xenial

RUN apt-get update
RUN apt-get install -y git-core build-essential
RUN apt-get install -y g++-4.9-aarch64-linux-gnu gcc-4.9-aarch64-linux-gnu \
  g++-4.7-arm-linux-gnueabihf gcc-4.7-arm-linux-gnueabihf
RUN apt-get install -y device-tree-compiler
RUN apt-get install -y dos2unix
RUN apt-get install -y ccache gcc-aarch64-linux-gnu
RUN apt-get install -y u-boot-tools
RUN apt-get install -y kpartx bsdtar mtools bc cpio

RUN cd /usr/bin/ && \
  ln -s arm-linux-gnueabihf-gcc-4.7 arm-linux-gnueabihf-gcc && \
  ln -s arm-linux-gnueabihf-g++-4.7 arm-linux-gnueabihf-g++ && \
  ln -s arm-linux-gnueabihf-cpp-4.7 arm-linux-gnueabihf-cpp

