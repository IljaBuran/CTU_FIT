import os
import pytest
import pygame
from src.client.slider import Slider

os.environ["SDL_VIDEODRIVER"] = "dummy"

@pytest.fixture
def pygame_setup():
    pygame.init()
    yield
    pygame.quit()

def test_slider_initialization(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    slider = Slider(100, 100, 300, 20, 0, 255, font, 128, (255, 0, 0), "Red")

    assert slider.rect == pygame.Rect(100, 100, 300, 20)
    assert slider.min == 0
    assert slider.max == 255
    assert slider.value == 128
    assert slider.color == (255, 0, 0)
    assert slider.label == "Red"
    assert slider.knob_radius == 10
    assert slider.knob_x == 100 + int((128 - 0) / (255 - 0) * 300)
    assert slider.knob_y == 110
    assert slider.dragging == False

def test_slider_handle_event_mouse_down_on_knob(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    slider = Slider(100, 100, 300, 20, 0, 255, font, 128, (255, 0, 0), "Red")
    mouse_pos = (slider.knob_x, slider.knob_y)
    event = pygame.event.Event(pygame.MOUSEBUTTONDOWN, {'pos': mouse_pos, 'button': 1})

    slider.handle_event(event)
    assert slider.dragging == True

def test_slider_handle_event_mouse_move_dragging(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    slider = Slider(100, 100, 300, 20, 0, 255, font, 128, (255, 0, 0), "Red")
    slider.dragging = True

    new_x = 250
    event = pygame.event.Event(pygame.MOUSEMOTION, {'pos': (new_x, slider.knob_y), 'rel': (0, 0)})
    slider.handle_event(event)

    expected_ratio = (new_x - 100) / 300
    expected_value = int(0 + expected_ratio * (255 - 0))
    assert slider.knob_x == new_x
    assert slider.value == expected_value

def test_slider_handle_event_mouse_up(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    slider = Slider(100, 100, 300, 20, 0, 255, font, 128, (255, 0, 0), "Red")
    slider.dragging = True

    event = pygame.event.Event(pygame.MOUSEBUTTONUP, {'pos': (250, slider.knob_y), 'button': 1})
    slider.handle_event(event)
    assert slider.dragging == False

def test_slider_handle_event_non_dragging(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    slider = Slider(100, 100, 300, 20, 0, 255, font, 128, (255, 0, 0), "Red")
    slider.dragging = False

    event = pygame.event.Event(pygame.MOUSEMOTION, {'pos': (250, slider.knob_y), 'rel': (0, 0)})
    slider.handle_event(event)
    assert slider.value == 128

def test_slider_knob_clamping(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    slider = Slider(100, 100, 300, 20, 0, 255, font, 128, (255, 0, 0), "Red")
    slider.dragging = True

    event_left = pygame.event.Event(pygame.MOUSEMOTION, {'pos': (50, slider.knob_y), 'rel': (0, 0)})
    slider.handle_event(event_left)
    assert slider.knob_x == 100
    assert slider.value == 0

    event_right = pygame.event.Event(pygame.MOUSEMOTION, {'pos': (450, slider.knob_y), 'rel': (0, 0)})
    slider.handle_event(event_right)
    assert slider.knob_x == 400
    assert slider.value == 255
