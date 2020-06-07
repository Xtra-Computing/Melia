HashjoinOnHARP
Prerequisites
The gcc-4.8 or above
The Hardware Accelerator Research Program (HARP) v2 platform at (https://wikis.uni-paderborn.de/pc2doc/HARP2).
You may need to be granted to access this platform.
Run the code
$ cd ./
$ make  # make the host execution program
$ aoc ./device/shj.cl -g --report --board bdw_fpga_v1.0 -o ./bin/shj.aocx  # make the FPGA execution program. It takes time.
$ cd ./bin
$ ./host
Cite this work
If you use it in your paper, please cite our work (full version).

@article{chenfpga,
  title={Is FPGA Useful for Hash Joins?},
  author={Chen, Xinyu and Chen, Yao and Bajaj, Ronak and He, Jiong and He, Bingsheng and Wong, Weng-Fai and Chen, Deming},
  year={2020},
  booktitle={Conference on Innovative Data Systems Research (CIDR), 2020},
}

Related publications
Xinyu Chen*, Ronak Bajaj^, Yao Chen, Jiong He, Bingsheng He, Weng-Fai Wong and Deming Chen. On-The-Fly Parallel Data Shuffling for Graph Processing on OpenCL-based FPGAs. FPL, 2019.
Related systems
Graph systems on GPU: G3 | Medusa
Other Thunder-series systems in Xtra NUS: ThunderGBM | ThunderSVM
