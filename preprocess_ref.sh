pfp=<pfp_thresholds binary>
bconstructor=<build_constructor binary>
rconstructor=<run_constructor binary>
prepare_ref="build/prepare_ref"
movi="build/movi"
t="/usr/bin/time -o "

fasta_list="$2"
index_dir="$3"
clean_fasta="$index_dir/ref.fa"
mkdir ${index_dir}

cmd="$t $index_dir/build.prepare_ref.time $prepare_ref $fasta_list $clean_fasta list"
echo $cmd
eval $cmd

cmd="$t $index_dir/build.pfp.time $pfp -f $clean_fasta"
echo $cmd
eval $cmd

if [ "$1" == "split" ]; then
    cmd="$t $index_dir/build_constructor.time $bconstructor $clean_fasta"
    echo $cmd
    eval $cmd
    cmd="$t $index_dir/run_constructor.time $rconstructor $clean_fasta -d 5"
    echo $cmd
    eval $cmd
    cmd="$t $index_dir/build.movi.time $movi build reg $clean_fasta $index_dir"
    echo $cmd
    eval $cmd
fi

if [ "$1" == "reg" ]; then
    cmd="$t $index_dir/build.movi.time $movi build reg $clean_fasta $index_dir"
    echo $cmd
    eval $cmd
fi