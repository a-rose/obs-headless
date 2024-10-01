# Streaming

This guide explains how to generate your own streams to use with obs-headless.

This guide uses RTMP streams, but `rtsp-simple-server` supports other formats; check out [their repo](https://github.com/aler9/rtsp-simple-server) for more info.

# Sources

Producing test streams locally:

1. Run the streaming server:

		make rtsp

2. Stream somthing to the server. You can choose between:

	a. Streaming live test sources to `rtsp-simple-server` (high CPU usage)

		# First source on port 1936
		ffmpeg -stream_loop -1 -re \
			-f lavfi -i "testsrc=size=1920x1080" \
			-f lavfi -i "sine=frequency=1000" -pix_fmt yuv420p \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-f flv rtmp://localhost/sourceA

		# Second source on port 1937, this one without colors so we can easily
		# distinguish between two sources when switching.
		ffmpeg -stream_loop -1 -re \
			-f lavfi -i "testsrc=size=1920x1080" \
			-f lavfi -i "sine=frequency=1000" -pix_fmt yuv420p -vf hue=s=0 \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-f flv rtmp://localhost/sourceB

	b. Transcoding and streaming a local file (medium CPU usage):

		ffmpeg -stream_loop -1 -re \
			-i myfile.mp4 \
			-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
			-c:a aac -b:a 128k \
			-f flv rtmp://localhost:8554/sourceA

	c. Generating and streaming local files (low CPU usage):

		make generate
		make testsrc

3. Edit your .json show file (`default.json` by default) to use `rtmp://localhost/sourceA` and `rtmp://localhost/sourceB`.

4. Set your stream output to `rtsp-simple-server` by setting a local url in `config.txt`:

		server rtmp://localhost/live/
		key key
		...

5. Start obs-headless (`make server`) and the client (`make client`)

6. Stream the output of `rtsp-simple-server`: `make play`