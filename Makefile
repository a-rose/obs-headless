###########################################################
# Env
###########################################################

SHELL:=/bin/bash
cwd:=$(shell pwd)
obs_sources:=${HOME}/dev/obs-studio
obs_version:=26.0.0

# Make sure variables defined here are exported in subprocesses (docker)
.EXPORT_ALL_VARIABLES:

image:=obs-headless
container:=obs-headless
version:=v2.1.0-obs${obs_version}
latest:=latest-obs${obs_version}

# TODO
#DOCKER_BUILDKIT:=1

build_opt:=
run_opt:=

ifeq ($(release), 1)
	build_opt=--no-cache
	run_opt=-d
endif

ifeq ($(dev), 1)
	image=obs-headless-dev
endif


###########################################################
# Docker params
###########################################################

# TODO
#	check obs_sources exists

build_params:=--network=host \
			  --build-arg OBS_VERSION=${obs_version}

run_params:=-it \
		  --rm \
		  --privileged \
		  --gpus all \
		  --name=${container} \
		  --net=host \
		  -e DISPLAY \
		  -v $(cwd)/etc/:/opt/obs-headless/etc/

dev_params:=${run_params} \
		  --entrypoint /bin/bash \
		  --security-opt seccomp=unconfined \
		  -v $(cwd)/etc/bashrc:/root/.bashrc \
		  -v $(cwd)/src:/usr/local/src/obs-headless/ \
		  -v ${obs_sources}:/usr/local/src/obs-studio

define print_build_variables
	@echo -e "\e[32m--- Build variables: ---\e[0m"
	@echo -e "\t\e[32mimage:version: ${image}:${version}\e[0m"
	@echo -e "\t\e[32mbuild_params: ${build_params}\e[0m"
	@echo -e "\e[32m------\e[0m"
endef

define print_run_variables
	@echo -e "\e[32m--- Run variables: ---\e[0m"
	@echo -e "\t\e[32mimage:version: ${image}:${version}\e[0m"
	@echo -e "\t\e[32mcontainer: ${container}\e[0m"
	@echo -e "\t\e[32mrun_params: ${run_params}\e[0m"
	@echo -e "\e[32m------\e[0m"
endef

define print_dev_variables
	@echo -e "\e[32m--- Dev variables: ---\e[0m"
	@echo -e "\t\e[32mimage:version: ${image}:${version}\e[0m"
	@echo -e "\t\e[32mcontainer: ${container}\e[0m"
	@echo -e "\t\e[32mdev_params: ${dev_params}\e[0m"
	@echo -e "\e[32m------\e[0m"
endef


###########################################################
# Docker targets
###########################################################

base:
	@$(call print_build_variables)
	@docker build -f obs-headless-base.Dockerfile -t obs-headless-base:${version} .
	@docker tag obs-headless-base:${version} obs-headless-base:${latest}

builder: base
	@$(call print_build_variables)
	@docker build -f obs-headless-builder.Dockerfile -t obs-headless-builder:${version} ${build_params} .
	@@docker tag obs-headless-builder:${version} obs-headless-builder:${latest}

dev: builder
	@$(call print_build_variables)
	@docker build -f obs-headless-dev.Dockerfile -t obs-headless-dev:${version} ${build_params} .
	@docker tag obs-headless-dev:${version} obs-headless-dev:${latest}

release: dev
	@$(call print_build_variables)
	@docker build -f obs-headless.Dockerfile -t obs-headless:${version} ${build_params} .
	@docker tag obs-headless:${version} obs-headless:${latest}

run: pre-run
	@$(call print_run_variables)
	@xhost + && docker run ${run_params} ${image}:${version}

shell: pre-run
	@echo "Using ${image}-builder"
	@$(call print_dev_variables)
	@xhost + && docker run ${dev_params} -it --entrypoint /bin/bash ${image}-builder:${version}


###########################################################
# Commands for interacting with a running container
###########################################################


client:
	@docker exec -it ${container} /opt/obs-headless/obs_headless_client

attach:
	@docker exec -it ${container} /bin/bash

logs:
	@docker logs -f --since 1m ${container}

stop:
	@docker stop ${container}

kill:
	@docker kill ${container}


###########################################################
# Internal tasks
###########################################################

pre-run: rm-silent

rm-silent:
	@docker rm ${container} 2> /dev/null || true