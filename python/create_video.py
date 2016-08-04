import os
import imageio
import numpy as np
import cv2

def get_positions_file(filename):
    positions = []
    for line in open(filename, "r"):
        data = line.split(" ")
        positions.append(np.array([int(data[0]), int(data[1])]))

    return np.array(positions)

def get_video(video, frame, event_telemetry, video_metadata):

    tracking_initial_frame = event_telemetry.metadata['initial_frame']
    video_initial_frame = int(video_metadata['MediaproPanaMetaData']['match']['videofile']['start']['@iFrame'])
    global_zero_frame = max(video_initial_frame, tracking_initial_frame)

    adjusted_frame = frame + global_zero_frame - video_initial_frame

    if adjusted_frame < 0:
        raise ValueError("Invalid frame number")
    return video.get_data(adjusted_frame)

def project(points, projection_matrix, z=0):
    """
    :param points: (n, 2) np.array of (x, y) coordinates from tracab
    :param projection_matrix: projection matrix from tracab
    """
    padded = np.hstack([points, z * np.ones((points.shape[0], 1)), np.ones((points.shape[0], 1))]).transpose()
    prod = np.matmul(projection_matrix, padded)
    scaled = prod / prod[-1, :]
    return scaled[:2].transpose()

def write_video(input_video_path, data_dir, tracab_id, params, event_telemetry, video_metadata):
    start_frame = params[0]
    end_frame = params[1]
    fps = 25

    output_dir = os.path.join(data_dir, tracab_id, "vid_files")
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

    out = cv2.VideoWriter(os.path.join(output_dir, "tracklets.avi"), \
                   cv2.VideoWriter_fourcc('m', 'p', '4', '2'), fps, (1920, 480), 1)

    traj_files = os.path.join(data_dir, tracab_id, "trajectory_files")
    num_frames = end_frame - start_frame + 1

    video = imageio.read(input_video_path)

    projection_line = video_metadata['MediaproPanaMetaData']['match']['videofile']['projection']['@matrix']
    projection_matrix = np.array(map(float, projection_line.split())).reshape(3, 4)

    for frame in range(0, num_frames):
        img = get_video(video, start_frame + frame, event_telemetry, video_metadata)
        colors = [(255, 0, 0), \
                  (255, 255, 0), \
                  (255, 255, 255), \
                  (0, 255, 0), \
                  (0, 255, 255),
                  (0, 0, 255)]
        count = 0
        for file in os.listdir(traj_files):
            positions = get_positions_file(os.path.join(traj_files, file))

            for i in range(0, frame + 1):
                projections = project(np.array([positions[i]]), projection_matrix, z=1.82 / 2)
                cv2.rectangle(img, (int(projections[0][0]), int(projections[0][1])), (int(projections[0][0] + 1), int(projections[0][1] + 1)), colors[count], 2)

            count = count + 1
            if count >= len(colors):
                count = 0

        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        out.write(img)

    out.release()