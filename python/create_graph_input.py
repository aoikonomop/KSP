import os
import numpy as np

#(1) Generate the spatial topology part of the configuration file.
#   - Although we have a 2D problem, we can convert that to 1D by assigning unique ids for each location and defining the
#     allowable connections between the grid locations.
#   - In our case, an object on any location can move to any adjacent location or stay at the same location, so a maximum
#     of 9 connections per grid location.

def generate_spatial_topology(numX, numY):
    numPositions = numX * numY
    outputMatrix = np.zeros((numPositions, numPositions), dtype=np.int)
    for idx in range(0, numPositions):
        outputMatrix[idx, idx] = 1
        if (idx + 1 < numPositions) and (idx % numX != (numX - 1)):
            outputMatrix[idx, idx + 1] = 1
        if (idx + numX + 1 < numPositions) and (idx % numX != (numX - 1)):
            outputMatrix[idx, idx + numX + 1] = 1
        if (idx + numX < numPositions):
            outputMatrix[idx, idx + numX] = 1
        if (idx + numX - 1 < numPositions) and (idx % numX !=0):
            outputMatrix[idx, idx + numX - 1] = 1
        if (idx - 1 >= 0) and (idx % numX !=0):
            outputMatrix[idx, idx - 1] = 1
        if (idx - numX - 1 >= 0) and (idx % numX !=0):
            outputMatrix[idx, idx - numX - 1] = 1
        if (idx - numX >= 0):
            outputMatrix[idx, idx - numX] = 1
        if (idx - numX + 1 >= 0) and (idx % numX != (numX - 1)):
            outputMatrix[idx, idx - numX + 1] = 1

    return outputMatrix

#(2) Generate entry points to the graph.
#   - Subjects are allowed to enter at the first time-frame at any position

def generate_entry_points(numPositions, numT):
    outputMatrix = np.zeros((numT, numPositions), dtype=np.int)
    for idx in range(0, numPositions):
        outputMatrix[0, idx] = int(1)

    return outputMatrix

#(3) Generate exit points to the graph.
#   - Subjects are allowed to exit at the last time-frame at any position

def generate_exit_points(numPositions, numT):
    outputMatrix = np.zeros((numT, numPositions), dtype=np.int)
    for idx in range(0, numPositions):
        outputMatrix[numT - 1, idx] = int(1)

    return outputMatrix

#(4) Load mock POMs produced by the available tracab data

def read_mock_data(mock_data_path, numPositions, numT):
    outputMatrix = np.zeros((numT, numPositions))
    t = 0;
    for file in os.listdir(mock_data_path):
        with open(os.path.join(mock_data_path, file), "r") as openfileobject:
            for line in openfileobject:
                [grid_position, confidence] = line.split(' ')
                if (float(confidence) - 1.0 < 0):
                    outputMatrix[t, int(grid_position)] = -1
                else:
                    outputMatrix[t, int(grid_position)] = 10 #dummy values that in reality should correspond to detection probabilities
        t = t + 1

    return outputMatrix

def writeMatrix(data, matrix):
    for i in range(0, len(matrix)):
        for j in range(0, len(matrix[i])):
            data.write("{0} ".format(matrix[i, j]))
        data.write("\n")

    data.write("\n")
    return data

def create_graph_input(data_dir, tracab_id, params):

    output_path = os.path.join(data_dir, tracab_id, "graph")
    if not os.path.exists(output_path):
        os.mkdir(output_path)

    graph_output_path = os.path.join(output_path, "config.dat")
    mock_data_path = os.path.join(data_dir, tracab_id, "mock")

    data = open(graph_output_path, "w")

    numT = params[1] - params[0] + 1
    numX = params[6]
    numY = params[7]

    #Write numPosition, numT
    data.write("{0} {1}\n".format(numX * numY, numT))
    data.write("\n")

    #Write topology matrix
    topologyMatrix = generate_spatial_topology(numX, numY)
    data = writeMatrix(data, topologyMatrix)

    #Write entry matrix
    entryMatrix = generate_entry_points(numX * numY, numT)
    data = writeMatrix(data, entryMatrix)

    #Write exit matrix
    exitMatrix = generate_exit_points(numX * numY, numT)
    data = writeMatrix(data, exitMatrix)

    #Write confidence matrix
    confidenceMatrix = read_mock_data(mock_data_path, numX * numY, numT)
    data = writeMatrix(data, confidenceMatrix)

    data.close()