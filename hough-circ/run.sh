#!/bin/sh


#export PYTHONPATH=$PYTHONPATH:/usr/python/:/usr/python/caffe:/usr/python/caffe/test:/usr/python/caffe/proto:/usr/python/caffe/imagenet

rm -rf ./result
mkdir -p result

python -uB main.py | tee result/out.log

# for f in data/*; do
#    echo $f
#    python main.py $f
# done


