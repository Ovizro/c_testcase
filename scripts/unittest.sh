width=$(stty size | awk '{print $2}')
# run add test_* in bin/*
for i in `ls bin/test_*`
do
    echo
    printf '%*s\n' "$width" '' | tr ' ' '*'

    echo run test file: $i
    $i $@
    if [ $? -ne 0 ]; then
        echo "\033[0m\033[1;31mtest $i failed\033[0m"
    fi
    echo
done

# printf '%*s\n' "$width" '' | tr ' ' '*'
