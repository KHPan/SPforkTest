我發現用FIFO再用fork再unlink fifo，如果child process沒有fclose(stdin)，就算child process一出現就exit，根本不會與stdin互動，也會導致fgets stdin出現幻覺

編譯
===
'''bash
git clone https://github.com/KHPan/SPforkTest.git
cd SPforkTest
make gcc
'''

直接使用
'''bash
./no < in.txt > out.txt
'''
這是沒有fclose的版本，in.txt的內容會重複出現

有fclose:
'''bash
./yes < in.txt > out.txt
'''

去掉mkfifo:
'''bash
./no_mkfifo < in.txt > out.txt
'''

去掉unlink:
'''bash
./no_unlink < in.txt > out.txt
'''

可以在執行檔之後的argv設定Adopt執行的位置