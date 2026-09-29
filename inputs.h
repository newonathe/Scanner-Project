#ifndef INPUTS_H
#define INPUTS_H

/* Process named files, or every *_input.txt file in the current directory. */
int runinputs(int argc, char **argv, int (*process)(const char *));

#endif
