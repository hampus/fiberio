FROM base/archlinux:latest

ENV CXX g++

#
# Install dependencies
#

RUN pacman --noconfirm -Syu
RUN pacman --noconfirm -S  pkg-config meson boost libuv gtest clang

#
# Add source code
#

ADD . /src
WORKDIR /src

#
# Build command
#

CMD meson build && cd build && ninja test
