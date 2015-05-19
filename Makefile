CFLAGS = -std=c99 -lc -D_GNU_SOURCE=1 -Wall -W -O3 $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs) -lSDL_image  -lSDL_ttf -lSDL_gfx

appchoo: appchoo.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

check: appchoo
	exec `echo '"duke.jpg" echo duke\n"börek.jpg" echo börek\n"cat.jpg" echo cat\n"mirror.png" echo mirror\n"tachikoma.jpg" echo tachikoma' | ./appchoo -t 10 -p 'Prompt!' -f 'FreeMonoBold.ttf' -d 'echo timeout'`

clean:
	rm -f appchoo

