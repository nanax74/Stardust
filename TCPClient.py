import socket

def main():
    server_ip = '192.168.68.115'  # サーバーのIPアドレス
    server_port = 7000  # サーバーのポート番号

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        print("Attempting to connect to server...")
        client_socket.connect((server_ip, server_port))
        print(f"Connected to {server_ip}:{server_port}")

        message = "Hello, TCP Server!"
        client_socket.sendall(message.encode())
        print(f"Sent: {message}")

        response = client_socket.recv(1024)
        print(f"Received: {response.decode()}")

    except Exception as e:
        print(f"Error: {e}")

    finally:
        print("Closing socket")
        client_socket.close()

if __name__ == "__main__":
    main()