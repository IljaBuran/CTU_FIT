import improutils as iu
import cv2
import numpy as np

def camera_calib(input_source, chess_shape, output_calib_file=None, img_show_delay=1):
    """
    Browses all images found in input_source and on each image tries to find chessboard corners.
    If chessboard corners are found, image corespondences with real world space are added to lists.
    Based on these image-world corespondences, camera calibration is made.

    Parameters
    ----------
    input_source : string
        Input source for cv2.VideoCapture (could be camera source or sequence of saved images where format has to be specified).
    chess_shape : tuple
        Number of inner corners per a chessboard row and column.
    output_calib_file : Optional[string]
        Output file where calibration is saved when neccesary.
    img_show_delay : int
        Delay in ms between shown images.
    Returns
    -------
    A tuple of camera matrix, distance coefficients and list of images with correctly detected chessboard corners.
    """

    cap = cv2.VideoCapture(input_source)

    if not cap.isOpened():
        cv2.destroyAllWindows()
        raise FileNotFoundError('Capture cannot be opened.')

    # prepare object points, like (0,0,0), (1,0,0), (2,0,0) ....,(6,5,0)
    objp = np.zeros((chess_shape[0] * chess_shape[1], 3), np.float32)
    objp[:, :2] = np.mgrid[0:chess_shape[0], 0:chess_shape[1]].T.reshape(-1, 2)

    # Arrays to store object points and image points from all the images.
    objpoints = []  # 3d point in real world space
    imgpoints = []  # 2d points in image plane.

    cntr = 0
    good_images = []

    while cap.isOpened():
        print(f"Img {cntr} is being processed..")
        cntr += 1
        ret, frame = cap.read()
        if not ret: break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # If found, add object points, image points (after refining them)
        ret, corners = cv2.findChessboardCorners(gray, chess_shape)

        if ret:
            objpoints.append(objp)
            corners = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1),
                                       (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.1))
            imgpoints.append(corners)

            # Draw and display the corners
            frame = cv2.drawChessboardCorners(frame, chess_shape, corners, ret)
            good_images.append(frame)

    print(f'{len(good_images)} from {cntr} frames were correctly detected.')
    iu.plot_images(good_images[0])
    print('Computing camera matrix...')
    rms, camera_matrix, dist_coefs, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None,
                                                                       None)

    if output_calib_file is not None:
        iu.save_camera_calib(output_calib_file, camera_matrix, dist_coefs)

    print('\nRMS:', rms)
    print('Camera matrix:\n', camera_matrix)
    print('Distortion coefficients: ', dist_coefs.ravel())

    return camera_matrix, dist_coefs, good_images