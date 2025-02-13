Gia na treksoume to bandwidth test:
1o terminal:
    make test;./test -s -m -p 54321 -f save.txt
2o terminal:
    ./test -m  -p 54321 -a 192.168.1.5 -f big.txt
