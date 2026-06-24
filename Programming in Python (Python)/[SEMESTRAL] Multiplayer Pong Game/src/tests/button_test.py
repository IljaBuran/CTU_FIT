import pytest
import pygame

from src.client.button import Button


@pytest.fixture
def pygame_setup():
    pygame.init()
    yield
    pygame.quit()


def test_button_initialization(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    button = Button(100, 150, 200, 50, "Click Me", (255, 0, 0), (255, 255, 255), font, "test_property")

    assert button.rect == pygame.Rect(100, 150, 200, 50)
    assert button.text == "Click Me"
    assert button.bg_color == (255, 0, 0)
    assert button.text_color == (255, 255, 255)
    assert button.property == "test_property"


def test_button_draw(mocker, pygame_setup):
    font = pygame.font.SysFont(None, 30)
    button = Button(100, 150, 200, 50, "Click Me", (255, 0, 0), (255, 255, 255), font, "test_property")
    mock_surface = mocker.Mock()

    button.draw(mock_surface)

    # Verify that draw.rect was called twice and blit was called once
    assert mock_surface.draw.rect.call_count == 2
    mock_surface.blit.assert_called_once()


def test_button_is_clicked(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    button = Button(100, 150, 200, 50, "Click Me", (255, 0, 0), (255, 255, 255), font, "test_property")

    # Simulate a mouse click inside the button
    event_inside = pygame.event.Event(pygame.MOUSEBUTTONDOWN, {'pos': (150, 160), 'button': 1})
    assert button.is_clicked(event_inside) == True

    # Simulate a mouse click outside the button
    event_outside = pygame.event.Event(pygame.MOUSEBUTTONDOWN, {'pos': (50, 60), 'button': 1})
    assert button.is_clicked(event_outside) == False

    # Simulate a different mouse button
    event_wrong_button = pygame.event.Event(pygame.MOUSEBUTTONDOWN, {'pos': (150, 160), 'button': 2})
    assert button.is_clicked(event_wrong_button) == False


def test_button_not_mouse_event(pygame_setup):
    font = pygame.font.SysFont(None, 30)
    button = Button(100, 150, 200, 50, "Click Me", (255, 0, 0), (255, 255, 255), font, "test_property")

    # Simulate a non-mouse event
    event = pygame.event.Event(pygame.KEYDOWN, {'key': pygame.K_RETURN})
    assert button.is_clicked(event) == False
