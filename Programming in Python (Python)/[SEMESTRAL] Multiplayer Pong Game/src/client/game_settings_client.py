class GameSetting:
    """
    A class to manage game_settings for the game
    """
    def __init__(self):
        """
        Initializes the game_settings with None,
        it will be updated from the server
        """
        self.settings = {
            'PADDLE_WIDTH': None,
            'PADDLE_HEIGHT': None,
            'PADDLE1_X': None,
            'PADDLE2_X': None,
            'PADDLE1_COLOR': None,
            'PADDLE2_COLOR': None,
            'PADDLE1_NAME': None,
            'PADDLE2_NAME': None,
            'BALL_SIZE': None,
            'BALL_COLOR': None,
            'MAX_CHAT_LINES': None
        }