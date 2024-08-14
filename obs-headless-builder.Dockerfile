ARG OBS_VERSION
FROM obs-headless-base:latest-obs${OBS_VERSION}

# Build OBS from sources
ENV OBS_BUILD_PATH="./build"
ENV OBS_INSTALL_PATH="/opt/obs-studio-portable"
ARG OBS_VERSION
RUN git clone --branch ${OBS_VERSION} --recursive https://github.com/obsproject/obs-studio.git
WORKDIR /usr/local/src/obs-studio
RUN cmake -S . \
		-B ${OBS_BUILD_PATH} \
		-G Ninja \
		-DLINUX_PORTABLE=ON \
		-DENABLE_BROWSER=OFF \
		-DENABLE_PIPEWIRE=OFF \
		-DENABLE_AJA=OFF \
		-DENABLE_LIBFDK=ON \
		-DCALM_DEPRECATION=ON \
	&& cmake --build ${OBS_BUILD_PATH} \
	&& cmake --install ${OBS_BUILD_PATH} --prefix ${OBS_INSTALL_PATH}
