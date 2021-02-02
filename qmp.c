#include <unistd.h>
#include <stdio.h>

#include "qms.h"
#include "smf.h"

#define NEVENTS     (1 << 16)
Event midi_evs[NEVENTS];

void
qms_putsample(int16_t left, int16_t right)
{
    write(1, & left, 2);
    write(1, &right, 2);
}

int
main(int argc, char *argv[])
{
    Seeker seeker;
    unsigned int i = 0;
    int nevs = 0;
    if (argc < 2) {
        fprintf(stderr, "usage:\n  %s song.mid\n", argv[0]);
        return 1;
    }
    switch (qms_smf2evs(argv[1], midi_evs, NEVENTS, &nevs)) {
    case SMF_OK:
        fprintf(stderr, "%u events compiled\n", nevs);
        break;
    case SMF_NOFILE:
        fprintf(stderr, "file not found\n");
        break;
    case SMF_BADSIG:
        fprintf(stderr, "invalid signature\n");
        break;
    case SMF_BADFMT:
        fprintf(stderr, "unsupported track format\n");
        break;
    case SMF_BADDIV:
        fprintf(stderr, "unsupported division format\n");
        break;
    case SMF_TOOBIG:
        fprintf(stderr, "too many events\n");
        break;
    }
    qms_init();
    qms_load(&seeker, midi_evs, nevs);
    qms_seek(&seeker, 0*R);
    while (!qms_play(&seeker, R>>2)) {
        fprintf(stderr, " %c\r", "|/-\\"[i++%4]);
        fflush(stderr);
    }
    return 0;
}
