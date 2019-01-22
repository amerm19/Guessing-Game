#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int MIN_NUM = 0;
int MAX_NUM = 1023;
int guessed_num = -1;
int next_round = 1;
int num_guess = 0;
int parent_catch_sigint = 0;

pid_t sender_pid;
pid_t recv_pid;

void sig1handler(int);
void sig2handler(int);
void sigpinthandler(int);
void sigcinthandler(int);

int check_input(int n) {

	if (n >= 0 && n <= 1023) {
		return 1; // true
	}
	return 0; //return false
}

int is_correct(char *str) {

	if (strcmp(str, "correct") == 0) {
		return 1;
	}
	return 0;

}

int valid_str(char *str) {

    if ( (strcmp(str, "correct") == 0) || (strcmp(str, "hi") == 0) || (strcmp(str, "lo") == 0) ) {
        return 1;
    }
    return 0;

}

int main() {

	int secret_num = 0;

	int ret = fork();
	if (ret < 0) {
		perror("fork");
		exit(1);
	}
	// Child process
	if (ret == 0) {

		sender_pid = getppid();
		recv_pid = getpid();

		printf("Input secret number from 0 - 1023 (inclusive): ");
		scanf("%d", &secret_num);

		while(check_input(secret_num) != 1) {
			printf("Invalid input. Please choose between 0 - 1023 (inclusive): ");
			scanf("%d", &secret_num);
		}

		kill(sender_pid, SIGCONT);

		int sig;
		char user_input[8] = {'s', 't', 'a', 'r', 't', '\0', '\0', '\0'};
		while (is_correct(user_input) != 1) {

			sigset_t waitset;
			// Initialize wait set.
			sigemptyset(&waitset);
			sigaddset(&waitset, SIGCONT);
			sigprocmask(SIG_BLOCK, &waitset, NULL);

			sigwait(&waitset, &sig);

			printf("Hint (hi, lo, correct): ");
			scanf("%s", user_input);
			while (valid_str(user_input) != 1) {
				printf("Hint: ");
				scanf("%s", user_input);
			}
			if (strcmp(user_input, "hi") == 0) {
				if(kill(sender_pid, SIGCONT) < 0) {
					perror("kill");
				}
				if(kill(sender_pid, SIGUSR1) < 0) {
					perror("kill");
				}
			} else if (strcmp(user_input, "lo") == 0) {
				if(kill(sender_pid, SIGCONT)) {
					perror("kill");
				}
				if(kill(sender_pid, SIGUSR2)){
					perror("kill");
				}
			}

		}

		if(kill(sender_pid, SIGINT)) {
			perror("kill");
		}
		if(kill(sender_pid, SIGCONT)) {
			perror("kill");
		}
		signal(SIGINT, sigcinthandler);


	} else { // parent process.

		int sig;

		sender_pid = ret;
		recv_pid = getpid();

		sigset_t waitset;
		// Initialize wait set.
		sigemptyset(&waitset);
		sigaddset(&waitset, SIGCONT);
		sigprocmask(SIG_BLOCK, &waitset, NULL);

		sigwait(&waitset, &sig);

		signal(SIGUSR1, sig1handler);
		signal(SIGUSR2, sig2handler);
		signal(SIGINT, sigpinthandler);

		int ran_num = 1;
		while (1){
			if (next_round) {
				sig = 0;
				sigset_t waitset;
				// Initialize wait set.
				sigemptyset(&waitset);
				sigaddset(&waitset, SIGCONT);
				sigprocmask(SIG_BLOCK, &waitset, NULL);
			
				ran_num = MIN_NUM + rand() / (RAND_MAX / (MAX_NUM - MIN_NUM + 1) + 1);

				guessed_num = ran_num;
				printf("Guess: %d\n", guessed_num);

				next_round = 0;

				kill(sender_pid, SIGCONT);
				sigwait(&waitset, &sig);
			} else if (parent_catch_sigint == 1) {
				if(kill(sender_pid, SIGINT)) {
					perror("kill");
				}
			}
		}

	}

	return 0;
}

void sig1handler(int signum) {
	fprintf(stderr, "%d # SIGUSR1 # %d\n", sender_pid, recv_pid);
	MAX_NUM = guessed_num - 1;
	num_guess++;
	next_round = 1;

}

void sig2handler(int signum) {
	fprintf(stderr, "%d # SIGUSR2 # %d\n", sender_pid, recv_pid);
	MIN_NUM = guessed_num + 1;
	num_guess++;
	next_round = 1;
}

void sigpinthandler(int signum) {
	fprintf(stderr, "%d # SIGINT # %d\n", sender_pid, recv_pid);
	printf("Number of guess: %d\n", num_guess);
	parent_catch_sigint = 1;
}

void sigcinthandler(int signum) {
	fprintf(stderr, "%d # SIGINT # %d\n", sender_pid, recv_pid);
	kill(sender_pid, SIGKILL);
	exit(0);
}