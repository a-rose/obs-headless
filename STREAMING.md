# Streaming

This guide explains how to generate your own streams to use with obs-headless.

TODO explain not RTMP only

# Sources

Producing test streams locally, for example using [rtsp-simple-server](https://github.com/aler9/rtsp-simple-server).

1. Run rtsp-simple-server:

		docker run --rm -it --network=host aler9/rtsp-simple-server

2. Stream somthing to the server. You can choose between:

	a. Streaming live test sources to `rtsp-simple-server` (high CPU usage)

		# First source on port 1936
		ffmpeg -stream_loop -1 -re \
			-f lavfi -i "testsrc=size=1920x1080" \
			-f lavfi -i "sine=frequency=1000" -pix_fmt yuv420p \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-f flv rtmp://localhost:8554/sourceA

		# Second source on port 1937, this one without colors so we can easily
		# distinguish between two sources when switching.
		ffmpeg -stream_loop -1 -re \
			-f lavfi -i "testsrc=size=1920x1080" \
			-f lavfi -i "sine=frequency=1000" -pix_fmt yuv420p -vf hue=s=0 \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-f flv rtmp://localhost:8554/sourceB

	b. Transcoding and streaming a local file (medium CPU usage):

		ffmpeg -stream_loop -1 -re \
			-i myfile.mp4 \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-f flv rtmp://localhost:8554/sourceA

	c. Generating and streaming local files (low CPU usage):

		# First, produce our own test files from the test source. This way,
		# we can encode once and reuse the file to save some CPU.
		# We only need to do this once.
		# Here, we use -t 600 to get a 10 minutes file, to avoid having to
		# restart the source too often.
		ffmpeg \
			-f lavfi -i "sine=frequency=1000" \
			-f lavfi -i "testsrc=size=1920x1080" -pix_fmt yuv420p \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-t 600 etc/testsrc.mp4

		# Produce a second file with no colors.
		ffmpeg \
			-f lavfi -i "sine=frequency=1000" \
			-f lavfi -i "testsrc=size=1920x1080" -pix_fmt yuv420p -vf hue=s=0 \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-t 600 etc/testsrc2.mp4

		# Stream our files
		ffmpeg -stream_loop -1 -re \
			-i etc/testsrc.mp4 \
			-c copy \
			-f rtsp rtsp://localhost:8554/sourceA

		ffmpeg -stream_loop -1 -re \
			-i etc/testsrc2.mp4 \
			-c copy \
			-f rtsp rtsp://localhost:8554/sourceB

3. Edit your .json show file (`default.json` by default) to use `rtsp://localhost:8554/sourceA` and `rtsp://localhost:8554/sourceB`.

4. Set your stream output to `rtsp-simple-server` by setting a local url in `config.txt`:

		server rtmp://localhost/live/
		key key
		...

5. Start obs-headless (`./docker.sh run dev`) and the client (`./docker.sh client`)

6. Stream the output of `rtsp-simple-server`: `ffplay rtmp://localhost/live/key`