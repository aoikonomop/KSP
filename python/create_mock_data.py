import glob
from scipy.spatial import distance_matrix
from tracab_telemetry import *

def create_mock_data(data_dir, tracab_id, params, event_telemetry, telemetry_map, video_metadata):

    start_frame = params[0]
    end_frame = params[1]
    xmin = params[2]
    xmax = params[3]
    ymin = params[4]
    ymax = params[5]
    num_x_grid = params[6]
    num_y_grid = params[7]

    tracking_initial_frame = event_telemetry.metadata['initial_frame']
    video_initial_frame = int(video_metadata['MediaproPanaMetaData']['match']['videofile']['start']['@iFrame'])
    global_zero_frame = max(video_initial_frame, tracking_initial_frame)

    xx = np.linspace(xmin, xmax, num_x_grid)
    yy = np.linspace(ymin, ymax, num_y_grid)

    XX, YY = np.meshgrid(xx, yy)
    S = np.array(list(zip(XX.flatten(), YY.flatten())))

    output_dir = os.path.join(data_dir, tracab_id, "mock")
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

    for frame in range(start_frame, end_frame):

        adjusted_frame = frame + global_zero_frame - tracking_initial_frame

        positions = get_positions(adjusted_frame, telemetry_map)
        D = distance_matrix(S, positions)
        NN_indices = D.argmin(axis=0)

        # Store grid values
        Z = np.zeros(len(S))
        Z[NN_indices] = 1

        # Save results
        with open(os.path.join(output_dir, "proba-f{}.dat".format(adjusted_frame)), "w") as f:
            f.write("\n".join(["{} {}".format(a, b) for a, b in zip(range(len(Z)), Z)]))