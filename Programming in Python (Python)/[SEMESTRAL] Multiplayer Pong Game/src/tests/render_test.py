import pygame
import sys
import time

class GameState:
    def __init__(self, bonus_box_x=None, bonus_box_y=None, bonus_box_diameter=None,
                 ball_x=400, ball_y=300, paddle1_y=250, paddle2_y=250,
                 score1=0, score2=0):
        self.bonus_box_x = bonus_box_x
        self.bonus_box_y = bonus_box_y
        self.bonus_box_diameter = bonus_box_diameter
        self.ball_x = ball_x
        self.ball_y = ball_y
        self.paddle1_y = paddle1_y
        self.paddle2_y = paddle2_y
        self.score1 = score1
        self.score2 = score2

class GameSetting:
    def __init__(self, ball_size=10, paddle1_color=(255, 0, 0), paddle2_color=(0, 0, 255),
                 paddle1_x=50, paddle2_x=750, paddle_width=10, paddle_height=100,
                 paddle1_name='Player1', paddle2_name='Player2'):
        self.settings = {
            'BALL_SIZE': ball_size,
            'PADDLE1_COLOR': paddle1_color,
            'PADDLE2_COLOR': paddle2_color,
            'PADDLE1_X': paddle1_x,
            'PADDLE2_X': paddle2_x,
            'PADDLE_WIDTH': paddle_width,
            'PADDLE_HEIGHT': paddle_height,
            'PADDLE1_NAME': paddle1_name,
            'PADDLE2_NAME': paddle2_name
        }

class Button:
    def __init__(self, x, y, width, height, text, bg_color, text_color, font, property):
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.bg_color = bg_color
        self.text_color = text_color
        self.font = font
        self.property = property
        self.text_surf = self.font.render(self.text, True, self.text_color)
        self.text_rect = self.text_surf.get_rect(center=self.rect.center)

    def draw(self, screen):
        pygame.draw.rect(screen, self.bg_color, self.rect)
        screen.blit(self.text_surf, self.text_rect)

class Slider:
    def __init__(self, x, y, width, height, min_val, max_val, font, initial, color, label):
        self.rect = pygame.Rect(x, y, width, height)
        self.min_val = min_val
        self.max_val = max_val
        self.value = initial
        self.color = color
        self.font = font
        self.label = label

    def draw(self, screen):
        pygame.draw.rect(screen, (200, 200, 200), self.rect)
        handle_x = self.rect.x + int((self.value - self.min_val) / (self.max_val - self.min_val) * self.rect.width)
        handle_rect = pygame.Rect(handle_x - 5, self.rect.y, 10, self.rect.height)
        pygame.draw.rect(screen, self.color, handle_rect)
        label_surf = self.font.render(self.label, True, (255, 255, 255))
        screen.blit(label_surf, (self.rect.x, self.rect.y - 25))

from src.client.render import Render

def initialize_render():
    render = Render()
    render.pygame_init()
    return render

def create_test_scenarios(font):
    scenarios = []

    game_state1 = GameState()
    game_setting1 = GameSetting()
    scenarios.append({
        'description': 'Default Play State',
        'game_state': game_state1,
        'game_setting': game_setting1,
        'chat': [],
        'text_buffer': '',
        'user_state': 'Play'
    })

    game_state2 = GameState(bonus_box_x=300, bonus_box_y=300, bonus_box_diameter=50)
    game_setting2 = GameSetting()
    scenarios.append({
        'description': 'Play State with Bonus Button',
        'game_state': game_state2,
        'game_setting': game_setting2,
        'chat': [],
        'text_buffer': '',
        'user_state': 'Play'
    })

    game_state3 = GameState()
    game_setting3 = GameSetting()
    scenarios.append({
        'description': 'Chat State',
        'game_state': game_state3,
        'game_setting': game_setting3,
        'chat': ["Player1: Hello!", "Player2: Hi there!", "Player1: How are you?"],
        'text_buffer': 'Type your message...',
        'user_state': 'Chat'
    })

    game_state4 = GameState()
    game_setting4 = GameSetting()
    scenarios.append({
        'description': 'Color Selection State',
        'game_state': game_state4,
        'game_setting': game_setting4,
        'chat': [],
        'text_buffer': '',
        'user_state': 'Color'
    })

    return scenarios

def run_test(render, scenario, font):
    print(f"Running Test: {scenario['description']}")
    render.change_user_state(scenario['user_state'])

    render.current_user_state = scenario['user_state']
    render.state_changed = True

    render.render_scene(
        game_state=scenario['game_state'],
        game_setting=scenario['game_setting'],
        chat=scenario['chat'],
        text_buffer=scenario['text_buffer']
    )

    render.render_refresh()
    display_duration = 3  # seconds
    start_time = time.time()
    running = True
    while running and (time.time() - start_time) < display_duration:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
        render.clock.tick(60)

def main():
    pygame.init()
    font = pygame.font.SysFont(None, 30)

    render = initialize_render()

    scenarios = create_test_scenarios(font)

    for scenario in scenarios:
        run_test(render, scenario, font)

    print("All tests completed. Closing the window in 2 seconds...")
    time.sleep(2)
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
