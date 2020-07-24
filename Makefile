CC = gcc
CFLAGS = -O2 -g -ggdb
main:
	${CC} main.c -o main.o ${CFLAGS}

clean:
	rm -v main.o

test: clean main
	@echo "Running tests..."
	@echo "[writeonly 1]"
	cat tests/writeonly/Write_Only_1_input.txt | ./main.o > ./test_output
	diff ./tests/writeonly/Write_Only_1_output.txt ./test_output
	@echo "[writeonly 2]"
	cat tests/writeonly/Write_Only_2_input.txt | ./main.o > ./test_output
	diff ./tests/writeonly/Write_Only_2_output.txt ./test_output

	@echo "[bulkreads 1]"
	cat tests/bulkreads/Bulk_Reads_1_input.txt | ./main.o > ./test_output
	diff ./tests/bulkreads/Bulk_Reads_1_output.txt ./test_output
	@echo "[bulkreads 2]"
	cat tests/bulkreads/Bulk_Reads_2_input.txt | ./main.o > ./test_output
	diff ./tests/bulkreads/Bulk_Reads_2_output.txt ./test_output

	@echo "[timeforachange 1]"
	cat tests/timeforachange/Time_for_a_change_1_input.txt | ./main.o > ./test_output
	diff ./tests/timeforachange/Time_for_a_change_1_output.txt ./test_output
	@echo "[timeforachange 2]"
	cat tests/timeforachange/Time_for_a_change_2_input.txt | ./main.o > ./test_output
	diff ./tests/timeforachange/Time_for_a_change_2_output.txt ./test_output

	@echo "[rollingback 1]"
	cat tests/rollingback/Rolling_Back_1_input.txt | ./main.o > ./test_output
	diff ./tests/rollingback/Rolling_Back_1_output.txt ./test_output
	@echo "[rollingback 2]"
	cat tests/rollingback/Rolling_Back_2_input.txt | ./main.o > ./test_output
	diff ./tests/rollingback/Rolling_Back_2_output.txt ./test_output

	@echo "[alteringhistory 1]"
	cat tests/alteringhistory/Altering_History_1_input.txt | ./main.o > ./test_output
	diff ./tests/alteringhistory/Altering_History_1_output.txt ./test_output
	@echo "[alteringhistory 2]"
	cat tests/alteringhistory/Altering_History_2_input.txt | ./main.o > ./test_output
	diff ./tests/alteringhistory/Altering_History_2_output.txt ./test_output
	
	@echo "[rollercoaster 1]"
	cat tests/rollercoaster/Rollercoaster_1_input.txt | ./main.o > ./test_output
	diff ./tests/rollercoaster/Rollercoaster_1_output.txt./test_output
	@echo "[rollercoaster 2]"
	cat tests/rollercoaster/Rollercoaster_2_input.txt | ./main.o > ./test_output
	diff ./tests/rollercoaster/Rollercoaster_2_output.txt./test_output