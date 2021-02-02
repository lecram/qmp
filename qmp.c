#include <unistd.h>
#include <stdio.h>
#include <termios.h>

#include "qms.h"
#include "smf.h"

#define NEVENTS     (1 << 16)
Event midi_evs[NEVENTS];

#define SEG_LEN 4410

struct termios term_prev;

void
setup_terminal(struct termios *term_prev)
{
    struct termios term_raw;

    /* disable echo and canonical mode */
    tcgetattr(0, term_prev);
    term_raw = *term_prev;
    term_raw.c_lflag &= ~(ECHO | ICANON);
    /* async read */
    term_raw.c_cc[VMIN] = 0;
    term_raw.c_cc[VTIME] = 0; /* in deciseconds */
    tcsetattr(0, TCSAFLUSH, &term_raw);
}

void
restore_terminal(struct termios *term_prev)
{
    tcsetattr(0, TCSAFLUSH, term_prev);
}

void
qms_putsample(int16_t left, int16_t right)
{
    write(1, & left, 2);
    write(1, &right, 2);
}

void
putsilence(unsigned int nsamples)
{
    while (nsamples--)
        qms_putsample(0, 0);
}

#define seek_bck(n) qms_seek(&seeker, seeker.smp_i > (n) ? seeker.smp_i - (n) : 0)
#define seek_fwd(n) qms_seek(&seeker, max_smp_i - seeker.smp_i > (n) ? seeker.smp_i + (n) : max_smp_i-1);

int
main(int argc, char *argv[])
{
    Seeker seeker;
    unsigned int cur_sec, max_sec, max_smp_i;
    char key;
    int paused, quit;
    int nevs = 0;
    if (argc < 2) {
        fprintf(stderr, "usage:\n  %s song.mid\n", argv[0]);
        return 1;
    }
    switch (qms_smf2evs(argv[1], midi_evs, NEVENTS, &nevs)) {
    case SMF_OK:
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
    setup_terminal(&term_prev);
    max_smp_i = midi_evs[nevs-1].offset;
    max_sec = max_smp_i / R;
    qms_init();
    qms_load(&seeker, midi_evs, nevs);
    qms_seek(&seeker, 0);
    paused = quit = 0;
    while (!quit) {
        if (!paused) {
            quit = qms_play(&seeker, SEG_LEN);
            cur_sec = seeker.smp_i / R;
            fprintf(stderr, " %02u:%02u/%02u:%02u\r",
                    cur_sec/60, cur_sec%60, max_sec/60, max_sec%60);
            fflush(stderr);
        } else {
            putsilence(SEG_LEN);
        }
        if (read(0, &key, 1)) {
            switch (key) {
                case 'q':
                    quit = 1;
                    break;
                case ' ':
                    paused = !paused;
                    break;
                case ',':
                    seek_bck(5*R);
                    break;
                case '.':
                    seek_fwd(5*R);
                    break;
                case '<':
                    seek_bck(30*R);
                    break;
                case '>':
                    seek_fwd(30*R);
                    break;
            }
        }
    }
    restore_terminal(&term_prev);
    return 0;
}
