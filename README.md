
This is a pretty naive minifier for vanilla Javascript.

What I mean by naive, is that it assumes your Javascript input has already been validated, and that it (currently) doesn't even do a lot of the fancier tricks minifiers do these days. That being said, it's a single, small file, runs very quickly (see below), and you should still expect a file size reduction between 20 and 40 percent.

Minifying a fairly small file with almost no comments still gets you decent savings:
```
> ./miniminify js.js

Minified file: js.js
initial size (kb): 68.479, minified: 50.829, a 25.77% reduction.
	| raw time (milliseconds)        | time/kb
	| fread time:  0.1010            | 0.0015
	| minify time: 0.3210            | 0.0047
	| fwrite time: 0.3350            | 0.0049
	| total time:  0.7570            | 0.0111

```

More test cases for robustness, better speed, and better size reduction to come.

# Compiling / Building
Run `make`.

