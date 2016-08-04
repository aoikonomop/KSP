import os

def write_trajectory_files(data_dir, tracab_id, params):

    xmin = params[2]
    xmax = params[3]
    ymin = params[4]
    ymax = params[5]
    num_x_grid = params[6]
    num_y_grid = params[7]

    trajectory_file = os.path.join(data_dir, tracab_id, "graph", "trajectories.dat")
    output_path = os.path.join(data_dir, tracab_id, "trajectory_files")
    if not os.path.exists(output_path):
        os.mkdir(output_path)

    count = 1
    for line in open(trajectory_file):

        x_bucket_width = (xmax - xmin) // num_x_grid
        y_bucket_width = (ymax - ymin) // num_y_grid

        points = []

        data = line.split(" ")
        if len(data) == 1:
            continue

        traj_data = open(os.path.join(output_path, str(count) + ".txt"), "w")

        for i in range(4, len(data)):
            grid_id = int(data[i])

            y_grid_pos = grid_id // num_x_grid
            x_grid_pos = grid_id - y_grid_pos * num_x_grid

            xpos = x_grid_pos * x_bucket_width - xmax
            ypos = y_grid_pos * y_bucket_width - ymax

            traj_data.write("{0} {1} \n".format(xpos, ypos))

        traj_data.close()
        count = count + 1