#include "../inc/args.h"
#include "../inc/commands.h"

#include <stdio.h>
#include <stdlib.h>

void parsecmdstr(cmdinput *cmd, char *cmdstr)
{
    switch (cmd->settype)
    {
    case 'i':
        if (sscanf(cmdstr, "%i:%23[^\n]", &cmd->id, cmd->to) != 2)
        {
            fprintf(stderr, "ERROR: Failed to parse argument '%s' to type 'ID:RESULT'\n", cmdstr);
            exit(EXIT_FAILURE);
        }
        break;
    case 'n':
        if (sscanf(cmdstr, "%23[^:]:%23[^\n]", cmd->from, cmd->to) != 2)
        {
            fprintf(stderr, "ERROR: Failed to parse argument '%s' to type 'NAME:RESULT'\n", cmdstr);
            exit(EXIT_FAILURE);
        }
        break;
    case '\0':
        fprintf(stderr, "ERROR: Found no flag modifier\n");
        exit(EXIT_FAILURE);
        break;
    default:
        fprintf(stderr, "ERROR: Failed to parse flag with '%c' modifier\n", cmd->settype);
        exit(EXIT_FAILURE);
        break;
    }
}

void parseargs(input *in, int argc, char *argv[])
{
    in->demopath = NULL;
    in->mappath = NULL;
    in->outpath = NULL;
    in->print = 0;
    in->extractmap = 0;
    
    in->numcmds = 0;

    int argi;
    for (argi = 1; argi < argc; argi++)
    {
        if (argv[argi][0] == '-')
        {
            cmdinput *cmd = &in->commands[in->numcmds];
            switch (argv[argi][1])
            {
            case 'n':
                if (argi + 1 >= argc)
                {
                    fprintf(stderr, "ERROR: Flag 'n' requires additional argument\n");
                    exit(EXIT_FAILURE);
                }
                cmd->cmdtype = argv[argi][1];
                cmd->settype = argv[argi][2];
                parsecmdstr(cmd, argv[argi + 1]);
                in->numcmds++;
                argi++;
                break;
            case 's':
                if (argi + 1 >= argc)
                {
                    fprintf(stderr, "ERROR: Flag 's' requires additional argument\n");
                    exit(EXIT_FAILURE);
                }
                cmd->cmdtype = argv[argi][1];
                cmd->settype = argv[argi][2];
                parsecmdstr(cmd, argv[argi + 1]);
                in->numcmds++;
                argi++;
                break;
            case 'o':
                if (argi + 1 >= argc)
                {    
                    fprintf(stderr, "ERROR: Flag 'o' requires additional argument\n");
                    exit(EXIT_FAILURE);
                }
                in->outpath = argv[argi + 1];
                argi++;
                break;
            case 'p':
                in->print = 1;
                break;
            case 'e':
                if (argi + 1 >= argc)
                {    
                    fprintf(stderr, "ERROR: Flag 'e' requires additional argument\n");
                    exit(EXIT_FAILURE);
                }
                in->extractmap = argv[argi + 1];
                argi++;
                break;
            case 'm':
                if (argi + 1 >= argc)
                {    
                    fprintf(stderr, "ERROR: Flag 'm' requires additional argument\n");
                    exit(EXIT_FAILURE);
                }
                in->mappath = argv[argi + 1];
                argi++;
                break;
            default:
                fprintf(stderr, "ERROR: Unknown flag '%c'\n", argv[argi][1]);
                exit(EXIT_FAILURE);
                break;
            }    
        }
        else 
        {
            if (in->demopath)
            {
                fprintf(stderr, "ERROR: Found two non-flag arguments\n");
                exit(EXIT_FAILURE);
            }
            in->demopath = argv[argi];
        }
    }
    if (!in->demopath)
    {
        fprintf(stderr, "ERROR: Found no demo input\n");
        exit(EXIT_FAILURE);
    }
}
