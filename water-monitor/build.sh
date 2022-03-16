#!/bin/bash

## TODO: Implement CI

#docker login lhr.ocir.io
docker build . -t water-monitor
docker tag water-monitor:latest lhr.ocir.io/lrapvn2kw2t6/hem/water-monitor:1.0
docker push lhr.ocir.io/lrapvn2kw2t6/hem/water-monitor:1.0