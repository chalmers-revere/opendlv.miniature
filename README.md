# opendlv.miniature

## Building using a Docker builder:

    cd docker
    make buildComplete
    make createDockerImage

The resulting Docker image chalmersrevere/opendlv-miniature-on-opendlv-on-opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest contains
the platform-specific Badger binaries on the OpenDLV binaries, the OpenDLV core binaries, and the latest OpenDaVINCI framework running on Ubuntu 16.04 LTS.

## Run the resulting Docker image:

    docker run -ti --rm --net host --user odv chalmersrevere/opendlv-miniature-on-opendlv-opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest /bin/bash

