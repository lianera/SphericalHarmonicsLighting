degree=3
samplenum=1000000
write_rendered=
#write_rendered="--write-rendered"
for f in data/*
do 
    if [ -d "$f" ]; then
        echo "===== processing $f ====="
        Release/sampler.exe $f jpg $degree $samplenum $write_rendered
    fi
done
