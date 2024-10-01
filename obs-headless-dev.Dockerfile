ARG OBS_VERSION
FROM obs-headless-builder:latest

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /usr/local/src
COPY src/ /usr/local/src/obs-headless

WORKDIR /usr/local/src/obs-headless/proto_gen
RUN echo -e "\033[32mGenerating proto files...\033[0m" \
	&& ldconfig \
	&& sh proto_gen.sh

WORKDIR /usr/local/src/obs-headless
ENV OBS_HEADLESS_INSTALL_PATH="/opt/obs-headless"
RUN echo -e "\033[32mPreparing build...\033[0m" \
	&& mkdir -p build \
	&& cd build \
	&& cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="${OBS_HEADLESS_INSTALL_PATH}" \
		-DCMAKE_INSTALL_RPATH="${OBS_HEADLESS_INSTALL_PATH}/lib" \
		-DOBS_INSTALL_PATH="${OBS_INSTALL_PATH}"

WORKDIR /usr/local/src/obs-headless/build
RUN echo -e "\033[32mBuilding...\033[0m" \
	&& make -j $(nproc) \
	&& make install

COPY etc/ /opt/obs-headless/etc

ENTRYPOINT ["/opt/obs-headless/etc/docker-entrypoint.sh"]
