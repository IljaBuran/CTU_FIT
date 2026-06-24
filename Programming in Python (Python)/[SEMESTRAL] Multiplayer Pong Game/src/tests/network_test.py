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

from src.client.network import Network
from src.client.gamestate import GameState
from src.client.game_settings_client import GameSetting


@pytest.fixture
def mock_socket():
    # Corrected the patch path to include 'src'
    with patch('src.client.network.socket.socket') as mock_socket_class:
        mock_socket_instance = mock_socket_class.return_value
        yield mock_socket_instance


def test_network_initialization(mock_socket):
    network = Network()
    assert network.sock == mock_socket
    assert isinstance(network.game_state, GameState)
    assert isinstance(network.game_setting, GameSetting)
    assert network.chat == []
    assert network.running == True
    assert network.role == ""


def test_network_send_command(mock_socket):
    network = Network()
    network.send_command("TEST_COMMAND")
    mock_socket.sendall.assert_called_with(b"TEST_COMMAND\n")


def test_network_handle_command_color(mock_socket):
    network = Network()
    new_state = network.handle_command("/COLOR", "Play")
    assert new_state == 'Color'


def test_network_handle_command_pause(mock_socket):
    network = Network()
    network.handle_command("/PAUSE", "Play")
    mock_socket.sendall.assert_called_with(b"/PAUSE\n")


def test_network_handle_command_invalid(mock_socket):
    network = Network()
    initial_state = "Play"
    new_state = network.handle_command("/INVALID_CMD", initial_state)
    assert new_state == initial_state
    assert "INVALID COMMAND!" in network.chat


def test_network_add_chat_message():
    network = Network()
    network.game_setting.settings['MAX_CHAT_LINES'] = 3
    network.add_chat_message("Hello")
    network.add_chat_message("World")
    network.add_chat_message("Test")
    assert network.chat == ["Hello", "World", "Test"]

    # Adding a fourth message should remove the first
    network.add_chat_message("Overflow")
    assert network.chat == ["World", "Test", "Overflow"]


def test_network_update_game_state():
    network = Network()
    data = ["100", "150", "200", "250", "1", "2"]
    network.update_game_state(data)
    assert network.game_state.ball_x == 100
    assert network.game_state.ball_y == 150
    assert network.game_state.paddle1_y == 200
    assert network.game_state.paddle2_y == 250
    assert network.game_state.score1 == 1
    assert network.game_state.score2 == 2


def test_network_update_game_setting():
    network = Network()
    data = [
        "10", "60", "0", "790",
        "#FFFFFF", "#FFFFFF",
        "Alice", "Bob",
        "10", "#FFFFFF", "5"
    ]
    network.update_game_setting(data)
    assert network.game_setting.settings['PADDLE_WIDTH'] == 10
    assert network.game_setting.settings['PADDLE_HEIGHT'] == 60
    assert network.game_setting.settings['PADDLE1_X'] == 0
    assert network.game_setting.settings['PADDLE2_X'] == 790
    assert network.game_setting.settings['PADDLE1_COLOR'] == "#FFFFFF"
    assert network.game_setting.settings['PADDLE2_COLOR'] == "#FFFFFF"
    assert network.game_setting.settings['PADDLE1_NAME'] == "Alice"
    assert network.game_setting.settings['PADDLE2_NAME'] == "Bob"
    assert network.game_setting.settings['BALL_SIZE'] == 10
    assert network.game_setting.settings['BALL_COLOR'] == "#FFFFFF"
    assert network.game_setting.settings['MAX_CHAT_LINES'] == 5


def test_network_process_message_role(mock_socket):
    network = Network()
    network.process_message("ROLE:PADDLE1")
    assert network.role == "PADDLE1"


def test_network_process_message_role_denied(mock_socket):
    with patch('sys.exit') as mock_exit:
        network = Network()
        network.process_message("ROLE:DENIED")
        mock_socket.close.assert_called_once()
        mock_exit.assert_called_once()


def test_network_process_message_chat(mock_socket):
    network = Network()
    network.game_setting.settings['MAX_CHAT_LINES'] = 5  # Set to a valid integer
    network.process_message("CHAT:Hello World")
    assert network.chat == ["Hello World"]


def test_network_process_message_bonus_create(mock_socket):
    network = Network()
    network.process_message("BONUS:100|200|50")
    assert network.game_state.bonus_box_x == 100
    assert network.game_state.bonus_box_y == 200
    assert network.game_state.bonus_box_diameter == 50


def test_network_process_message_bonus_done(mock_socket):
    network = Network()
    network.game_state.bonus_box_x = 100
    network.game_state.bonus_box_y = 200
    network.game_state.bonus_box_diameter = 50
    network.process_message("BONUS:DONE")
    assert network.game_state.bonus_box_x == 0
    assert network.game_state.bonus_box_y == 0
    assert network.game_state.bonus_box_diameter == 0


def test_network_receive_game_state(mock_socket):
    network = Network()
    network.running = True

    mock_socket.recv.side_effect = [
        b"100|150|200|250|1|2\n",
        b"",  # Simulate server disconnect
    ]

    with patch('builtins.print') as mock_print:
        network.receive_game_state()

    assert network.game_state.ball_x == 100
    assert network.game_state.ball_y == 150
    assert network.game_state.paddle1_y == 200
    assert network.game_state.paddle2_y == 250
    assert network.game_state.score1 == 1
    assert network.game_state.score2 == 2
    assert network.running == False
    mock_socket.close.assert_called_once()
