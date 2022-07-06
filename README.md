
This is a pretty naive minifier for vanilla Javascript.

What I mean by naive, is that it assumes your Javascript input has already been validated, and that it (currently) doesn't even do a lot of the fancier tricks minifiers do these days. That being said, it's a single, small file, runs very quickly (see below), and you should still expect a file size reduction between 20 and 40 percent.

Minifying a fairly small file with almost no comments still gets you decent savings:
```
> ./miniminify ../test.js
Minified file: ../test.js
initial size (kb): 14.853, minified: 10.734, a 27.73% reduction.
	raw time            | time/kb
	fread time:  0.1030 | 0.0069
	minify time: 0.1190 | 0.0080
	fwrite time: 0.3380 | 0.0228
	total time:  0.5600 | 0.0377
```

More test cases for robustness, better speed, and better size reduction to come.

# Compiling / Building
Run `make`.

