#readonly SOCKET_PATH=127.0.0.1
readonly SOCKET_PATH=/tmp/service.sock
#readonly PORT=2715
#echo "Hello World" | nc $SOCKET_PATH $PORT
echo "Hello World" | nc -U $SOCKET_PATH