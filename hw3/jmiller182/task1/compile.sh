if [ "$#" -lt 2 ]; then
  echo "Error: missing arg"
  exit 1
fi 

gcc task1.c -o task1 && ./task1 $1