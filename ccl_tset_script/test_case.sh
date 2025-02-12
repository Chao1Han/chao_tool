search_dir="/home/gta/hanchao/torch/"
log_dir="/home/gta/hanchao/logs"

fsdp_log_dir="$log_dir/fsdp"
fsdp2_log_dir="$log_dir/fsdp2"
ddp_log_dir="$log_dir/ddp"
dtensor_log_dir="$log_dir/dtensor"

mkdir -p "$fsdp_log_dir"
mkdir -p "$fsdp2_log_dir"
mkdir -p "$ddp_log_dir"
mkdir -p "$dtensor_log_dir"

fsdp2_tests=(
    "test/distributed/_composable/fsdp/test_fully_shard_autograd.py"
    "test/distributed/_composable/fsdp/test_fully_shard_compile.py"
    "test/distributed/_composable/fsdp/test_fully_shard_grad_scaler.py"
    "test/distributed/_composable/fsdp/test_fully_shard_memory.py"
    "test/distributed/_composable/fsdp/test_fully_shard_state_dict.py"
    "test/distributed/_composable/fsdp/test_fully_shard_clip_grad_norm_.py"
    "test/distributed/_composable/fsdp/test_fully_shard_extensions.py"
    "test/distributed/_composable/fsdp/test_fully_shard_init.py"
    "test/distributed/_composable/fsdp/test_fully_shard_mixed_precision.py"
    "test/distributed/_composable/fsdp/test_fully_shard_state.py"
    "test/distributed/_composable/fsdp/test_fully_shard_comm.py"
    "test/distributed/_composable/fsdp/test_fully_shard_frozen.py"
    "test/distributed/_composable/fsdp/test_fully_shard_logging.py"
    "test/distributed/_composable/fsdp/test_fully_shard_overlap.py"
    "test/distributed/_composable/fsdp/test_fully_shard_training.py"
)

fsdp_tests=(
    "test/distributed/fsdp/test_distributed_checkpoint.py"
    "test/distributed/fsdp/test_fsdp_apply.py"
    "test/distributed/fsdp/test_fsdp_checkpoint.py"
    "test/distributed/fsdp/test_fsdp_clip_grad_norm.py"
    "test/distributed/fsdp/test_fsdp_comm_hooks.py"
    "test/distributed/fsdp/test_fsdp_comm.py"
    "test/distributed/fsdp/test_fsdp_core.py"
    "test/distributed/fsdp/test_fsdp_dtensor_state_dict.py"
    "test/distributed/fsdp/test_fsdp_exec_order.py"
    "test/distributed/fsdp/test_fsdp_fine_tune.py"
    "test/distributed/fsdp/test_fsdp_freezing_weights.py"
    "test/distributed/fsdp/test_fsdp_fx.py"
    "test/distributed/fsdp/test_fsdp_hybrid_shard.py"
    "test/distributed/fsdp/test_fsdp_ignored_modules.py"
    "test/distributed/fsdp/test_fsdp_memory.py"
    "test/distributed/fsdp/test_fsdp_misc.py"
    "test/distributed/fsdp/test_fsdp_mixed_precision.py"
    "test/distributed/fsdp/test_fsdp_multiple_forward.py"
    "test/distributed/fsdp/test_fsdp_optim_state.py"
    "test/distributed/fsdp/test_fsdp_overlap.py"
    "test/distributed/fsdp/test_fsdp_pure_fp16.py"
    "test/distributed/fsdp/test_fsdp_state_dict.py"
    "test/distributed/fsdp/test_fsdp_traversal.py"
    "test/distributed/fsdp/test_fsdp_uneven.py"
    "test/distributed/fsdp/test_fsdp_use_orig_params.py"
    "test/distributed/fsdp/test_hsdp_dtensor_state_dict.py"
)

ddp_tests=(
    "test/distributed/test_backends.py"
    "test/distributed/test_c10d_common.py"
    "test/distributed/test_c10d_functional_native.py"
    "test/distributed/test_c10d_logger.py"
    "test/distributed/test_c10d_object_collectives.py"
    "test/distributed/test_c10d_spawn.py"
    "test/distributed/test_composability.py"
    "test/distributed/test_control_collectives.py"
    "test/distributed/test_data_parallel.py"
    "test/distributed/test_device_mesh.py"
    "test/distributed/test_distributed_spawn.py"
    "test/distributed/test_dynamo_distributed.py"
    "test/distributed/test_fake_pg.py"
    "test/distributed/test_functional_api.py"
    "test/distributed/test_inductor_collectives.py"
    "test/distributed/test_multi_threaded_pg.py"
    "test/distributed/test_pg_wrapper.py"
    "test/distributed/test_store.py"
    "test/distributed/test_symmetric_memory.py"
)

dtensor_tests=(
    "test/distributed/tensor/debug/test_comm_mode.py"
    "test/distributed/tensor/parallel/test_micro_pipeline_tp.py"
    "test/distributed/tensor/parallel/test_parallelize_api.py"
    "test/distributed/tensor/parallel/test_tp_random_state.py"
    "test/distributed/tensor/test_attention.pyy"
    "test/distributed/tensor/test_dtensor.py"
    "test/distributed/tensor/test_dtensor_compile.py"
    "test/distributed/tensor/test_matrix_ops.py"
    "test/distributed/tensor/test_random_ops.py"
    "test/distributed/tensor/test_redistribute.py"
)

run_tests() {
    local test_list=("$@")
    local log_dir="$1"
    shift
    for test_file in "$@"; do
        full_path="${search_dir}/${test_file}"
        log_file="${log_dir}/$(basename ${test_file%.py}).log"
        
        echo "Running test: $full_path"
        echo "Log file: $log_file"
        
        python "$full_path" > "$log_file" 2>&1
    done
    python /home/gta/hanchao/get_result.py --file_path "$log_dir"
    python /home/gta/hanchao/calculate_pass_rate.py --summary_log_path "${log_dir}/summary.log"
}

run_tests "$fsdp2_log_dir" "${fsdp2_tests[@]}"
run_tests "$fsdp_log_dir" "${fsdp_tests[@]}"
run_tests "$ddp_log_dir" "${ddp_tests[@]}"
run_tests "$dtensor_log_dir" "${dtensor_tests[@]}"