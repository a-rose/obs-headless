ARG OBS_VERSION
FROM obs-headless-dev:latest as builder

RUN echo -e "\033[32mRemoving dev packages...\033[0m" \
	&& apt-get remove --purge -y \
		vim gdb valgrind net-tools iptables procps tcpdump \
		linux-tools-common linux-tools-generic \
	&& rm -rf /var/lib/apt/lists/*


# Copy the whole filesystem in a new build stage, to flatten the layers into
# one and get a minimal image size
FROM scratch
COPY --from=builder / /
WORKDIR /opt/obs-headless

ENTRYPOINT ["/opt/obs-headless/etc/docker-entrypoint.sh"]