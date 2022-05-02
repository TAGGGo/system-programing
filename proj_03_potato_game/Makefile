TARGETS=player ringmaster

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

player: player.c
	g++ -std=c++17 -g -o $@ $<

ringmaster: ringmaster.c
	g++ -std=c++17 -g -o $@ $<