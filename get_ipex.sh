name=$1
git config --global --add safe.directory '*'
if [ ! -e ${name} ]; then
git clone --recurse-submodules https://github.com/intel-innersource/frameworks.ai.pytorch.ipex-gpu.git ${name}
fi