class GameState:
    """
    A class to represent the game state of the game.
    It's updated from server every frame.
    """
    def __init__(self):
        self.ball_x = 0
        self.ball_y = 0

        self.paddle1_y = 0
        self.paddle2_y = 0

        self.score1 = 0
        self.score2 = 0

        self.bonus_box_x = None
        self.bonus_box_y = None
        self.bonus_box_diameter = None
