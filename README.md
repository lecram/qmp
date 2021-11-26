# qmp

quick music player

A terminal player for [qms](https://github.com/lecram/qms).

## Building

Assuming you have qms sources alongside the qmp folder:

```
$ cd qmp
$ cc -o qmp -I../qms/ ../qms/qms.c ../qms/smf.c qmp.c
```

## Running

Currently qms sends samples to stdout. Pipe to
[aplay](https://github.com/alsa-project/alsa-utils) to get sound.

```
$ ./qmp song.mid | aplay -q -t raw -f cd
```

## Controls

```
  q       quit
  SPACE   pause/resume
  z/x/c   tempo down/reset/up
  ,/.     seek back/forward 5 seconds
  </>     seek back/forward 30 seconds
  0..9    seek to N*10% time of song
  m       set mark
  j       seek to mark
```
