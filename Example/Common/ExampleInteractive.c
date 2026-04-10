#include "ExampleInteractive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* const PROMPT = "SolidSyslog> ";

enum
{
    MAX_LINE_LENGTH = 256
};

static void PrintPrompt(void)
{
    printf("%s", PROMPT);
    fflush(stdout);
}

static int ParseCount(const char* args)
{
    if (args[0] == '\0')
    {
        return 1;
    }

    int count = atoi(args);
    return (count > 0) ? count : 1;
}

static void HandleSend(const char* args, const struct SolidSyslogMessage* message)
{
    int count = ParseCount(args);

    for (int i = 0; i < count; i++)
    {
        SolidSyslog_Log(message);
    }

    printf("Sent %d message%s\n", count, (count == 1) ? "" : "s");
}

void ExampleInteractive_Run(const struct SolidSyslogMessage* message, FILE* input)
{
    char line[MAX_LINE_LENGTH];

    PrintPrompt();

    while (fgets(line, sizeof(line), input) != NULL)
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        if (strcmp(line, "quit") == 0)
        {
            break;
        }

        if (strncmp(line, "send", 4) == 0)
        {
            const char* args = line + 4;
            if (*args == ' ')
            {
                args++;
            }
            HandleSend(args, message);
        }

        PrintPrompt();
    }
}
