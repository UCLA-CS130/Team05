#!/bin/bash

# Save shrunken docker image
echo "Saving the current docker image to be transferred to AWS instance";
docker save -o webserver_image httpserver;

# Transfer docker image to AWS instance
scp -i ./team05-pk.pem webserver_image ubuntu@ec2-54-213-82-160.us-west-2.compute.amazonaws.com:;

# SSH login to AWS instance
echo "Deploying docker image to AWS instance";
ssh -i ./team05-pk.pem ubuntu@ec2-54-213-82-160.us-west-2.compute.amazonaws.com '
# Kill all docker container processes that are running
docker kill $(docker ps -a -q);

# Load the new docker image
docker load -i webserver_image;

# Run the docker image
docker run --rm -t -p 2020:2020 httpserver;'

