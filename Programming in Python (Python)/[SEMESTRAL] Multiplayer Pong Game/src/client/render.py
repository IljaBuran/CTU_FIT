import pygame

from src.client.gamestate import GameState
from src.client.game_settings_client import GameSetting
from src.client.button import Button
from src.client.slider import Slider


class Render:
    """
    A class to handle rendering of the game scene,
    including ui (buttons, sliders, etc.)
    """
    def __init__(self):
        """
        Init with default settings
        """
        self.screen = None
        self.clock = None
        self.font = None
        self.SCREEN_WIDTH = 800
        self.SCREEN_HEIGHT = 600
        self.current_user_state = None
        self.state_changed = False
        self.buttons = []
        self.sliders = []
        self.bonus = False

    def pygame_init(self):
        """
        Init pygame and set up the display window, clock and font
        """
        pygame.init()
        self.screen = pygame.display.set_mode((self.SCREEN_WIDTH, self.SCREEN_HEIGHT))
        pygame.display.set_caption("Pong")
        self.clock = pygame.time.Clock()
        self.font = pygame.font.SysFont(None, 30)
        self.current_user_state = 'Play'
        self.state_changed = True

    def change_user_state(self, state):
        """
        Change the current user state,
        keeps track whether the state was changed
        """
        self.state_changed = True
        self.current_user_state = state

    def rgb_to_hex(self):
        """
        Convert RGB slider values to a hexadecimal color string
        """
        if self.sliders:
            color = (
                self.sliders[0].value,
                     self.sliders[1].value,
                self.sliders[2].value
            )
            return '#{:02X}{:02X}{:02X}'.format(*color)

    def render_text(self, text, location = (0,0)):
        """
        Render text on the screen
        """
        text_surface = self.font.render(text, True, (255, 255, 255))
        self.screen.blit(text_surface, location)

    def add_bonus_button(self, x, y, dia):
        """
        Add a bonus button on the screen
        """
        button = Button(x, y, dia, dia, 'B', (255, 0, 0),
                        (0, 0, 0), self.font, 'bonus_button')
        self.buttons.append(button)

    def delete_bonus_button(self):
        """
        Remove all bonus buttons from the screen
        """
        for button in self.buttons:
            if button.property == 'bonus_button':
                self.buttons.remove(button)

    def render_scene(
            self,
            game_state: GameState,
            game_setting: GameSetting,
            chat: list,
            text_buffer
    ):
        """
        Render the entire game scene.

        :param game_state: current game state updated from server
        :param game_setting: game settings received from server
        :param chat: list of chat messages
        :param text_buffer: the current text input for the user
        """
        self.screen.fill((0, 0, 0))

        # remove all existing bonus buttons
        self.delete_bonus_button()

        # if there's bonus button, draw
        if game_state.bonus_box_x is not None:
            self.add_bonus_button(
                game_state.bonus_box_x,
                game_state.bonus_box_y,
                game_state.bonus_box_diameter
            )

        # draw bonus buttons
        for b in self.buttons:
            if b.property == 'bonus_button':
                b.draw(self.screen)

        # draw ball
        pygame.draw.circle(self.screen,
                           (255, 255, 255),
                           (game_state.ball_x, game_state.ball_y),
                           game_setting.settings['BALL_SIZE'],
                           2
                           )
        # draw paddles
        pygame.draw.rect(
            self.screen,
            game_setting.settings['PADDLE1_COLOR'],
            (
                game_setting.settings['PADDLE1_X'],
                game_state.paddle1_y,
                game_setting.settings['PADDLE_WIDTH'],
                game_setting.settings['PADDLE_HEIGHT']
            )
        )
        pygame.draw.rect(
            self.screen,
            game_setting.settings['PADDLE2_COLOR'],
            (
                game_setting.settings['PADDLE2_X'],
                game_state.paddle2_y,
                game_setting.settings['PADDLE_WIDTH'],
                game_setting.settings['PADDLE_HEIGHT']
            )
        )
        # draw Score + names
        score_text = self.font.render(
            f"{game_setting.settings['PADDLE1_NAME']}({game_state.score1}) :"
            f" ({game_state.score2}){game_setting.settings['PADDLE2_NAME']}",
            True,
            (255, 255, 255)
        )
        self.screen.blit(
            score_text,
            (
                self.SCREEN_WIDTH // 2 - score_text.get_width() // 2,
                20
            )
        )

        if self.state_changed:
            self.state_changed = False
            self.buttons = []
            self.sliders = []
            if self.current_user_state == 'Color':
                # create color selection UI
                button = Button(
                    self.SCREEN_WIDTH * 0.55,
                    self.SCREEN_HEIGHT * 0.65,
                    120,
                    40,
                    'ACCEPT',
                    (255, 255, 255),
                    (0, 0, 0),
                    self.font,
                    'color_button'
                )
                red_slider = Slider(
                    250,
                    175,
                    300,
                    20,
                    0, 255,
                    self.font,
                    128,
                    (255, 0, 0),
                    "Red"
                )
                green_slider = Slider(
                    250,
                    255,
                    300,
                    20,
                    0,
                    255,
                    self.font,
                    128,
                    (0, 255, 0),
                    "Green"
                )
                blue_slider = Slider(
                    250,
                    335,
                    300,
                    20,
                    0,
                    255,
                    self.font,
                    128,
                    (0, 0, 255),
                    "Blue"
                )
                self.sliders = [red_slider, green_slider, blue_slider]
                self.buttons.append(button)

        # render chat messages
        offset = self.SCREEN_HEIGHT - 20
        if chat:
            for line in reversed(chat):
                line_surf = self.font.render(line, True, (200, 200, 200))
                self.screen.blit(line_surf, (10, offset))
                offset -= line_surf.get_height() + 2
        # render chat input box
        if self.current_user_state == 'Chat':
            input_rec = pygame.Rect(
                5,
                self.SCREEN_HEIGHT - 40,
                self.SCREEN_WIDTH - 10,
                30
            )
            pygame.draw.rect(
                self.screen,
                (50, 50, 50),
                input_rec
            )
            input_surf = self.font.render(
                text_buffer,
                True,
                (255, 255, 255)
            )
            self.screen.blit(
                input_surf,
                (10, self.SCREEN_HEIGHT - 35)
            )
        # render color selection UI
        if self.current_user_state == 'Color':
            background = pygame.Rect(
                self.SCREEN_WIDTH * 0.25,
                self.SCREEN_HEIGHT * 0.15,
                self.SCREEN_WIDTH * 0.5,
                self.SCREEN_WIDTH * 0.5
            )
            pygame.draw.rect(
                self.screen,
                (125, 125, 125),
                background
            )
            for button in self.buttons:
                if button.property == 'color_button':
                    button.draw(self.screen)
            for slider in self.sliders:
                slider.draw(self.screen)
            preview_rec = pygame.Rect(
                self.SCREEN_WIDTH * 0.3,
                self.SCREEN_HEIGHT * 0.65, 140,
                70
            )
            pygame.draw.rect(
                self.screen,
                (
                    self.sliders[0].value,
                    self.sliders[1].value,
                    self.sliders[2].value
                ),
                preview_rec
            )

    def render_refresh(self):
        pygame.display.flip()
