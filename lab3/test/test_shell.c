/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "screen.h"
#include "stdio.h"
#include "syscall.h"
#include "test.h"

#include "common.h"
#include "string.h"

#ifdef P3_TEST

struct task_info task0;

struct task_info task1 = { "task1", (uint64_t)&ready_to_exit_task, USER_PROCESS,
			   1 };
struct task_info task2 = { "task2", (uint64_t)&wait_lock_task, USER_PROCESS,
			   1 };
struct task_info task3 = { "task3", (uint64_t)&wait_exit_task, USER_PROCESS,
			   1 };

struct task_info task4 = { "task4", (uint64_t)&semaphore_add_task1,
			   USER_PROCESS, 1 };
struct task_info task5 = { "task5", (uint64_t)&semaphore_add_task2,
			   USER_PROCESS, 1 };
struct task_info task6 = { "task6", (uint64_t)&semaphore_add_task3,
			   USER_PROCESS, 1 };

struct task_info task7 = { "task7", (uint64_t)&producer_task, USER_PROCESS, 1 };
struct task_info task8 = { "task8", (uint64_t)&consumer_task1, USER_PROCESS,
			   1 };
struct task_info task9 = { "task9", (uint64_t)&consumer_task2, USER_PROCESS,
			   1 };

struct task_info task10 = { "task10", (uint64_t)&barrier_task1, USER_PROCESS,
			    1 };
struct task_info task11 = { "task11", (uint64_t)&barrier_task2, USER_PROCESS,
			    1 };
struct task_info task12 = { "task12", (uint64_t)&barrier_task3, USER_PROCESS,
			    1 };

struct task_info task13 = { "SunQuan", (uint64_t)&SunQuan, USER_PROCESS, 1 };
struct task_info task14 = { "LiuBei", (uint64_t)&LiuBei, USER_PROCESS, 1 };
struct task_info task15 = { "CaoCao", (uint64_t)&CaoCao, USER_PROCESS, 1 };

#ifdef P4_TEST
struct task_info task16 = { "mem_test1", (uint64_t)&rw_task1, USER_PROCESS, 1 };
struct task_info task17 = { "plan", (uint64_t)&drawing_task1, USER_PROCESS, 1 };
#endif

#ifdef P5_TEST
struct task_info task18 = { "mac_send", (uint64_t)&mac_send_task, USER_PROCESS,
			    1 };
struct task_info task19 = { "mac_recv", (uint64_t)&mac_recv_task, USER_PROCESS,
			    1 };
#endif

#ifdef P6_TEST

struct task_info task19 = { "fs_test", (uint64_t)&test_fs, USER_PROCESS, 1 };
#endif
struct task_info task16 = { "multicore", (uint64_t)&test_multicore,
			    USER_PROCESS, 1 };
static struct task_info *test_tasks[NUM_MAX_TASK] = {
	&task0,	 &task1,  &task2,  &task3,  &task4,  &task5,
	&task6,	 &task7,  &task8,  &task9,  &task10, &task11,
	&task12, &task13, &task14, &task15, &task16
};

#endif

char *command_boundary = "> -------------- COMMAND --------------";
char *user_name = "> ROOT@UCAS_OS$ ";

void hint_print()
{
	screen_move_cursor(pcb[0].cursor_x, ++pcb[0].cursor_y);
	printf("[INFO] Input format: [ps/clear] | [exec/wait/kill] <task_num> \n");
}

int get_num(char in_buf[])
{
	int len = strlen(in_buf);
	int i, flag = 0, ret = 0;
	for (i = 0; i < len; i++) {
		if (flag == 0) {
			if (in_buf[i] != ' ')
				continue;
			else
				flag = 1;
		} else if (in_buf[i] >= '0' && in_buf[i] <= '9') {
			ret = ret * 10 + (in_buf[i] - '0');
		}
	}
	return ret;
}

void test_shell()
{
	//TODO ok

	screen_move_cursor(pcb[0].cursor_x, pcb[0].cursor_y);
	printf("%s\n", command_boundary); // show boundary
	screen_move_cursor(pcb[0].cursor_x, ++pcb[0].cursor_y);
	printf("%s", user_name); // show username

	char in_buf[IN_LEN_MAX] = { 0 };
	int in_id = 0;

	while (1) {
		char in = sys_read_keyboard();
		if (in == 0) // do not recv ch
			continue;
		else if (in != '\r') {
			if (in == 8) { // backspace
				if (in_id > 0) {
					printf("%c", in);
					in_buf[in_id--] = '\0';
				}
			} else if (in_id < IN_LEN_MAX) { // not backspace
				printf("%c", in);
				in_buf[in_id++] = in;
			}
		} else { // parsing buffer when '\r'
			printf("%c", in);
			in_buf[in_id] = '\0';
			if (memcmp(in_buf, "ps", 2) == 0 &&
			    in_buf[2] == '\0') { // ps
				pcb[0].cursor_x = SHELL_LEFT_LOC;
				screen_move_cursor(pcb[0].cursor_x,
						   ++pcb[0].cursor_y);
				sys_process_show();

				// pcb[0].cursor_y++;
			} else if (memcmp(in_buf, "clear", 5) == 0 &&
				   in_buf[5] == '\0') { // clear
				sys_screen_clear(SHELL_BOUNDARY + 1,
						 SCREEN_HEIGHT);

				pcb[0].cursor_y = SHELL_BOUNDARY + 1;
			} else if (memcmp(in_buf, "kill", 4) == 0) { //kill
				int be_kill = get_num(in_buf);

				int has_killed =
					(be_kill > 0) ?
						sys_kill((pid_t)be_kill) :
						(-1);
				pcb[0].cursor_x = SHELL_LEFT_LOC;
				screen_move_cursor(pcb[0].cursor_x,
						   ++pcb[0].cursor_y);
				if (has_killed == 0)
					printf("PROCESS (pid=%d) has been KILLED.\n",
					       be_kill);
				else if (has_killed == -1)
					printf("PROCESS NOT EXISTED.\n");

				pcb[0].cursor_y++;
			} else if (memcmp(in_buf, "wait", 4) == 0) { //wait
				int wait_who = get_num(in_buf);

				int has_waited =
					(wait_who > 0) ?
						sys_waitpid((pid_t)wait_who) :
						(-1);
				pcb[0].cursor_x = SHELL_LEFT_LOC;
				if (has_waited == -1) {
					screen_move_cursor(pcb[0].cursor_x,
							   ++pcb[0].cursor_y);
					printf("PROCESS %d NOT EXISTED.\n",
					       wait_who);
				}

				pcb[0].cursor_y++;
			} else if (memcmp(in_buf, "exec", 4) == 0) { //exec
				int be_exec = get_num(in_buf);

				int has_exec;
				if (be_exec > 0 && be_exec < 16)
					has_exec =
						sys_spawn(test_tasks[be_exec]);
				else
					has_exec = -1; // invalid task

				pcb[0].cursor_x = SHELL_LEFT_LOC;
				screen_move_cursor(pcb[0].cursor_x,
						   ++pcb[0].cursor_y);
				if (has_exec == -1) {
					printf("Task %d EXEC FAILED/NOT EXISTED.\n",
					       be_exec);
				} else {
					printf("Task %d EXEC SUCCESS.\n",
					       be_exec);
				}

				pcb[0].cursor_y++;
			} else { // error
				printf("%c", in);
                pcb[0].cursor_x = SHELL_LEFT_LOC;
				hint_print();
			}

			pcb[0].cursor_x = SHELL_LEFT_LOC;
			screen_move_cursor(pcb[0].cursor_x, pcb[0].cursor_y);
			in_id = 0;
			memset(in_buf, 0, IN_LEN_MAX * (sizeof(char)));
			printf("%s", user_name); // show username
		}
	}
}
