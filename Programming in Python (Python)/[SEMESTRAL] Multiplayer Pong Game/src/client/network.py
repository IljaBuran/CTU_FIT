import socket
import sys
from typing import List, Tuple

from src.client.gamestate import GameState
from src.client.game_settings_client import GameSetting


class Network:
    """
    A class to handle network communication between the client and server,
    including connecting, sending commands, receiving game states, and managing chat messages.
    """

    def __init__(self) -> None:
        """
        Initialize with socket, game state, game settings,
        and chat message buffer.
        """
        self.sock: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(10)  # Set a timeout of 10 seconds
        self.role: str = ""
        self.game_state: GameState = GameState()
        self.game_setting: GameSetting = GameSetting()
        self.chat: List[str] = []
        self.running: bool = True  # To control the receive thread

    def disconnect(self) -> None:
        """
        Disconnect from the server by sending a disconnect command and closing the socket.
        """
        self.send_command('/DISCONNECT')
        self.sock.close()

    def connect(self, addr: Tuple[str, int]) -> None:
        """
        Connect to the server at the specified address.
        """
        try:
            self.sock.connect(addr)
            print("[Connected successfully]")
        except ConnectionRefusedError:
            print("[Connection Failed]. Is the server running?")
            sys.exit()
        except socket.timeout:
            print("[ERROR] Connection timed out. Please check the server IP and try again.")
            sys.exit()
        except socket.error as e:
            print(f"[ERROR] Socket error occurred: {e}")
            sys.exit()

    def send_command(self, command: str) -> None:
        """
        Send a command to the server.
        """
        try:
            message: str = f"{command}\n"
            self.sock.sendall(message.encode())
        except socket.error as e:
            print(f"[ERROR] Could not send data to server: {e}")

    def handle_command(self, command: str, state: str) -> str:
        """
        Handle a command entered by the user.
        """
        try:
            _, cmd = command.split("/", 1)
            cmd = cmd.lower()
        except ValueError:
            print("[ERROR] Invalid command format.")
            self.chat.append('INVALID COMMAND FORMAT!')
            return state

        if cmd == "color":
            return 'Color'
        elif cmd == "pause":
            self.send_command("/PAUSE")
            return 'Play'
        elif cmd == "continue":
            self.send_command("/CONTINUE")
            return 'Play'
        else:
            print('[ERROR] Invalid Command')
            self.chat.append('INVALID COMMAND!')
            return state

    def add_chat_message(self, message: str) -> None:
        """
        Add a chat message to the chat buffer, ensuring it doesn't exceed the maximum allowed lines.
        """
        self.chat.append(message)
        max_lines: int = self.game_setting.settings.get('MAX_CHAT_LINES', 5)
        if len(self.chat) > max_lines:
            self.chat.pop(0)

    def update_game_state(self, data: List[str]) -> None:
        """
        Update the game state based on the received data.
        """
        try:
            self.game_state.ball_x = int(data[0])
            self.game_state.ball_y = int(data[1])
            self.game_state.paddle1_y = int(data[2])
            self.game_state.paddle2_y = int(data[3])
            self.game_state.score1 = int(data[4])
            self.game_state.score2 = int(data[5])
        except (IndexError, ValueError) as e:
            print(f"[ERROR] Invalid game state data: {e}")

    def update_game_setting(self, data: List[str]) -> None:
        """
        Update the game settings based on the received data.
        """
        try:
            self.game_setting.settings['PADDLE_WIDTH'] = int(data[0])
            self.game_setting.settings['PADDLE_HEIGHT'] = int(data[1])
            self.game_setting.settings['PADDLE1_X'] = int(data[2])
            self.game_setting.settings['PADDLE2_X'] = int(data[3])
            self.game_setting.settings['PADDLE1_COLOR'] = str(data[4])
            self.game_setting.settings['PADDLE2_COLOR'] = str(data[5])
            self.game_setting.settings['PADDLE1_NAME'] = str(data[6])
            self.game_setting.settings['PADDLE2_NAME'] = str(data[7])
            self.game_setting.settings['BALL_SIZE'] = int(data[8])
            self.game_setting.settings['BALL_COLOR'] = str(data[9])
            self.game_setting.settings['MAX_CHAT_LINES'] = int(data[10])
            print("[Settings updated]")
        except (IndexError, ValueError) as e:
            print(f"[ERROR] Invalid game setting data: {e}")

    def update_bonus_box(self, data: List[str]) -> None:
        """
        Update the bonus box position and size based on the received data.
        """
        try:
            self.game_state.bonus_box_x = int(data[0])
            self.game_state.bonus_box_y = int(data[1])
            self.game_state.bonus_box_diameter = int(data[2])
        except (IndexError, ValueError) as e:
            print(f"[ERROR] Invalid bonus box data: {e}")

    def delete_bonus_box(self) -> None:
        """
        Reset the bonus box position and size to remove it from the game.
        """
        self.game_state.bonus_box_x = 0
        self.game_state.bonus_box_y = 0
        self.game_state.bonus_box_diameter = 0
        print("[Bonus box removed]")

    def receive_game_state(self) -> None:
        """
        Continuously receive game state updates from the server and process them.
        """
        buffer: str = ""
        while self.running:
            try:
                data: bytes = self.sock.recv(4096)
                if not data:
                    print("[SERVER DISCONNECTED]")
                    self.running = False
                    break

                buffer += data.decode()
                while '\n' in buffer:
                    msg, buffer = buffer.split('\n', 1)
                    self.process_message(msg.strip())

            except ConnectionResetError:
                print("[ERROR] Connection lost")
                self.running = False
                break
            except ValueError as ve:
                print(f"[ERROR] Value error: {ve}")
                self.running = False
                break
            except Exception as e:
                print(f"[ERROR] Unexpected error: {e}")
                self.running = False
                break
        self.sock.close()

    def process_message(self, msg: str) -> None:
        """
        Process a single message received from the server.
        """
        if msg.startswith("ROLE:"):
            self.handle_role(msg)
        elif msg.startswith("SETTING:"):
            self.handle_setting(msg)
        elif msg.startswith("CHAT:"):
            self.handle_chat(msg)
        elif msg.startswith("BONUS:"):
            self.handle_bonus(msg)
        else:
            parts: List[str] = msg.split("|")
            if len(parts) == 6:
                self.update_game_state(parts)
            else:
                print(f"[WARNING] Unrecognized message format: {msg}")

    def handle_role(self, msg: str) -> None:
        """
        Handle a role assignment message from the server.
        """
        try:
            _, role = msg.split(":", 1)
            role = role.strip()
        except ValueError:
            print("[ERROR] Invalid ROLE message format.")
            return

        if role == 'DENIED':
            print("[SERVER] The game is full.")
            self.sock.close()
            sys.exit()
        else:
            self.role = role
            print(f"SUCCESS: MY ROLE: {self.role}")

    def handle_setting(self, msg: str) -> None:
        """
        Handle a game setting update message from the server.
        """
        try:
            _, settings_str = msg.split(":", 1)
            settings = settings_str.split("|")
            if len(settings) == 11:
                self.update_game_setting(settings)
            else:
                print("[ERROR] Incorrect number of game settings received.")
        except ValueError:
            print("[ERROR] Invalid SETTING message format.")

    def handle_chat(self, msg: str) -> None:
        """
        Handle a chat message from the server.
        """
        try:
            _, message = msg.split(":", 1)
            message = message.strip()
            self.add_chat_message(message)
            print(f"[CHAT] {message}")
        except ValueError:
            print("[ERROR] Invalid CHAT message format.")

    def handle_bonus(self, msg: str) -> None:
        """
        Handle a bonus box update message from the server.
        """
        try:
            _, bonus_data = msg.split(":", 1)
            bonus_data = bonus_data.strip()
            if bonus_data.startswith("DONE"):
                self.delete_bonus_box()
                print('[INFO] Bonus box has been removed.')
            else:
                parts: List[str] = bonus_data.split("|")
                if len(parts) == 3:
                    self.update_bonus_box(parts)
                    print("[INFO] New bonus box created.")
                else:
                    print("[ERROR] Incorrect number of bonus box properties received.")
        except ValueError:
            print("[ERROR] Invalid BONUS message format.")
