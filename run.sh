gcc -c -o lab1/simsh.o lab1/simsh.c
gcc -c -o lab1/parse_command.o lab1/parse_command.c

gcc -o lab1/simsh.bin lab1/simsh.o lab1/parse_command.o 

./lab1/simsh.bin