import pygame

class Button:
    """
    A class to create and manage clickable buttons on screen.
    """
    def __init__(self, x, y, width, height, text, bg_color, text_color, font, property):
        """
        Initialize the button object from parameters provided.
        """
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.bg_color = bg_color
        self.text_color = text_color
        self.text_surf = font.render(self.text, True, self.text_color)
        self.text_rect = self.text_surf.get_rect(center=self.rect.center)
        self.property = property

    def draw(self, surface):
        """
        Draw the button on the screen.
        """
        pygame.draw.rect(surface, self.bg_color, self.rect)
        pygame.draw.rect(surface, (0, 0, 0), self.rect, 2)  # Border
        surface.blit(self.text_surf, self.text_rect)

    def is_clicked(self, event):
        """
        Check if the button clicked.
        """
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if self.rect.collidepoint(event.pos):
                return True
        return False