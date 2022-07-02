
This is a pretty naive minifier for vanilla Javascript.

What I mean by naive, is that it assumes your Javascript input has already been validated, and that it (currently) doesn't even do a lot of the fancier tricks minifiers do these days. That being said, it's a single, small file, runs very quickly (see below), and you should still expect a file size reduction between 20 and 40 percent.

Minifying a fairly small file with almost no comments still gets you decent savings:
```
Minified: test.js
initial size (bytes): 14853, minified: 10768, a 27.50% reduction.
	fread time:  0.000098 seconds
	minify time: 0.000125 seconds
	fwrite time: 0.000305 seconds
	total time:  0.000528 seconds
```

More test cases, better speed, and better size reduction to come.

