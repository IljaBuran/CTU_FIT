import pygame
import sys
import threading

from src.client.render import Render
from src.client.network import Network


def client():
    """
    Initializes and runs the main game loop, handles user input,
    calls corresponding network and render functions
    """
    network = Network()

    # attempt to connect to the server
    while True:
        server_ip = input("Server IP (default=127.0.0.1): ") or "127.0.0.1"
        server_port = 12345
        try:
            network.sock.connect((server_ip, server_port))
            print("[CONNECTED] to server.")
            break

        except ConnectionRefusedError:
            print("[ERROR] Could not connect. Is server running?")

    # prompt user for his nickname
    while True:
        name = input("Enter your name: ")
        if name == "":
            print("[ERROR] Name cannot be empty. Try again.")
        elif len(name) > 10:
            print("[ERROR] Name cannot be longer than 10 characters. Try again.")
        else:
            break

    # start a thread to receive game_state updates from server
    threading.Thread(
        target=network.receive_game_state,
        daemon=True
    ).start()

    # send player's name to the server
    network.send_command(f'/NAME:{name}')

    # initializes window
    window = Render()
    window.pygame_init()
    text_buffer = ""

    running = True
    while running:
        window.clock.tick(60) # limits 60 FPS
        keys = pygame.key.get_pressed()

        # handle pygame.events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        # handle player's input
            # if key gets pressed
            if event.type == pygame.KEYDOWN:
                if window.current_user_state == 'Chat':
                    if event.key == pygame.K_RETURN:
                        if text_buffer:
                            if text_buffer.startswith('/'):
                                # handle command input
                                new_state = network.handle_command(
                                    text_buffer,
                                    window.current_user_state
                                )
                                window.change_user_state(new_state)
                            else:
                                # send chat message
                                network.send_command(f"CHAT:{text_buffer}")
                                window.change_user_state('Play')
                            text_buffer = ""
                        else:
                            window.change_user_state('Play')
                    # if backspace is pressed in 'Chat' mode, remove last character from buffer
                    elif event.key == pygame.K_BACKSPACE:
                        text_buffer = text_buffer[:-1]
                    else:
                        # add character to buffer
                        if event.unicode.isprintable():
                            text_buffer += event.unicode
                else:
                    # if 't' button is pressed in 'Play' mode, switches into 'Chat' mode
                    if event.key == pygame.K_t and window.current_user_state == 'Play':
                        window.change_user_state('Chat')

            # handle button clicks
            if window.buttons:
                for button in window.buttons:
                    if button.is_clicked(event):
                        # send color change command
                        if button.property == 'color_button':
                            network.send_command(f"/COLOR:{window.rgb_to_hex()}")
                            window.change_user_state('Play')
                        # send bonus command
                        if button.property == 'bonus_button' and window.current_user_state == 'Play':
                            network.send_command(f"/BONUS:DONE")
            # handle slider events
            if window.sliders:
                for slider in window.sliders:
                    slider.handle_event(event)

        # handle player's movement
        if window.current_user_state == 'Play':
            if keys[pygame.K_UP]:
                network.send_command(f"{network.role}:UP")
            elif keys[pygame.K_DOWN]:
                network.send_command(f"{network.role}:DOWN")
        # draw on the screen
        window.render_scene(
            network.game_state,
            network.game_setting,
            network.chat,
            text_buffer
        )
        window.render_refresh()

    # clean up
    pygame.quit()
    sys.exit()