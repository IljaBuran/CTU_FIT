from src.server.server import Server

def main():
    """
    Entry point for the server app.
    """
    print("Server starting...")
    server = Server()
    server.initialize()

if __name__ == "__main__":
    main()