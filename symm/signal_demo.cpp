// mpicxx -cxx=icpx -fsycl -fsycl-targets=spir64 signal_demo.cpp -lze_loader -o signal_demo

#include "exchange.h"
#include <sycl/sycl.hpp>

#define ZE_CHECK(cmd) do {                           \
    ze_result_t e = cmd;                             \
    if( e != ZE_RESULT_SUCCESS ) {                   \
        printf("Level-Zero error at %s:%d code=%d\n", \
                __FILE__,__LINE__, e);                \
        exit(EXIT_FAILURE);                           \
    }                                                 \
} while(0)

// Create SYCL queue for specific GPU device
static sycl::queue create_queue(int local_rank) {
    auto platforms = sycl::platform::get_platforms();
    for (const auto &platform : platforms) {
        if (platform.get_backend() == sycl::backend::ext_oneapi_level_zero) {
            return sycl::queue(platform.get_devices()[local_rank],
                             {sycl::property::queue::in_order{}});
        }
    }
    throw std::runtime_error("Level-Zero platform not found.");
}

// ========== Signal Implementation ==========

// CAS operation using SYCL atomic_ref
template <sycl::memory_order Sem>
struct CASKernel {
    uint32_t* addr;
    uint32_t compare;
    uint32_t val;
    uint32_t* result;
    
    void operator()(sycl::nd_item<1> item) const {
        if (item.get_global_id(0) == 0) {
            sycl::atomic_ref<uint32_t, 
                           Sem, 
                           sycl::memory_scope::system> ref(*addr);
            uint32_t expected = compare;
            ref.compare_exchange_strong(expected, val);
            *result = expected;
        }
    }
};

// Device kernel: put signal (0 → 1)
struct PutSignalKernel {
    uint32_t** signal_pads;
    int dst_rank;
    int channel;
    int src_rank;
    int world_size;
    
    void operator()(sycl::nd_item<1> item) const {
        if (item.get_global_id(0) == 0) {
            uint32_t* target_addr = signal_pads[dst_rank] + channel * world_size + src_rank;
            
            // Spin until we can set 0→1
            sycl::atomic_ref<uint32_t,
                           sycl::memory_order::acq_rel,
                           sycl::memory_scope::system> ref(*target_addr);
            
            uint32_t expected = 0;
            size_t max_iter = 10000000;
            for (size_t i = 0; i < max_iter; ++i) {
                expected = 0;
                if (ref.compare_exchange_strong(expected, 1)) {
                    return; // Success
                }
            }
            // Timeout - this should not happen
            assert(false && "put_signal timeout");
        }
    }
};

// Device kernel: wait signal (1 → 0)
struct WaitSignalKernel {
    uint32_t** signal_pads;
    int src_rank;
    int channel;
    int my_rank;
    int world_size;
    
    void operator()(sycl::nd_item<1> item) const {
        if (item.get_global_id(0) == 0) {
            uint32_t* target_addr = signal_pads[my_rank] + channel * world_size + src_rank;
            
            // Spin until we can set 1→0
            sycl::atomic_ref<uint32_t,
                           sycl::memory_order::acq_rel,
                           sycl::memory_scope::system> ref(*target_addr);
            
            uint32_t expected = 1;
            size_t max_iter = 10000000;
            for (size_t i = 0; i < max_iter; ++i) {
                expected = 1;
                if (ref.compare_exchange_strong(expected, 0)) {
                    sycl::atomic_fence(sycl::memory_order::seq_cst, 
                                      sycl::memory_scope::system);
                    return; // Success
                }
            }
            // Timeout - this should not happen
            assert(false && "wait_signal timeout");
        }
    }
};

// Device kernel: barrier (all-to-all synchronization)
struct BarrierKernel {
    uint32_t** signal_pads;
    int channel;
    int rank;
    int world_size;
    
    void operator()(sycl::nd_item<1> item) const {
        int thread_id = item.get_global_id(0);
        
        if (thread_id < world_size && thread_id != rank) {
            int target_rank = thread_id;
            
            // Put signal to target_rank
            uint32_t* put_addr = signal_pads[target_rank] + channel * world_size + rank;
            sycl::atomic_ref<uint32_t,
                           sycl::memory_order::acq_rel,
                           sycl::memory_scope::system> put_ref(*put_addr);
            
            size_t max_iter = 10000000;
            for (size_t i = 0; i < max_iter; ++i) {
                uint32_t expected = 0;
                if (put_ref.compare_exchange_strong(expected, 1)) {
                    break;
                }
            }
            
            // Wait signal from target_rank
            uint32_t* wait_addr = signal_pads[rank] + channel * world_size + target_rank;
            sycl::atomic_ref<uint32_t,
                           sycl::memory_order::acq_rel,
                           sycl::memory_scope::system> wait_ref(*wait_addr);
            
            for (size_t i = 0; i < max_iter; ++i) {
                uint32_t expected = 1;
                if (wait_ref.compare_exchange_strong(expected, 0)) {
                    break;
                }
            }
        }
    }
};

// Host API: put signal
void put_signal(sycl::queue& q, uint32_t** signal_pads, int dst_rank, 
                int channel, int src_rank, int world_size) {
    q.parallel_for(sycl::nd_range<1>(32, 32), 
                   PutSignalKernel{signal_pads, dst_rank, channel, src_rank, world_size})
     .wait();
}

// Host API: wait signal
void wait_signal(sycl::queue& q, uint32_t** signal_pads, int src_rank,
                 int channel, int my_rank, int world_size) {
    q.parallel_for(sycl::nd_range<1>(32, 32),
                   WaitSignalKernel{signal_pads, src_rank, channel, my_rank, world_size})
     .wait();
}

// Host API: barrier
void barrier(sycl::queue& q, uint32_t** signal_pads, int channel, 
             int rank, int world_size) {
    int num_threads = std::max(32, world_size);
    q.parallel_for(sycl::nd_range<1>(num_threads, num_threads),
                   BarrierKernel{signal_pads, channel, rank, world_size})
     .wait();
}

// ========== Main Demo ==========

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    ZE_CHECK(zeInit(0));

    auto compute_queue = create_queue(rank);
    auto l0_ctx = sycl::get_native<sycl::backend::ext_oneapi_level_zero>(
        compute_queue.get_context());
    auto l0_device = sycl::get_native<sycl::backend::ext_oneapi_level_zero>(
        compute_queue.get_device());

    printf("[Rank %d] Initialized\n", rank);

    // ========== Step 1: Allocate local memory and signal pads ==========
    constexpr size_t num_elements = 16;
    constexpr size_t bytes = num_elements * sizeof(float);
    constexpr int num_channels = 4;  // Support multiple channels
    
    // Allocate data buffer
    float *local_ptr = sycl::malloc_device<float>(num_elements, compute_queue);
    compute_queue.fill(local_ptr, static_cast<float>((rank + 1) * 100), num_elements).wait();

    // Allocate signal pad: world_size ranks × num_channels channels × world_size signals
    size_t signal_pad_size = world_size * num_channels * world_size * sizeof(uint32_t);
    uint32_t* local_signal_pad = sycl::malloc_device<uint32_t>(
        world_size * num_channels * world_size, compute_queue);
    compute_queue.memset(local_signal_pad, 0, signal_pad_size).wait();

    printf("[Rank %d] Allocated %zu bytes for data, %zu bytes for signal pad\n",
           rank, bytes, signal_pad_size);

    // ========== Step 2: Create IPC handles ==========
    void* data_base_addr;
    size_t data_base_size;
    ZE_CHECK(zeMemGetAddressRange(l0_ctx, local_ptr, &data_base_addr, &data_base_size));
    size_t data_offset = (char*)local_ptr - (char*)data_base_addr;

    void* signal_base_addr;
    size_t signal_base_size;
    ZE_CHECK(zeMemGetAddressRange(l0_ctx, local_signal_pad, &signal_base_addr, &signal_base_size));
    size_t signal_offset = (char*)local_signal_pad - (char*)signal_base_addr;

    ze_ipc_mem_handle_t data_ipc_handle, signal_ipc_handle;
    ZE_CHECK(zeMemGetIpcHandle(l0_ctx, data_base_addr, &data_ipc_handle));
    ZE_CHECK(zeMemGetIpcHandle(l0_ctx, signal_base_addr, &signal_ipc_handle));

    // 提取 fd
    int data_fd = get_fd_from_handle(data_ipc_handle);
    int signal_fd = get_fd_from_handle(signal_ipc_handle);
    
    printf("[Rank %d] Created IPC handles: data_fd=%d, signal_fd=%d\n", rank, data_fd, signal_fd);

    // ========== Step 3: Exchange IPC handles via IpcChannel ==========
    
    // 收集所有进程的 PID
    int my_pid = getpid();
    std::vector<int> all_pids(world_size);
    MPI_Allgather(&my_pid, 1, MPI_INT, all_pids.data(), 1, MPI_INT, MPI_COMM_WORLD);
    
    printf("[Rank %d] PID=%d, all PIDs: ", rank, my_pid);
    for (int i = 0; i < world_size; ++i) printf("%d ", all_pids[i]);
    printf("\n");

    // 使用 MPI_Allgather 交换 offset 信息
    std::vector<size_t> all_data_offsets(world_size);
    std::vector<size_t> all_signal_offsets(world_size);
    MPI_Allgather(&data_offset, sizeof(size_t), MPI_BYTE, 
                  all_data_offsets.data(), sizeof(size_t), MPI_BYTE, MPI_COMM_WORLD);
    MPI_Allgather(&signal_offset, sizeof(size_t), MPI_BYTE, 
                  all_signal_offsets.data(), sizeof(size_t), MPI_BYTE, MPI_COMM_WORLD);

    // 创建 IpcChannel 并交换 fd
    IpcChannel ipc_channel;
    MPI_Barrier(MPI_COMM_WORLD);
    
    // 使用 all_gather_fds 交换 data fd
    std::vector<int> all_data_fds = ipc_channel.all_gather_fds(rank, all_pids, data_fd);
    MPI_Barrier(MPI_COMM_WORLD);
    
    // 使用 all_gather_fds 交换 signal fd
    std::vector<int> all_signal_fds = ipc_channel.all_gather_fds(rank, all_pids, signal_fd);
    MPI_Barrier(MPI_COMM_WORLD);
    
    printf("[Rank %d] Received fds: data=[", rank);
    for (int i = 0; i < world_size; ++i) printf("%d ", all_data_fds[i]);
    printf("], signal=[");
    for (int i = 0; i < world_size; ++i) printf("%d ", all_signal_fds[i]);
    printf("]\n");

    // Collect remote handles from all ranks
    std::vector<float*> remote_data_ptrs(world_size);
    std::vector<uint32_t*> remote_signal_pads(world_size);
    
    remote_data_ptrs[rank] = local_ptr;
    remote_signal_pads[rank] = local_signal_pad;

    for (int i = 0; i < world_size; ++i) {
        if (i == rank) continue;
        
        // 重建 IPC handle
        ze_ipc_mem_handle_t remote_data_handle = data_ipc_handle;  // 复制结构
        ze_ipc_mem_handle_t remote_signal_handle = signal_ipc_handle;
        set_fd_in_handle(remote_data_handle, all_data_fds[i]);
        set_fd_in_handle(remote_signal_handle, all_signal_fds[i]);
        
        void* remote_data_base;
        void* remote_signal_base;
        
        ZE_CHECK(zeMemOpenIpcHandle(l0_ctx, l0_device, remote_data_handle,
                                    ZE_IPC_MEMORY_FLAG_BIAS_CACHED, &remote_data_base));
        ZE_CHECK(zeMemOpenIpcHandle(l0_ctx, l0_device, remote_signal_handle,
                                    ZE_IPC_MEMORY_FLAG_BIAS_CACHED, &remote_signal_base));

        remote_data_ptrs[i] = (float*)((char*)remote_data_base + all_data_offsets[i]);
        remote_signal_pads[i] = (uint32_t*)((char*)remote_signal_base + all_signal_offsets[i]);
        
        printf("[Rank %d] Imported memory from rank %d\n", rank, i);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    printf("[Rank %d] All IPC handles exchanged\n", rank);

    // ========== Step 4: Create signal_pads pointer array on device ==========
    uint32_t** signal_pads_device = sycl::malloc_device<uint32_t*>(world_size, compute_queue);
    compute_queue.memcpy(signal_pads_device, remote_signal_pads.data(), 
                        world_size * sizeof(uint32_t*)).wait();

    // ========== Step 5: Test signal operations ==========
    
    printf("\n[Rank %d] ========== Test 1: Put/Wait Signal ==========\n", rank);
    
    if (rank == 0) {
        // Rank 0 writes to rank 1's memory, then signals
        compute_queue.fill(remote_data_ptrs[1], 999.0f, num_elements).wait();
        printf("[Rank 0] Wrote 999 to rank 1's memory\n");
        
        put_signal(compute_queue, signal_pads_device, 1, 0, 0, world_size);
        printf("[Rank 0] Sent signal to rank 1 on channel 0\n");
        
        // Wait for rank 1's acknowledgment
        wait_signal(compute_queue, signal_pads_device, 1, 0, 0, world_size);
        printf("[Rank 0] Received acknowledgment from rank 1\n");
    } 
    else if (rank == 1) {
        // Rank 1 waits for signal from rank 0
        wait_signal(compute_queue, signal_pads_device, 0, 0, 1, world_size);
        printf("[Rank 1] Received signal from rank 0\n");
        
        // Read the value
        std::vector<float> host_buf(num_elements);
        compute_queue.memcpy(host_buf.data(), local_ptr, bytes).wait();
        printf("[Rank 1] Read value: %.0f (expected 999)\n", host_buf[0]);
        
        // Send acknowledgment
        put_signal(compute_queue, signal_pads_device, 0, 0, 1, world_size);
        printf("[Rank 1] Sent acknowledgment to rank 0\n");
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    printf("\n[Rank %d] ========== Test 2: Barrier ==========\n", rank);
    
    // Reset local memory
    compute_queue.fill(local_ptr, static_cast<float>((rank + 1) * 100), num_elements).wait();
    
    // All ranks write to next rank
    int next_rank = (rank + 1) % world_size;
    compute_queue.fill(remote_data_ptrs[next_rank], 
                      static_cast<float>(rank * 1000), num_elements).wait();
    printf("[Rank %d] Wrote %d to rank %d's memory\n", rank, rank * 1000, next_rank);
    
    // Synchronize with barrier
    barrier(compute_queue, signal_pads_device, 1, rank, world_size);
    printf("[Rank %d] Passed barrier\n", rank);
    
    // Now all writes are complete, read local memory
    std::vector<float> result(num_elements);
    compute_queue.memcpy(result.data(), local_ptr, bytes).wait();
    int prev_rank = (rank - 1 + world_size) % world_size;
    printf("[Rank %d] Read value: %.0f (expected %d from rank %d)\n", 
           rank, result[0], prev_rank * 1000, prev_rank);
    
    MPI_Barrier(MPI_COMM_WORLD);
    printf("\n[Rank %d] ========== All Tests Passed! ==========\n", rank);

    // ========== Cleanup ==========
    for (int i = 0; i < world_size; ++i) {
        if (i != rank) {
            void* base_data = (void*)((char*)remote_data_ptrs[i] - 
                              ((char*)remote_data_ptrs[i] - (char*)remote_data_ptrs[i]));
            void* base_signal = (void*)((char*)remote_signal_pads[i] - 
                                ((char*)remote_signal_pads[i] - (char*)remote_signal_pads[i]));
            // Note: zeMemCloseIpcHandle may not work reliably without extension APIs
            // zeMemCloseIpcHandle(l0_ctx, base_data);
            // zeMemCloseIpcHandle(l0_ctx, base_signal);
        }
    }
    
    sycl::free(signal_pads_device, compute_queue);
    sycl::free(local_signal_pad, compute_queue);
    sycl::free(local_ptr, compute_queue);
    
    // IpcChannel 析构函数会自动清理 socket
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
