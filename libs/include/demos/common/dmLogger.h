#pragma once

typedef struct DmLogger DmLogger;

DmLogger* InitLogger(const char* logfile);
void CloseLogger(DmLogger* logger);

void LogInfo(DmLogger* logger, const char* message);
void LogWarning(DmLogger* logger, const char* message);
void LogError(DmLogger* logger, const char* message);
void LogDebug(DmLogger* logger, const char* message);

/*
* ------------------------------------
*	IMPLEMENTATION
* ------------------------------------
*/
#ifdef DM_LOGGER_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include "SDL2/SDL_log.h"

#define BUFFER_SIZE 2048

typedef enum
{
	DM_STDOUT = 1,
	DM_FILE
}DM_Output;

typedef struct DmLogger {
	DM_Output prefferedOutput;  //STDOUT or to a FILE
	int ncached;
	char buffer[BUFFER_SIZE];
	FILE* fp;
}DmLogger;

static void LogOutput(void* userdata, int category, SDL_LogPriority priority, const char* message) {
	if (!userdata)
		return;

	DmLogger* l = (DmLogger*)userdata;
	
	switch (l->prefferedOutput)
	{
	case DM_FILE:
		fwrite(message, 1, sizeof(message), l->fp);
		break;
	case DM_STDOUT:
		printf("%s", message);
		break;
	default:
		break;
	}
}

DmLogger* InitLogger(const char* logfile) {
	DmLogger* l = NULL;

	l = (DmLogger*)malloc(sizeof(DmLogger));
	if (l) {
		l->prefferedOutput = DM_FILE;
		l->ncached = 0;
		l->fp = NULL;

		l->fp = fopen(logfile, "w");
		if (l->fp) {
			setbuf(l->fp, l->buffer);
			SDL_LogSetOutputFunction(LogOutput, (void*)l);
			SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
			return l;
		}
		printf("Failed to create log file, defaulting to STDOUT");
		l->prefferedOutput = DM_STDOUT;
		return l;
	}
	printf("Failed to create logger");

	return l;
}

void CloseLogger(DmLogger* logger) {
	if (!logger)
		return;

	if (logger->fp)
	{
		fflush(logger->fp);
		fclose(logger->fp);
	}

	free(logger);
}

void LogInfo(DmLogger* logger, const char* message) {
	size_t len = strlen(message);
	if (logger->ncached + len >= BUFFER_SIZE) {
		fflush(logger->fp);
		logger->ncached = 0;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[INFO]  %s", message);
	logger->ncached += len;
}

#endif // DM_LOGGER_IMPLEMENTATION
