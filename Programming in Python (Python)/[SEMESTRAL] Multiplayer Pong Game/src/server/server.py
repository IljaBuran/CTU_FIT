import random
import socket
import threading
import time

from src.server.gamestate import GameState
from src.server.game_settings import GameSetting

class Server:
    """
    A class to handle the server-side operations of the Pong game.
    Handles client connections, game state updates, and communication between clients.
    """
    def __init__(self):
        """
        Initialize the Server instance with game settings, game state, and networking parameters.
        """
        self.game_setting = GameSetting('server_config.json')
        self.game_state = GameState(
            screen_h=self.game_setting.settings['SCREEN_HEIGHT'],
            paddle_h=self.game_setting.settings['PADDLE_HEIGHT'],
            ball_vel=self.game_setting.settings['BALL_VEL']
        )
        self.HOST = ""
        self.PORT = 12345
        self.FPS = 60
        self.connections = []
        self.available_roles = ["PADDLE1", "PADDLE2"]
        self.player_names = {
            "PADDLE1": "Player1",
            "PADDLE2": "Player2"
        }
        self.lock = threading.Lock()
        self.server = None
        self.pause = True
        self.bonus_p1 = []
        self.bonus_p2 = []

    def initialize(self):
        """
        Initialize and start the server to listen for incoming client connections,
        starts the game state update thread.
        """
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.bind((self.HOST, self.PORT))
        self.server.listen(5)
        print(f"[LISTENING] Server on port {self.PORT}")

        threading.Thread(target=self.update_game_state, daemon=True).start()

        while True:
            conn, addr = self.server.accept()
            threading.Thread(target=self.handle_connection, args=(conn, addr), daemon=True).start()

    def handle_connection(self, conn, addr):
        """
        Handle an individual client connection, assigns roles, sends game settings,
        processes incoming messages.
        """

        print(f"[NEW CONNECTION] {addr}")

        assigned_role = None
        with self.lock:
            if not self.available_roles:
                print("[LISTENING] No available roles. Closing connection.")
                conn.sendall("ROLE:DENIED\n".encode())
                conn.close()
                return
            else:
                assigned_role = self.available_roles.pop(0)
                msg = f"ROLE:{assigned_role}\n"
                conn.sendall(msg.encode())
                print(f"[INFO] {addr} assigned role {assigned_role}")
                self.connections.append(conn)

        self.broadcast_game_setting(conn)

        buffer = ""
        try:
            while True:
                data = conn.recv(4096).decode()
                if not data:
                    print(f"[DISCONNECTED] {addr} closed connection.")
                    break

                buffer += data
                while '\n' in buffer:
                    msg, buffer = buffer.split('\n', 1)
                    self.process_client_message(msg, conn, assigned_role)

        except ConnectionResetError:
            print(f"[DISCONNECTED] {addr} closed connection.")
        finally:
            conn.close()
            with self.lock:
                if assigned_role:
                    self.available_roles.append(assigned_role)
                    print(f"[INFO] Freed role {assigned_role}. Available: {self.available_roles}")
                if conn in self.connections:
                    self.connections.remove(conn)

    def process_client_message(self, message, conn, assigned_role):
        """
        Process a single message received from a client.
        """
        print(f"MESSAGE RECEIVED:{message}")
        with self.lock:
            if message.startswith("PADDLE1:"):
                direction = message.split(":", 1)[1]
                if direction == 'UP':
                    self.game_state.paddle1_y -= self.game_setting.settings['PADDLE_MOVEMENT_DISTANCE']
                elif direction == 'DOWN':
                    self.game_state.paddle1_y += self.game_setting.settings['PADDLE_MOVEMENT_DISTANCE']
                self.game_state.paddle1_y = max(0, min(self.game_state.paddle1_y, self.game_setting.settings['SCREEN_HEIGHT'] - self.game_setting.settings['PADDLE_HEIGHT']))

            elif message.startswith("PADDLE2:"):
                direction = message.split(":", 1)[1]
                if direction == 'UP':
                    self.game_state.paddle2_y -= self.game_setting.settings['PADDLE_MOVEMENT_DISTANCE']
                elif direction == 'DOWN':
                    self.game_state.paddle2_y += self.game_setting.settings['PADDLE_MOVEMENT_DISTANCE']
                self.game_state.paddle2_y = max(0, min(self.game_state.paddle2_y, self.game_setting.settings['SCREEN_HEIGHT'] - self.game_setting.settings['PADDLE_HEIGHT']))

            elif message.startswith("CHAT:"):
                text = message.split(":", 1)[1]
                self.broadcast_chat(assigned_role, text)

            elif message.startswith("/"):
                command = message[1:].strip()
                if command.startswith("COLOR:"):
                    color = command.split(":", 1)[1]
                    self.change_player_color(assigned_role, color)
                elif command.startswith("PAUSE"):
                    self.pause = True
                elif command.startswith("CONTINUE"):
                    self.pause = False
                elif command.startswith("NAME:"):
                    name = command.split(":", 1)[1]
                    self.game_setting.settings[assigned_role + '_NAME'] = name
                    self.broadcast_game_setting()
                elif command.startswith("BONUS:DONE"):
                    if assigned_role == "PADDLE1":
                        self.bonus_p1.append('ball_speed')
                    if assigned_role == 'PADDLE2':
                        self.bonus_p2.append('ball_speed')
                    self.delete_bonus_box()
                else:
                    print(f"[ERROR] Unknown command: {command}")
            else:
                print(f"[WARNING] Unrecognized message from {conn.getpeername()}: {message}")

    def change_player_color(self, role, color):
        """
        Change the color of a player's paddle.
        """
        #print(f"COLOR:CHANGE->{role, color}")
        if role == "PADDLE1":
            self.game_setting.settings['PADDLE1_COLOR'] = color
            print(f"[INFO] {role} color changed to {self.game_setting.settings['PADDLE1_COLOR']}")
        elif role == "PADDLE2":
            self.game_setting.settings['PADDLE2_COLOR'] = color
            print(f"[INFO] {role} color changed to {self.game_setting.settings['PADDLE2_COLOR']}")
        self.broadcast_game_setting()


    def broadcast_game_state(self):
        """
        Broadcast the current game state to all connected clients.
        """
        with self.lock:
            state_str = (
                f"{self.game_state.ball_x}|{self.game_state.ball_y}|"
                f"{self.game_state.paddle1_y}|{self.game_state.paddle2_y}|"
                f"{self.game_state.score1}|{self.game_state.score2}\n"
            ).encode()

        with self.lock:
            current_connections = self.connections.copy()

        for conn in current_connections:
            try:
                conn.sendall(state_str)
            except Exception as e:
                print(f"[ERROR] Failed to send game state to a client: {e}")
                with self.lock:
                    if conn in self.connections:
                        self.connections.remove(conn)
                conn.close()

    def broadcast_game_setting(self, conn=None):
        """
        Broadcast the current game settings to all connected clients or a specific client.
        """
        state_str = (
            f"SETTING:"
            f"{self.game_setting.settings['PADDLE_WIDTH']}|{self.game_setting.settings['PADDLE_HEIGHT']}|"
            f"{self.game_setting.settings['PADDLE1_X']}|{self.game_setting.settings['PADDLE2_X']}|"
            f"{self.game_setting.settings['PADDLE1_COLOR']}|{self.game_setting.settings['PADDLE2_COLOR']}|"
            f"{self.game_setting.settings['PADDLE1_NAME']}|{self.game_setting.settings['PADDLE2_NAME']}|"
            f"{self.game_setting.settings['BALL_SIZE']}|{self.game_setting.settings['BALL_COLOR']}|"
            f"{self.game_setting.settings['MAX_CHAT_LINES']}\n"
        ).encode()

        for conn in self.connections:
            try:
                conn.sendall(state_str)
            except:
                if conn in self.connections:
                    conn.close()
                    self.connections.remove(conn)

    def broadcast_chat(self, role, message):
        """
        Broadcast a chat message from a player to all connected clients.
        """
        full_message = ""

        if role == "PADDLE1":
            full_message = f"CHAT:{self.game_setting.settings['PADDLE1_NAME']}: {message}\n".encode()
        elif role == "PADDLE2":
            full_message = f"CHAT:{self.game_setting.settings['PADDLE2_NAME']}: {message}\n".encode()

        for conn in self.connections:
            try:
                conn.sendall(full_message)
            except:
                if conn in self.connections:
                    conn.close()
                    self.connections.remove(conn)

    def make_bonus_box(self):
        """
        Create and broadcast a bonus box at a random position on the screen.
        """
        box_dia = 50
        box_x = random.randint(self.game_setting.settings['PADDLE_WIDTH'] + 20, self.game_setting.settings['SCREEN_WIDTH'] - self.game_setting.settings['PADDLE_WIDTH'] - 20 - box_dia)
        box_y = random.randint(10, self.game_setting.settings['SCREEN_HEIGHT'] - 10 - box_dia)
        box_str = (
            f"BONUS:"
            f"{box_x}|{box_y}|"
            f"{box_dia}\n"
        ).encode()

        for conn in self.connections:
            try:
                conn.sendall(box_str)
            except:
                if conn in self.connections:
                    conn.close()
                    self.connections.remove(conn)

    def delete_bonus_box(self):
        """
        Inform all clients to delete the bonus box from their screens.
        """
        for conn in self.connections:
            try:
                conn.sendall('BONUS:DONE\n'.encode())
            except:
                if conn in self.connections:
                    conn.close()
                    self.connections.remove(conn)

    def reset_ball_velocity(self, intype=None):
        """
        Reset the ball's velocity.
        """
        vel = self.game_setting.settings['BALL_VEL']
        if intype is None:
            ran = random.randint(0, 1)
            if ran == 0:
                self.game_state.ball_vel_y = vel
            else:
                self.game_state.ball_vel_y = -vel

        if intype == 'start':
            ran = random.randint(0, 3)
            vel = int(vel * 0.7)
            if ran == 0:
                self.game_state.ball_vel_x = vel
                self.game_state.ball_vel_y = vel
            if ran == 1:
                self.game_state.ball_vel_x = -vel
                self.game_state.ball_vel_y = vel
            if ran == 2:
                self.game_state.ball_vel_x = vel
                self.game_state.ball_vel_y = -vel
            if ran == 3:
                self.game_state.ball_vel_x = -vel
                self.game_state.ball_vel_y = -vel

    def update_game_state(self):
        """
        Continuously update the game state, handle collisions, and broadcast updates to clients,
        rtuns in a separate thread.
        """
        count = 0
        limit = random.randint(2, 6)
        while True:
            time.sleep(1/self.FPS)
            with self.lock:
                if not self.pause:

                    if count >= limit:
                        self.make_bonus_box()
                        count = 0
                        limit = random.randint(2, 6)

                    # Move ball
                    self.game_state.ball_x += self.game_state.ball_vel_x
                    self.game_state.ball_y += self.game_state.ball_vel_y

                    # Check collisions

                    # Top/Bottom
                    if self.game_state.ball_y <= 0 or self.game_state.ball_y + self.game_setting.settings['BALL_SIZE'] > self.game_setting.settings['SCREEN_HEIGHT']:
                        self.game_state.ball_vel_y *= -1
                    # Left Paddle
                    # if collision_disable != 1:
                    #     if (self.game_state.ball_x <= self.game_setting.settings['PADDLE1_X'] + self.game_setting.settings['PADDLE_WIDTH'] and
                    #         self.game_state.paddle1_y < self.game_state.ball_y + self.game_setting.settings['BALL_SIZE'] and
                    #         self.game_state.paddle1_y + self.game_setting.settings['PADDLE_HEIGHT'] > self.game_state.ball_y):
                    #         self.game_state.ball_vel_x *= -1
                    #         collision_disable = 1
                    #         count += 1
                    #         self.reset_ball_velocity()
                    #         for b in self.bonus_p1:
                    #             if b == 'ball_speed':
                    #                 self.game_state.ball_vel_x = int(self.game_state.ball_vel_x * 1.5)
                    #                 self.game_state.ball_vel_y = int(self.game_state.ball_vel_y * 1.5)
                    #                 self.bonus_p1.remove(b)
                    #                 break
                    #
                    # # Right Paddle
                    # if collision_disable != 2:
                    #     if (self.game_state.ball_x + self.game_setting.settings['BALL_SIZE'] >= self.game_setting.settings['PADDLE2_X'] and
                    #         self.game_state.paddle2_y < self.game_state.ball_y + self.game_setting.settings['BALL_SIZE'] and
                    #         self.game_state.paddle2_y + self.game_setting.settings['PADDLE_HEIGHT'] > self.game_state.ball_y):
                    #         self.game_state.ball_vel_x *= -1
                    #         collision_disable = 2
                    #         count += 1
                    #         self.reset_ball_velocity()
                    #         for b in self.bonus_p2:
                    #             if b == 'ball_speed':
                    #                 self.game_state.ball_vel_x = int(self.game_state.ball_vel_x * 1.5)
                    #                 self.game_state.ball_vel_y = int(self.game_state.ball_vel_y * 1.5)
                    #                 self.bonus_p2.remove(b)
                    #                 break

                    # Left Paddle
                    if self.game_state.ball_vel_x < 0:
                        if self.game_state.ball_y >= self.game_state.paddle1_y and self.game_state.ball_y <= self.game_state.paddle1_y + self.game_setting.settings['PADDLE_HEIGHT']:
                            if self.game_state.ball_x - self.game_setting.settings['BALL_SIZE'] <= self.game_setting.settings['PADDLE1_X'] + self.game_setting.settings['PADDLE_WIDTH']:
                                self.game_state.ball_vel_x *= -1
                                count += 1
                                self.reset_ball_velocity()
                                for b in self.bonus_p1:
                                    if b == 'ball_speed':
                                        self.game_state.ball_vel_x = int(self.game_state.ball_vel_x * 1.5)
                                        self.game_state.ball_vel_y = int(self.game_state.ball_vel_y * 1.5)
                                        self.bonus_p1.remove(b)
                                        break

                    # Right Paddle
                    else:
                        if self.game_state.ball_y >= self.game_state.paddle2_y and self.game_state.ball_y <= self.game_state.paddle2_y + self.game_setting.settings['PADDLE_HEIGHT']:
                            if self.game_state.ball_x + self.game_setting.settings['BALL_SIZE'] >= self.game_setting.settings['PADDLE2_X']:
                                self.game_state.ball_vel_x *= -1
                                count += 1
                                self.reset_ball_velocity()
                                for b in self.bonus_p2:
                                    if b == 'ball_speed':
                                        self.game_state.ball_vel_x = int(self.game_state.ball_vel_x * 1.5)
                                        self.game_state.ball_vel_y = int(self.game_state.ball_vel_y * 1.5)
                                        self.bonus_p2.remove(b)
                                        break

                    # Collision left (Paddle2 score++)
                    if self.game_state.ball_x + self.game_setting.settings['BALL_SIZE'] / 2 < 0:
                        self.game_state.score2 += 1
                        self.game_state.ball_x = self.game_setting.settings['SCREEN_WIDTH'] // 2 - self.game_setting.settings['BALL_SIZE'] // 2
                        self.game_state.ball_y = self.game_setting.settings['SCREEN_HEIGHT'] // 2 - self.game_setting.settings['BALL_SIZE'] // 2
                        self.reset_ball_velocity('start')

                    # Collision right (Paddle1 score++)
                    if self.game_state.ball_x - self.game_setting.settings['BALL_SIZE'] / 2 > self.game_setting.settings['SCREEN_WIDTH']:
                        self.game_state.score1 += 1
                        self.game_state.ball_x = self.game_setting.settings['SCREEN_WIDTH'] // 2 - self.game_setting.settings['BALL_SIZE'] // 2
                        self.game_state.ball_y = self.game_setting.settings['SCREEN_HEIGHT'] // 2 - self.game_setting.settings['BALL_SIZE'] // 2
                        self.reset_ball_velocity('start')

            self.broadcast_game_state()