for file in *.c; do
  echo "obj-y += ${file%.c}.o"

done

