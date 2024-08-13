// icpx -fsycl  -g -Xarch_host -fsanitize=address  -shared-libasan demo.cpp -o a.out -fsycl-targets=spir64_gen,spir64 -Xs "-device pvc" -ldnnl
#if __has_include(<sycl/sycl.hpp>)

#include <sycl/sycl.hpp>
#elif __has_include(<CL/sycl.hpp>)
#include <CL/sycl.hpp>
#else
#error "Unsupported compiler"
#endif
#include "oneapi/dnnl/dnnl.hpp"
#include <cassert>

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace dnnl;

memory::dims get_dims(int ndims) {
    memory::dims dimensions(ndims, 10);
    return dimensions;
}
memory::dims get_strides(int ndims) {
    memory::dims strides(ndims);
    int64_t stride = 1;
    for (int i = ndims - 1; i >= 0; --i) {
        strides[i] = stride;
        stride *= 10;
    }
    return strides;
}


int main() {
    const int ndims = 4;
    memory::dims dims = get_dims(ndims);
    memory::dims strides = get_strides(ndims);
    dnnl_memory_desc_t md;
    dnnl_data_type_t dtype = static_cast<dnnl_data_type_t>(dnnl::memory::data_type::f32);
    auto status = dnnl_memory_desc_create_with_strides(&md, ndims, dims.data(), dtype, strides.data());

    dnnl_dims_t* md_padded_dims = nullptr;

    dnnl_memory_desc_query(md, dnnl_query_padded_dims, &md_padded_dims);
    dnnl_memory_desc_destroy(md);
    assert(md_padded_dims != nullptr);

    const auto padded_dim = *md_padded_dims[0];

    return 0;
}

// int main() {
//     const int ndims = 4;
//     memory::dims dims = get_dims(ndims);
//     memory::dims strides = get_strides(ndims);
//     dnnl_memory_desc_t md;
//     dnnl_data_type_t dtype = static_cast<dnnl_data_type_t>(dnnl::memory::data_type::f32);
//     auto status = dnnl_memory_desc_create_with_strides(&md, ndims, dims.data(), dtype, strides.data());

//     dnnl_dims_t md_padded_dims;

//     dnnl_memory_desc_query(md, dnnl_query_padded_dims, &md_padded_dims);
//     dnnl_memory_desc_destroy(md);

//     const auto padded_dim = md_padded_dims[0];

//     return 0;
// }
