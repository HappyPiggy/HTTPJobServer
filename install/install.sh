#! /bin/sh

sudo apt-get install docker.io

sudo systemctl enable docker

sudo docker load -i httpjob.tar

sudo docker run -d  --restart always -v /var/log/httpjob:/record -p 8080:8080 httpjob:v1
