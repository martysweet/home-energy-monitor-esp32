#!/bin/bash

## TODO: Implement CI

#docker login lhr.ocir.io

docker buildx create --name armbuilder --use
docker buildx inspect --bootstrap


docker buildx build --platform linux/amd64,linux/arm64 -t lhr.ocir.io/lrapvn2kw2t6/hem/water-monitor:latest --push .
#docker build . -t water-monitor
#docker tag water-monitor:latest lhr.ocir.io/lrapvn2kw2t6/hem/water-monitor:1.0
#docker push lhr.ocir.io/lrapvn2kw2t6/hem/water-monitor:1.0
#docker run --privileged --rm tonistiigi/binfmt --install arm64,riscv64,arm