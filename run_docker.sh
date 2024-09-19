IMAGE_NAME=ipex:driver950
VIDEO=$(getent group video | sed -E 's,^video:[^:]*:([^:]*):.*$,\1,')
RENDER=$(getent group render | sed -E 's,^render:[^:]*:([^:]*):.*$,\1,')
test -z "$RENDER" || RENDER_GROUP="--group-add ${RENDER}"

# SCRIPT=../set_env.sh

docker run \
    --group-add ${VIDEO} \
    ${RENDER_GROUP} \
    --device=/dev/dri \
    --ipc=host \
  	--privileged \
    -v ${HOME}:/home/gta \
    -it $IMAGE_NAME bash