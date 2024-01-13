width=$(stty size | awk '{print $2}')
# run add test_* in bin/*
for i in `ls bin/test_*`
do
    printf '%*s\n' "$width" '' | tr ' ' '*'

    echo run test file: $i
    $i $@
    if [ $? -ne 0 ];then
        printf "\033[0m\033[1;31mtest $i failed\033[0m\n"
    fi
    echo
done
