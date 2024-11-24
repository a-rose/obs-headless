
ARG OBS_VERSION
FROM obs-headless-base:latest

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /usr/local/src
COPY src/client /usr/local/src/obs-headless/client
COPY src/lib /usr/local/src/obs-headless/lib

WORKDIR /usr/local/src/obs-headless/lib
RUN echo -e "\033[32mGenerating proto files...\033[0m" \
	&& ldconfig \
	&& sh proto_gen.sh

WORKDIR /usr/local/src/obs-headless/client
RUN echo -e "\033[32mPreparing build...\033[0m" \
	&& mkdir -p build

WORKDIR /usr/local/src/obs-headless/client/build
ENV OBS_HEADLESS_INSTALL_PATH="/opt/obs-headless"
RUN cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="${OBS_HEADLESS_INSTALL_PATH}"

RUN echo -e "\033[32mBuilding...\033[0m" \
	&& make -j $(nproc) \
	&& make install

COPY etc/client /opt/obs-headless/etc

ENTRYPOINT ["/opt/obs-headless/client"]
