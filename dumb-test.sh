# don't use this
trap "echo script failed!; exit 1" ERR

for i in {30..1024}
do
    gcc -o out "-DTABLE_SIZEZZ=$i" main.c && ./out
done

