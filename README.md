我發現用FIFO再用fork再unlink fifo，如果child process沒有fclose(stdin)，就算child process一出現就exit，根本不會與stdin互動，也會導致fgets stdin出現幻覺

編譯
===
'''bash
git clone https://github.com/KHPan/SPforkTest.git
cd SPforkTest
make gcc
'''

直接使用