
# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [2.2.0] - 2024-08-20
 
### Added
- feat: a support for multiple active sources in a scene
- feat(client): add command to describe current studio state
- build: add bash functions 'rebuild' and 'start' to use from shell mode.
- feat(Docker): keep dev tools in the dev image

### Changed
- chore: update base image to Ubuntu 22.04 and update build
- chore: update cmake_minimum_required to 3.22
- chore: update obs to v30.2.2
- chore: update gRPC-generated files
- docs: update dev instructions
- refactor(client): code tidying

### Fixed
- fix(Show): fix name in UpdateProto
- fix(bashrc): fix rb alias
- fix(bashrc): fix st alias
