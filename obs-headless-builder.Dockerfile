ARG OBS_VERSION
FROM obs-headless-base:latest-obs${OBS_VERSION}

# Build OBS from sources
ENV OBS_INSTALL_PATH="/opt/obs-studio-portable"
ARG OBS_VERSION
RUN git clone --recursive https://github.com/obsproject/obs-studio.git \
	&& cd obs-studio \
	&& git checkout ${OBS_VERSION} \
	&& mkdir build \
	&& cd build \
	&& cmake .. \
		-DCMAKE_INSTALL_PREFIX=${OBS_INSTALL_PATH} \
		-DUNIX_STRUCTURE=0 \
		-DBUILD_BROWSER=OFF \
		-DENABLE_PIPEWIRE=OFF \
	&& make -j$(nproc) \
	&& make install
