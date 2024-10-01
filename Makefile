include .env
export


##########################################
# Build
##########################################

# Build all compose services at once
build:
	@echo "\n\033[42m=== Building server-base ===\033[0m"
	@docker compose build server-base
	@echo "\n\033[42m=== Building server-builder ===\033[0m"
	@docker compose build server-builder
	@echo "\n\033[42m=== Building server-dev ===\033[0m"
	@docker compose build server-dev
	@echo "\n\033[42m=== Building server ===\033[0m"
	@docker compose build server

# Generate test sources
generate:
	@mkdir -p sources
	@echo "\n\033[42m=== Generating testsrc.mp4 (sourceA: color) ===\033[0m"
	@docker compose run sourceA \
		-f lavfi -i "sine=frequency=1000" \
		-f lavfi -i "testsrc=size=1920x1080" -pix_fmt yuv420p \
		-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
		-c:a aac -b:a 128k \
		-t 600 sources/testsrc.mp4
	@echo "\n\033[42m=== Generating testsrc2.mp4 (sourceB: black and white) ===\033[0m"
	@docker compose run sourceB \
		-f lavfi -i "sine=frequency=1000" \
		-f lavfi -i "testsrc=size=1920x1080" -pix_fmt yuv420p -vf hue=s=0 \
		-c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 \
		-c:a aac -b:a 128k \
		-t 600 sources/testsrc2.mp4


##########################################
# Run
##########################################

# Start the streaming server
rtsp:
	@echo "\n\033[42m=== Starting the streaming server ===\033[0m"
	@docker compose up -d rtsp

# Start the streaming sources and server
testsrc: rtsp
	@echo "\n\033[42m=== Starting the streaming sources ===\033[0m"
	@docker compose up -d rtsp sourceA sourceB

# Start obs-headless server
server:
	@echo "\n\033[42m=== Starting obs-headless ===\033[0m"
	@xhost + 
	@docker compose up server

# Start sources + server
up: testsrc server

# Stop and remove all containers
down:
	@docker compose down --remove-orphans

# Start builder dev container
builder:
	@xhost + 
	@docker compose run server-builder

# Start obs-headless server dev container
dev:
	@xhost + 
	@docker compose run server-dev

# Start obs-headless client container
client:
	@docker compose run client

# Play obs-headless server output stream
play:
	@ffplay rtmp://localhost/live/key
