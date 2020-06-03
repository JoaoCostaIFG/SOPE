# Simpledu T4G06

This is a simpler version of the **gnu's du**. This project was
developed for a UNIX OS development university class.  
Check this repo's collaborators section to see my project partners.

## Collaborators

This project was developed by _João Costa_ (me), [João Lucas](https://github.com/joaolucasmartins)
and [Ricardo Fontão](https://github.com/ricardofontao2000) for our Operating
Systems class (SOPE) at [MIEIC, FEUP](https://sigarra.up.pt/feup/en/cur_geral.cur_view?pv_curso_id=742)
(2019/2020).  

## Compilation

Use `make` to compile the program.  
Use `make clean` to delete old compilation files + binary.  
To run the program: `simpledu -l [path] [-a] [-b] [-B size] [-L] [-S]
[--max-depth=N]`. You can also specify multiple options using a single
argument: `simpledu -lLabS`.

## Tests

Run the `ttest.sh` script like so:  
`./ttests.sh [optional_path [-L]]`  
You can pass an optional path for it to run on (instead of only the defaults).
You can also specify the -L options which runs the tests that involve dereferencing.

## License

The code in this repository is published under a GNU General
Public License: **gpl-3.0**.

