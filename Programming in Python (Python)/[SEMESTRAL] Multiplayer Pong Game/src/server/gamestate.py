class GameState:
    def __init__(self, screen_h, paddle_h, ball_vel):
        """
        A class to represent the current state of the Pong game,
        is updated every frame from server
        """

        self.ball_x = 0
        self.ball_y = 0
        self.ball_vel_x = ball_vel
        self.ball_vel_y = ball_vel

        self.paddle1_y = screen_h // 2 - paddle_h // 2
        self.paddle2_y = screen_h // 2 - paddle_h // 2

        self.score1 = 0
        self.score2 = 0

        self.bonus_rec_x = None
        self.bonus_rec_y = None
        self.bonus_rec_diameter = None