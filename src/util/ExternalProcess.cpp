/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "ExternalProcess.h"

#include <signal.h>
#include <unistd.h>

#include <sys/wait.h>


ExternalProcess::ExternalProcess()
	:
	fChildPid(-1)
{
}


ExternalProcessResult
ExternalProcess::Run(const std::vector<BString>& argv,
	ExternalProcessOutputCallback callback, void* cookie)
{
	ExternalProcessResult result;
	result.exitCode = -1;

	if (argv.empty())
		return result;

	int outPipe[2];
	if (pipe(outPipe) != 0)
		return result;

	pid_t pid = fork();
	if (pid < 0) {
		close(outPipe[0]);
		close(outPipe[1]);
		return result;
	}

	if (pid == 0) {
		// Processo figlio: stdout e stderr vanno nella stessa pipe, cosi'
		// il chiamante vede l'output combinato nell'ordine in cui e' stato
		// scritto (utile per i messaggi di progresso di cdrecord in
		// Milestone 6).
		close(outPipe[0]);
		dup2(outPipe[1], STDOUT_FILENO);
		dup2(outPipe[1], STDERR_FILENO);
		close(outPipe[1]);

		std::vector<char*> cArgv;
		for (size_t i = 0; i < argv.size(); i++)
			cArgv.push_back(const_cast<char*>(argv[i].String()));
		cArgv.push_back(NULL);

		execvp(cArgv[0], cArgv.data());
		_exit(127); // execvp fallita (programma non trovato)
	}

	// Processo genitore.
	fChildPid = pid;
	close(outPipe[1]);

	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = read(outPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
		buffer[bytesRead] = '\0';
		result.output << buffer;
		if (callback != NULL)
			callback(buffer, (size_t)bytesRead, cookie);
	}
	close(outPipe[0]);

	int status = 0;
	waitpid(pid, &status, 0);
	fChildPid = -1;

	result.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	return result;
}


void
ExternalProcess::Cancel()
{
	pid_t pid = (pid_t)fChildPid;
	if (pid > 0)
		kill(pid, SIGTERM);
}
