from create_mock_data import *
from create_graph_input import create_graph_input
from create_trajectory_files import write_trajectory_files
from create_video import write_video
from download_utils import download_single_entry, unzip_entries


def main():
    #Set Parameters
    data_dir = "C:\\Users\\antonios.o\\Documents\\Test_projects\\MOT-MTP\Data\\"
    predator_git_path = "C:\\Users\\antonios.o\\Documents\\GitHub\\hudl_predator\\predator\\"
    tracab_id = "56f0146f1c374d31a4109265"

    xmin, xmax = -55, 55  # x range in Tracab coordinate system
    ymin, ymax = -44, 44  # y range in Tracab coordinate system
    num_x_grid = 110  # Grid size
    num_y_grid = 88  # Grid size
    false_neg_rate = 0.1  # P(s_hat == 0 | s == 1)
    false_pos_rate = 0.1  # P(s_hat == 1 | s == 0)
    start_frame = 20000  # The Tracab ID of the frame to start on
    num_frames = 100
    end_frame = start_frame + num_frames  # The Tracab ID of the frame to end on

    #Populate params array
    params = []
    params.append(start_frame) #0
    params.append(end_frame)   #1
    params.append(xmin)        #2
    params.append(xmax)        #3
    params.append(ymin)        #4
    params.append(ymax)        #5
    params.append(num_x_grid)  #6
    params.append(num_y_grid)  #7
    params.append(num_frames)  #8

    #Download entries from S3

    print("Downloading data from S3...")

    if not os.path.exists(os.path.join(data_dir, tracab_id)):
        download_single_entry(tracab_id, data_dir)
        unzip_entries(data_dir)

    print("Done!")

    #Get telemetry and metadata

    print("Getting telemetry and metadata...")

    game_root = os.path.join(data_dir, tracab_id)
    game_id = glob.glob(os.path.join(game_root, 'full_event_*.hdf5'))[0][-11:-5]

    video_file_name = '{}-pana.mp4'.format(game_id)

    event_telemetry = get_event_telemetry(game_root, game_id, predator_git_path)
    telemetry_map = get_telemetry_map(event_telemetry)

    #Load metadata
    panorama_dir = get_panorama(game_id, os.path.join(data_dir, tracab_id))
    if panorama_dir is None:
        raise ValueError("Don't have tracab panorama for this game (at least not in the format I expect :/)")

    video_path = os.path.join(panorama_dir, video_file_name)

    video_metadata_file_name = '{}-videometadata.xml'.format(game_id)
    video_metadata_path = os.path.join(panorama_dir, video_metadata_file_name)
    video_metadata = get_video_metadata(video_metadata_path)

    print("Done!")

    #Create mock data from tracab

    print("Creating mock data...")

    create_mock_data(data_dir, tracab_id, params, event_telemetry, telemetry_map, video_metadata)

    print("Done!")

    #Create input for C++ code

    print("Preparing KSP input...")

    create_graph_input(data_dir, tracab_id, params)

    print("Done!")

    #Run C++ code

    print("Running KSP...")

    KSP_executable_path = "C:\\Users\\antonios.o\\Documents\\Test_projects\\MOT-MTP\\mtp\\MTP\\x64\\Release\\MTP.exe"
    config_path = os.path.join(data_dir, tracab_id, "graph", "config.dat")
    trajectory_path = os.path.join(data_dir, tracab_id, "graph", "trajectories.dat")
    graph_path = os.path.join(data_dir, tracab_id, "graph", "graph.dat")

    args = KSP_executable_path + " " + config_path + " " + trajectory_path + " " + graph_path + " 1"
    os.system(args)

    print("Done!")

    #Project back and save trajectory files

    print("Writing trajectory files...")

    write_trajectory_files(data_dir, tracab_id, params)

    print("Done!")

    #Write output video

    print("Writing output video...")

    write_video(video_path, data_dir, tracab_id, params, event_telemetry, video_metadata)

    print("Done!")

if __name__ == "__main__":
    main()