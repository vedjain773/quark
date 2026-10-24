#!/bin/bash

total_files=0
passed=0
for file in tests/*.c
do
    expected=$(grep "EXPECTED:" "$file" | cut -d':' -f2 | xargs)
    
    echo -ne "\r\033[K\e[34m[Testing]\e[0m: $file"

    total_files=$((total_files+1))
    for mode in normal optimized
    do
        if [ "$mode" = "optimized" ]; then
            ./quark "$file" --optimize -o out/test.o
            label="[OPT]"
        else
            ./quark "$file" -o out/test.o
            label="[STD]"
        fi

        if [ $? -ne 0 ]; then
            echo -e "\033[31m [FAIL] \033[0m $label $file compiler crashed"
            continue
        fi

        gcc out/test.o -o out/test

        if [ $? -ne 0 ]; then
            echo -e "\033[31m [FAIL] \033[0m $label $file linker error"
            continue
        fi

        ./out/test
        actual=$?

        if [ "$actual" = "$expected" ]; then
          passed=$((passed+1))
        else
          echo -e "\033[31m [FAIL] \033[0m $label $file expected=$expected got=$actual"
        fi

    done
done

rm -f test.o out/test output.ll

total_tests=$((total_files*2))
echo -ne "\r\033[KTotal tests passed: $passed / $total_tests\n"
