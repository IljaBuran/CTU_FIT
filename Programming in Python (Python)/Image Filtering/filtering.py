"""import numpy"""
import numpy as np

def filter_image(image: np.array, kernel: np.array) -> np.array:
    """apply a convolution filter to a 2D or 3D image"""

    pad = [
        (kernel.shape[0] // 2, kernel.shape[0] - 1 - kernel.shape[0] // 2),
        (kernel.shape[1] // 2, kernel.shape[1] - 1 - kernel.shape[1] // 2)
    ]
    if image.ndim == 3:
        pad.append((0, 0))

    padded = np.pad(image, pad, mode='constant', constant_values=0)
    output = np.zeros_like(image)
    height, width = image.shape[:2]

    for i in range(height):
        for j in range(width):
            if image.ndim == 2:
                window = padded[i:i + kernel.shape[0], j:j + kernel.shape[1]]
                res = np.sum(kernel * window)
            else:
                window = padded[i:i + kernel.shape[0], j:j + kernel.shape[1], :]
                res = np.einsum('ij,ijc->c', kernel, window)

            output[i, j] = np.clip(res, 0, 255).astype(int)

    return output


def apply_filter(image: np.array, kernel: np.array) -> np.array:
    """ Apply given filter on image """
    # A given image has to have either 2 (grayscale) or 3 (RGB) dimensions
    assert image.ndim in [2, 3]
    # A given filter has to be 2 dimensional and square
    assert kernel.ndim == 2
    assert kernel.shape[0] == kernel.shape[1]
    # TO DO

    return filter_image(image, kernel)
