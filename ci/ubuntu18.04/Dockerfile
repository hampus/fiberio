FROM ubuntu:18.04

ENV CXX g++
ENV COVERAGE=0
ENV DEBIAN_FRONTEND=noninteractive

#
# Install dependencies
#

RUN apt-get update

RUN apt-get -y install wget build-essential automake libtool pkg-config meson \
    libboost-fiber-dev libgtest-dev clang lcov ruby git

RUN gem install coveralls-lcov

#
# Build libuv
#

RUN mkdir /deps
WORKDIR /deps

RUN wget https://github.com/libuv/libuv/archive/v1.20.3.tar.gz
RUN tar xf v1.20.3.tar.gz
WORKDIR libuv-1.20.3
RUN ./autogen.sh && ./configure --prefix=/usr && make -j8 && make install

#
# Add source code
#

ADD . /src
WORKDIR /src

#
# Build command
#

CMD meson -Dcpp_std=c++17 -Db_coverage=true build && cd build && ninja test && \
    bash -c 'if [ "$COVERAGE" == "1" ] ; then \
        ninja coverage && echo "Running coveralls-lcov" && \
        coveralls-lcov meson-logs/coverage.info ; \
    fi'
