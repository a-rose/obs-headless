ARG OBS_VERSION
FROM obs-headless-base:latest-obs${OBS_VERSION}

# Build OBS from sources
ENV OBS_INSTALL_PATH="/opt/obs-studio-portable"
ARG OBS_VERSION
RUN git clone --branch ${OBS_VERSION} --recursive https://github.com/obsproject/obs-studio.git \
	&& cd obs-studio \
	&& mkdir build \
	&& cd build \
	&& cmake .. \
		-DCMAKE_INSTALL_PREFIX=${OBS_INSTALL_PATH} \
		-DLINUX_PORTABLE=ON \
		-DENABLE_BROWSER=OFF \
		-DENABLE_PIPEWIRE=OFF \
		-DENABLE_AJA=OFF \
		-DENABLE_LIBFDK=ON \
	&& make -j$(nproc) \
	&& make install
