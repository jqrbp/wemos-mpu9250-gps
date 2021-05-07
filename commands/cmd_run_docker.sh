#!/bin/sh
docker run --name wemos-gps-backend -v $(pwd)/../data:/usr/share/nginx/html:ro -p8080:80 -d nginx