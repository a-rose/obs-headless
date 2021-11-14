#!/bin/bash
#
# See function usage() for help
set -e

#######################################
# Configuration
#######################################

IMAGE="obs-headless"
CONTAINER="obs-headless"
VERSION=v1.0.0

run_args="--rm \
	--privileged \
	--gpus all \
	--name=$CONTAINER \
	-e DISPLAY \
	-v $(pwd)/etc/config.txt:/usr/local/src/obs-headless/config.txt \
	-v $(pwd)/shows/:/usr/local/src/obs-headless/shows/ \
	--net=host"

shell_args="$run_args"

#######################################
# Input validation
#######################################

action=$1
type=$2
echo "action='$action' type='$type'"

function check_type() {
	if [ "$type" = "dev" ]; then
		tag="dev"
	elif [ "$type" = "release" ]; then
		tag=$VERSION
	else
		echo "Invalid build type"
		usage
		exit 1
	fi
}

if [ -z "$action" ]; then
	action="usage"
fi

#######################################
# Targets
#######################################

function usage() {
	echo "./docker.sh <action> <build type>"
	echo ""
	echo "  --- BUILDING ------------------------------------------------------"
	echo "	  build <dev|release>:		build a 'dev' or 'release' image"
	echo ""
	echo "  --- RUNNING -------------------------------------------------------"
	echo "	  run <dev|release>:	run a container from the specified image"
	echo "	  gdb <dev|release>:	run gdb in a container from the specified image"
	echo "	  shell <dev|release>:	start an interactive container from the specified image"
	echo ""
	echo "  --- INTERACTING ---------------------------------------------------"
	echo "	  client:	start the test client"
	echo "	  attach:	attach a terminal to a running container"
	echo "	  logs:		get logs from a running container"
	echo "	  stop:		stop a running container"
	echo "	  kill:		kill a running container"
}


function build() {
	check_type
	echo "build type=$type"


	if [ "$type" = "release" ]; then
		build_args="--no-cache --build-arg BUILD_TYPE=Release"
	else
		build_args="--build-arg BUILD_TYPE=Debug"
	fi

	docker build . \
		$build_args \
		-f Dockerfile \
		-t $IMAGE:"$type"

	retVal=$?
	if [ $retVal -ne 0 ]; then
		echo "❌ Error building $IMAGE:$type!"
	else
		echo "✅ $IMAGE:$type built!"
	fi

	exit $retVal
}


function pre_run_steps() {
	xhost +
}

function run() {
	check_type
	pre_run_steps

	echo "Run $IMAGE:$tag"
	docker run $run_args $IMAGE:"$tag"
}

function gdb() {
	check_type
	pre_run_steps

	echo "Run $IMAGE:$tag"
	docker run -it $run_args --entrypoint gdb $IMAGE:"$tag" /usr/local/src/obs-headless/build/obs_headless_server
}

function shell() {
	check_type
	pre_run_steps

	docker run $shell_args -it --entrypoint /bin/bash $IMAGE:"$tag"
}


function client() {
	docker exec -it $CONTAINER /usr/local/src/obs-headless/build/obs_headless_client
}

function attach() {
	docker exec -it $CONTAINER /bin/bash
}

function logs() {
	docker logs  -f --since 1m $CONTAINER
}

function stop() {
	docker stop $CONTAINER
}

function kill() {
	docker kill $CONTAINER
}

#######################################
# Execute the target
#######################################

# Go!
$action
