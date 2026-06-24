import pytest
import os
import json
from src.server.game_settings import GameSetting


@pytest.fixture
def temp_config_file(tmp_path):
    config_file = tmp_path / "test_server_config.json"
    yield config_file
    if config_file.exists():
        os.remove(config_file)


def test_game_setting_default_initialization(temp_config_file):
    gs = GameSetting(config_file=str(temp_config_file))
    assert gs.settings['SCREEN_WIDTH'] == 800
    assert gs.settings['SCREEN_HEIGHT'] == 600
    assert gs.settings['PADDLE_WIDTH'] == 10
    assert gs.settings['PADDLE_HEIGHT'] == 60
    assert gs.settings['PADDLE1_X'] == 0
    assert gs.settings['PADDLE2_X'] == 800 - 0 - 10
    assert gs.settings['PADDLE1_COLOR'] == '#FFFFFF'
    assert gs.settings['PADDLE2_COLOR'] == '#FFFFFF'
    assert gs.settings['PADDLE1_NAME'] == 'Player1'
    assert gs.settings['PADDLE2_NAME'] == 'Player2'
    assert gs.settings['BALL_COLOR'] == '#FFFFFF'
    assert gs.settings['PADDLE_MOVEMENT_DISTANCE'] == 5
    assert gs.settings['BALL_SIZE'] == 10
    assert gs.settings['BALL_VEL'] == 6
    assert gs.settings['MAX_CHAT_LINES'] == 5


def test_game_setting_load_from_file(temp_config_file):
    # Create a config file with custom settings
    custom_settings = {
        'SCREEN_WIDTH': 1024,
        'SCREEN_HEIGHT': 768,
        'PADDLE_WIDTH': 15,
        'PADDLE_HEIGHT': 75,
        'PADDLE1_X': 20,
        'PADDLE2_X': 989,
        'PADDLE1_COLOR': '#FF0000',
        'PADDLE2_COLOR': '#00FF00',
        'PADDLE1_NAME': 'Alice',
        'PADDLE2_NAME': 'Bob',
        'BALL_COLOR': '#0000FF',
        'PADDLE_MOVEMENT_DISTANCE': 10,
        'BALL_SIZE': 12,
        'BALL_VEL': 8,
        'MAX_CHAT_LINES': 10
    }
    with open(temp_config_file, 'w') as f:
        json.dump(custom_settings, f)

    gs = GameSetting(config_file=str(temp_config_file))
    for key, value in custom_settings.items():
        assert gs.settings[key] == value


def test_game_setting_save_to_file(temp_config_file):
    gs = GameSetting(config_file=str(temp_config_file))
    gs.settings['SCREEN_WIDTH'] = 1280
    gs.save_to_file(str(temp_config_file))

    with open(temp_config_file, 'r') as f:
        data = json.load(f)

    assert data['SCREEN_WIDTH'] == 1280
