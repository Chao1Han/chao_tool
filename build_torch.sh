git config --global --add safe.directory '*'
if [ ! -e torch ]; then
    git clone --recurse-submodules https://github.com/intel-innersource/frameworks.ai.pytorch.private-gpu torch
fi
cd torch
read -p "install (y/n)? " CONT
if [ "$CONT" = "y" ]; then
python -m pip uninstall -y torch
python -m pip install -r requirements.txt
git submodule update --init --recursive
python setup.py install > build.log
fi