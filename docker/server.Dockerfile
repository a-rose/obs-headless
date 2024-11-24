ARG OBS_VERSION
FROM obs-headless-builder:latest

ENV DEBIAN_FRONTEND=noninteractive

COPY src/server /usr/local/src/obs-headless/server
COPY src/lib /usr/local/src/obs-headless/lib

WORKDIR /usr/local/src/obs-headless/lib
RUN echo -e "\033[32mGenerating proto files...\033[0m" \
	&& ldconfig \
	&& sh proto_gen.sh

WORKDIR /usr/local/src/obs-headless/server
RUN echo -e "\033[32mPreparing build...\033[0m" \
	&& mkdir -p build

WORKDIR /usr/local/src/obs-headless/server/build
ENV OBS_HEADLESS_INSTALL_PATH="/opt/obs-headless"
RUN cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="${OBS_HEADLESS_INSTALL_PATH}" \
		-DCMAKE_INSTALL_RPATH="${OBS_HEADLESS_INSTALL_PATH}/lib" \
		-DOBS_INSTALL_PATH="${OBS_INSTALL_PATH}"

RUN echo -e "\033[32mBuilding...\033[0m" \
	&& make -j $(nproc) \
	&& make install

COPY etc/server /opt/obs-headless/etc

ENTRYPOINT ["/opt/obs-headless/etc/docker-entrypoint.sh"]
