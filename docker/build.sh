IMAGE_NAME=ipex:driver950
docker build --build-arg http_proxy=$http_proxy \
             --build-arg https_proxy=$https_proxy \
             -t ${IMAGE_NAME} .
