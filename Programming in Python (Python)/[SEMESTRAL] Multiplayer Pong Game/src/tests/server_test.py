import os
import sys
import pytest
import socket
from unittest import mock
from unittest.mock import MagicMock, patch

# Ensure that 'src' is in PYTHONPATH
# Adjust the path based on your project structure
# Here, we assume 'src' is one level up from 'tests'
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from src.server.server import Server
from src.server.gamestate import GameState
from src.server.game_settings import GameSetting


@pytest.fixture
def mock_socket():
    # Corrected the patch path to exclude 'src'
    with patch('server.server.socket.socket') as mock_socket_class:
        mock_socket_instance = mock_socket_class.return_value
        yield mock_socket_instance



def test_server_handle_new_connection(mock_socket):
    server = Server()
    mock_conn = mock.Mock()
    mock_addr = ('127.0.0.1', 5555)

    mock_conn.recv.return_value = b''

    with patch.object(server, 'broadcast_game_setting') as mock_broadcast:
        server.handle_connection(mock_conn, mock_addr)
        mock_conn.sendall.assert_called_with(b"ROLE:PADDLE1\n")
        assert "PADDLE1" in server.available_roles
        assert mock_conn not in server.connections
        mock_broadcast.assert_called_once_with(mock_conn)

def test_server_process_client_message_paddle_move(mock_socket):
    server = Server()
    server.game_setting.settings = {
        'PADDLE_MOVEMENT_DISTANCE': 5,
        'SCREEN_HEIGHT': 600,
        'PADDLE_HEIGHT': 60
    }
    server.game_state.paddle1_y = 100
    server.game_state.paddle2_y = 100

    server.process_client_message("PADDLE1:UP", mock_socket, "PADDLE1")
    assert server.game_state.paddle1_y == 95

    server.process_client_message("PADDLE1:DOWN", mock_socket, "PADDLE1")
    assert server.game_state.paddle1_y == 100

    server.process_client_message("PADDLE2:UP", mock_socket, "PADDLE2")
    assert server.game_state.paddle2_y == 95

    server.process_client_message("PADDLE2:DOWN", mock_socket, "PADDLE2")
    assert server.game_state.paddle2_y == 100


def test_server_process_client_message_chat(mock_socket):
    server = Server()
    server.game_setting.settings = {
        'PADDLE1_NAME': 'Alice',
        'PADDLE2_NAME': 'Bob'
    }

    with patch.object(server, 'broadcast_chat') as mock_broadcast_chat:
        server.process_client_message("CHAT:Hello World", mock_socket, "PADDLE1")
        mock_broadcast_chat.assert_called_with("PADDLE1", "Hello World")


def test_server_process_client_message_color_change(mock_socket):
    server = Server()
    server.game_setting.settings = {
        'PADDLE1_COLOR': '#FFFFFF',
        'PADDLE2_COLOR': '#FFFFFF'
    }

    with patch.object(server, 'change_player_color') as mock_change_color:
        server.process_client_message("/COLOR:#FF00FF", mock_socket, "PADDLE1")
        mock_change_color.assert_called_with("PADDLE1", "#FF00FF")


def test_server_process_client_message_invalid_command(mock_socket):
    server = Server()
    with patch('builtins.print') as mock_print:
        server.process_client_message("/INVALID_CMD", mock_socket, "PADDLE1")
        mock_print.assert_any_call("[ERROR] Unknown command: INVALID_CMD")


def test_server_broadcast_chat_paddle1(mock_socket):
    server = Server()
    server.game_setting.settings = {
        'PADDLE1_NAME': 'Alice',
        'PADDLE2_NAME': 'Bob'
    }
    server.connections = [mock_socket]

    with patch.object(mock_socket, 'sendall') as mock_sendall:
        server.broadcast_chat("PADDLE1", "Hello")
        mock_sendall.assert_called_with(b"CHAT:Alice: Hello\n")


def test_server_make_bonus_box(mock_socket):
    server = Server()
    server.game_setting.settings = {
        'PADDLE_WIDTH': 10,
        'SCREEN_WIDTH': 800,
        'SCREEN_HEIGHT': 600
    }
    server.connections = [mock_socket]

    with patch('random.randint', side_effect=[100, 200, 50]):
        server.make_bonus_box()
        mock_socket.sendall.assert_called_with(b"BONUS:100|200|50\n")


def test_server_delete_bonus_box(mock_socket):
    server = Server()
    server.connections = [mock_socket]

    with patch.object(mock_socket, 'sendall') as mock_sendall:
        server.delete_bonus_box()
        mock_sendall.assert_called_with(b"BONUS:DONE\n")
