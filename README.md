# movi

movi is a full text index for indexing large pangenomes. It is designed based on move data structure.

**Build from Source**

To build from source:
```
git clone https://github.com/mohsenzakeri/movi
cd movi
mkdir build
cd build
cmake ..
make
```

Building the movi index on a fasta file requires preprocessing the fasta using the [pfp_thresholds](https://github.com/maxrossi91/pfp-thresholds) software. Please download and install [pfp_thresholds](https://github.com/maxrossi91/pfp-thresholds) before proceeding to the next step.

After the installation, please edit the `preprocess_ref.sh` script in the main directory of movi to include the path to the `pfp_thresholds` binary in the first line:
```
pfp=<PATH TO PFP_THRESHOLDS BINARY>
```

Please also make sure the paths to the `movi` and `prepare_ref` binaries (built in the `build` directory after the installation) is also correct according to where the binaries are located on your system:
```
movi=<PATH TO MOVI BINARY>
prepare_ref=<PATH TO PREPARE_REF BINARY>
```

**Build the movi index**

After including the correct paths in the script, you may run the script as following to build the default movi index:
```
bash preprocess_ref.sh reg <fasta list file> <index directory>
```
`<fasta list file>` is a file which contains the address of all the reference fasta files to be indexed with movi. Each line in the list file should be the address of a separate fasta file.

`<index directory>` is the directory where you want the movi index to be located.


**Compute Pseudo Matching Lengths (PML) using movi**

To compute PMLs using movi, please run the following command on the fastq or fasta file of the reads:
```
movi query <index directory> <reads file>
```
`<reads file>` is the address of the fasta or fastq file containing the reads.

After the query command finishes, a file with the same name as the read file and the extension `mpml.bin` is generated in the directory that also includes the reads file.
Since this file is in the binary format, to view the PMLs please run the following command:
```
movi view <mpml file> | less
```
`<mpml file>` is the file generated in the query step.