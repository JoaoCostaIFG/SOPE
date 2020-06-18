build:
	@gcc a.c -o processo1 -l pthread -l rt
	@gcc a2.c -o processo2 -l pthread -l rt
	
run: build
	@./processo1 &
	@./processo2

.PHONY: build run
