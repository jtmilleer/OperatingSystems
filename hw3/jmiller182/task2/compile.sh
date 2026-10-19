if [ "$#" -lt 2 ]; then
  echo "Error: missing arg"
  exit 1
fi

gcc task2.c -o task2 && ./task2 $1