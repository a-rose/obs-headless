ARG OBS_VERSION
FROM obs-headless-dev:latest-obs${OBS_VERSION} as builder

# Copy the whole filesystem in a new build stage, to flatten the layers into
# one and get a minimal image size
FROM scratch
COPY --from=builder / /
WORKDIR /opt/obs-headless

ENTRYPOINT ["/opt/obs-headless/etc/docker-entrypoint.sh"]