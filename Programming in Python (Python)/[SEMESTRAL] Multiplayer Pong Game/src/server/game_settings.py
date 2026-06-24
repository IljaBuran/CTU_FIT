import json
import os
import sys


class GameSetting:
    """
    A class to manage game settings for the app,
    save/loads game into json file
    """
    def __init__(self, config_file='server_config.json'):
        if getattr(sys, 'frozen', False):
            # If the application is frozen
            base_dir = os.path.dirname(sys.executable)
        else:
            # If running as a script
            base_dir = os.path.dirname(os.path.abspath(__file__))

        self.config_path = os.path.join(base_dir, config_file)
        self.settings = {
            'SCREEN_WIDTH': 800,
            'SCREEN_HEIGHT': 600,
            'PADDLE_WIDTH': 10,
            'PADDLE_HEIGHT': 60,
            'PADDLE1_X': 0,
            'PADDLE1_COLOR': '#FFFFFF',
            'PADDLE2_COLOR': '#FFFFFF',
            'PADDLE1_NAME': 'Player1',
            'PADDLE2_NAME': 'Player2',
            'BALL_COLOR': '#FFFFFF',
            'PADDLE_MOVEMENT_DISTANCE': 5,
            'BALL_SIZE': 10,
            'BALL_VEL': 6,
            'MAX_CHAT_LINES': 5
        }
        self.settings['PADDLE2_X'] = self.settings['SCREEN_WIDTH'] - self.settings['PADDLE1_X'] - self.settings[
            'PADDLE_WIDTH']

        # Load settings from file or save default settings
        if os.path.exists(self.config_path):
            self.load_from_file(self.config_path)
        else:
            self.save_to_file(self.config_path)

    def load_from_file(self, file_path):
        """
        Load game settings from a JSON configuration file.
        """
        try:
            with open(file_path, 'r') as file:
                data = json.load(file)
                # Update settings with loaded data
                for key in self.settings:
                    if key in data:
                        self.settings[key] = data[key]
            print(f"Settings loaded from '{file_path}'.")
        except FileNotFoundError:
            print(f"Configuration file '{file_path}' not found. Creating with default settings.")
            self.save_to_file(file_path)

    def save_to_file(self, file_path):
        """
        Saves game settings to a JSON configuration file.
        """
        try:
            with open(file_path, 'w') as file:
                json.dump(self.settings, file, indent=4)
            print(f"Settings saved to '{file_path}'.")
        except Exception as e:
            print(f"Failed to save settings to '{file_path}': {e}")

    def display_settings(self):
        """
        [DEBUG] displays game settings.
        """
        print("Current Game Settings:")
        for key, value in self.settings.items():
            print(f"  {key}: {value}")
