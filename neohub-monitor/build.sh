#!/bin/bash
docker buildx build --platform linux/amd64,linux/arm64 -t lhr.ocir.io/lrapvn2kw2t6/hem/neohub-monitor:latest --push .