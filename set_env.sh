# compiler
source ${HOME}/intel/oneapi/compiler/latest/env/vars.sh

#MKL
export MKL_DPCPP_ROOT=${HOME}/intel/oneapi/mkl/latest
export LD_LIBRARY_PATH=${MKL_DPCPP_ROOT}/lib:${MKL_DPCPP_ROOT}/lib64:${MKL_DPCPP_ROOT}/lib/intel64:${LD_LIBRARY_PATH}
export LIBRARY_PATH=${MKL_DPCPP_ROOT}/lib:${MKL_DPCPP_ROOT}/lib64:${MKL_DPCPP_ROOT}/lib/intel64:$LIBRARY_PATH
export CMAKE_INCLUDE_PATH=${MKL_DPCPP_ROOT}/include/
export CMAKE_LIBRARY_PATH=${MKL_DPCPP_ROOT}/lib/intel64/

#CCL
source ${HOME}/intel/oneapi/ccl/latest/env/vars.sh

#PTI for kineto profiler
source ${HOME}/intel/oneapi/pti/latest/env/vars.sh
export BUILD_SEPARATE_OPS=ON
export USE_AOT_DEVLIST='pvc'
export BUILD_WITH_CPU=OFF
export USE_MKL=OFF
export USE_XETLA=OFF
export USE_DS_KERNELS=OFF
export USE_ONEMKL=OFF
export USE_PTI=OFF
export USE_KINETO=OFF
export USE_XCCL=ON

# export SYCL_PI_LEVEL_ZERO_USE_IMMEDIATE_COMMANDLISTS=1
# export SYCL_PI_LEVEL_ZERO_DEVICE_SCOPE_EVENTS=0
# export ExperimentalCopyThroughLock=1

alias pyclean="python setup.py clean"
alias pyin="pip uninstall -y intel_extension_for_pytorch;python setup.py install"
alias gitme="git config --local user.name 'hanchao';git config --local user.email 'chao1.han@intel.com'"

