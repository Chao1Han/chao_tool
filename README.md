## Docker
```bash
# build docker
bash docker/build.sh

# run docker
bash run_docker.sh

# into docker
docker exec -it ipex:driver602 /bin/bash

# stop docker
docker stop ipex:driver602 ; docker rm ipex:driver602
```

## Log2excel
A tool transfor pytorch profile table to excel.
***Note: Please use it in windows***

## Requriement
```bash
python -m pip install xlwt
```

## Example
Put your profile result to `raw_log.txt`, strip head and tail, your `raw_log.txt` content may like this:
```
aten::copy_        93.76%     452.662ms        93.76%     452.662ms       2.219ms      58.870ms        12.59%      58.870ms     288.578us           204  
trans_matmul         0.31%       1.487ms         0.63%       3.053ms     127.215us      53.287ms        11.39%      85.977ms       3.582ms            24  
aten::bmm         0.31%       1.475ms         0.32%       1.557ms      64.889us      53.270ms        11.39%      53.270ms       2.220ms            24  
linear_gelu         0.41%       2.002ms         0.47%       2.293ms      95.523us      52.829ms        11.29%      52.829ms       2.201ms            24  
```
just run `python log2excel.py` and get result in `datatable.xls`
