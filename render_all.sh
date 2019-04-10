
envs=""
N=0
objs=""
M=0
for f in data/*
do 
    if [ -d "$f" ]; then
        echo "add $f"
        envs="$envs $f jpg"
        ((N+=1))
    else
        objs="$objs $f"
        ((M+=1))
    fi
done

Release/lighting.exe $N $envs $M $objs