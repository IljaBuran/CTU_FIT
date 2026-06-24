import pygame

class Slider:
    """
    A class to create and manage a slider UI element.
    """
    def __init__(self, x, y, width, height, min_val, max_val, font, initial, color, label):
        """
        Initialize the Slider instance with position, size, range, and appearance.
        """
        self.rect = pygame.Rect(x, y, width, height)
        self.min = min_val
        self.max = max_val
        self.font = font
        self.value = initial
        self.color = color
        self.label = label

        # Knob properties
        self.knob_radius = height // 2
        self.knob_x = self.get_knob_pos()
        self.knob_y = y + height // 2
        self.dragging = False

    def get_knob_pos(self):
        """
        Calculate the knob's x-position based on the current slider value.
        """
        ratio = (self.value - self.min) / (self.max - self.min)
        return self.rect.x + int(ratio * self.rect.width)

    def handle_event(self, event):
        """
        Handle Pygame events to manage slider interactions.
        """
        if event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:
                # Check if the mouse is on the knob
                mouse_x, mouse_y = event.pos
                distance = ((mouse_x - self.knob_x) ** 2 + (mouse_y - self.knob_y) ** 2) ** 0.5
                if distance <= self.knob_radius:
                    self.dragging = True

        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:
                self.dragging = False

        elif event.type == pygame.MOUSEMOTION:
            if self.dragging:
                mouse_x, _ = event.pos
                # Clamp the knobs x position within the slider's track
                self.knob_x = max(self.rect.x, min(mouse_x, self.rect.x + self.rect.width))
                # Update the value based on knob's position
                ratio = (self.knob_x - self.rect.x) / self.rect.width
                self.value = int(self.min + ratio * (self.max - self.min))

    def draw(self, surface):
        """
        Draw the slider track, knob, and label on the given surface.
        """
        # Draw the slider track
        pygame.draw.rect(surface, (200, 200, 200), self.rect)
        # Draw the knob
        pygame.draw.circle(surface, self.color, (self.knob_x, self.knob_y), self.knob_radius)
        # Draw the label and value
        label_surf = self.font.render(f"{self.label}: {self.value}", True, (0, 0, 0))
        surface.blit(label_surf, (self.rect.x, self.rect.y - 25))
