for f in data/*
do 
    if [ -d "$f" ]; then
        rm $f/coefficients.txt
        rm -rf $f/output-images
    fi
done
