readonly SOCKET_PATH=127.0.0.1
readonly PORT=2715
echo "Hello World" | nc $SOCKET_PATH $PORT
