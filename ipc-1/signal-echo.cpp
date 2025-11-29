#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <ucontext.h>

void sig_handler(int signum, siginfo_t* siginfo, void* context) {
	ucontext_t* ctx = (ucontext_t*)context;
	unsigned long RIP = ctx->uc_mcontext.gregs[REG_RIP];
	unsigned long RAX = ctx->uc_mcontext.gregs[REG_RAX];
	unsigned long RBX = ctx->uc_mcontext.gregs[REG_RBX];

	uid_t uid = siginfo->si_uid;
	pid_t pid = siginfo->si_pid;
	
	passwd* pwd = getpwuid(uid);
	const char* username = pwd? pwd->pw_name : "USER";
	std::cout << "	RECEIVED A SIGUSR1 SIGNAL FROM PROCESS " << pid << " EXECUTED BY " << uid << "[ " << username << " ]\n";
	std::cout << "STATE OF CONTEXT\n : RIP = "<< RIP << '\n' << " RAX = " << RAX << '\n' << "RBX = " << RBX << '\n';

}

void thanks(int signum, siginfo_t* siginfo, void* context){

	std::cout << "THANKS A LOT HAVE A NICE DAY|N";
	exit(0);
}

int main () {

	pid_t thispid = getpid();
	std::cout <<"HI THRERE I AM : " << thispid << '\n';
	std::cout << "I AM SLEEPING WHILE(TRUE) I AM USELESS KILL ME\n";
	std::cout << "if i am not running in the background paste this :[kill -USR1 "<< thispid << "] into another terminal" << std::endl;
	struct sigaction sa{};
	sa.sa_sigaction = sig_handler;
	sa.sa_flags = SA_SIGINFO;
	struct sigaction sb{};
	sb.sa_sigaction = thanks;
	sb.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sa, nullptr);
	sigaction(SIGINT, &sb, nullptr);
	sigaction(SIGTERM, &sb, nullptr);
	sigaction(SIGABRT, &sb, nullptr);
	while(true){
		sleep(10);
	}
	return 0;

}
