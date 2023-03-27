ARG OBS_VERSION
FROM obs-headless-builder:latest-obs${OBS_VERSION}

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /usr/local/src

ENV OBS_HEADLESS_INSTALL_PATH="/opt/obs-headless"
COPY src/ /usr/local/src/obs-headless
RUN cd obs-headless \
	\
	&& echo -e "\033[32mGenerating proto files...\033[0m" \
	&& cd proto_gen/ \
	&& sh proto_gen.sh \
	&& cd .. \
	\
	&& echo -e "\033[32mPreparing build...\033[0m" \
	&& mkdir -p build \
	&& cd build \
	&& cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="${OBS_HEADLESS_INSTALL_PATH}" \
		-DCMAKE_INSTALL_RPATH="${OBS_HEADLESS_INSTALL_PATH}/lib" \
		-DOBS_INSTALL_PATH="${OBS_INSTALL_PATH}" \
	\
	&& echo -e "\033[32mBuilding...\033[0m" \
	&& make -j$(nproc) \
	&& make install

COPY etc/ /opt/obs-headless/etc

ENTRYPOINT ["/opt/obs-headless/etc/docker-entrypoint.sh"]