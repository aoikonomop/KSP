import boto3
import re
import os
import sys
import xmltodict
import numpy as np
import zipfile

spark_home = os.environ.get('SPARK_HOME', None)
if not spark_home:
    raise ValueError('SPARK_HOME environment variable is not set')

sys.path.insert(0, os.path.join(spark_home, 'python'))
sys.path.insert(0, os.path.join(spark_home, 'python\\lib\py4j-0.10.1-src.zip'))
execfile(os.path.join(spark_home, 'python\\pyspark\\shell.py'))

PREDATOR_GIT_PATH = "C:\\Users\\antonios.o\\Documents\\GitHub\\hudl_predator\\predator\\"

# Utilities for getting panorama video
def get_panorama(game_id, base_dir='data'):
    """
    Gets the panorama for a GAME_ID (checks local, downloads from S3 if needed)
    """
    s3 = boto3.resource('s3')
    bucket = s3.Bucket('hudl-hadoop')
    for o in bucket.objects.filter(Prefix='dvms-mirror'):
        if re.match('dvms-mirror/[0-9]{{4}}/[0-9]{{8}}-{id}/{id}_tracab_pano.zip'.format(id=game_id), o.key):
            game_dir, game_file = o.key.split('/')[-2:]
            local_path = os.path.join(base_dir, game_dir, game_file)
            if os.path.exists(local_path):
                print("Found local copy at {}".format(local_path))
                return _check_unzip(local_path)

            # if we don't already have it, download it
            local_game_dir = os.path.join(base_dir, game_dir)
            if not os.path.isdir(local_game_dir):
                print("Creating local game_dir")
                os.mkdir(local_game_dir)
            print("Downloading panorama from S3")
            bucket.download_file(o.key, local_path)
            return _check_unzip(local_path)
    return None

def _check_unzip(zip_path):
    """
    Unzips a tracab .zip, or returns the location if it's already unzipped
    """
    unzipped_name = zip_path[:-4]
    if os.path.isdir(unzipped_name):
        print("Found unzipped")
        return unzipped_name
    with zipfile.ZipFile(zip_path) as z:
        #zip_parent_dir = os.path.join(*zip_path.split(os.sep))[:-4]
        zip_parent_dir = os.path.dirname(zip_path)
        print("Unzipping {zip} into {folder}".format(zip=zip_path, folder=zip_parent_dir))
        z.extractall(zip_parent_dir)
    return unzipped_name


def get_telemetry_map(event_telemetry):
    telemetry_list = event_telemetry.home_team.get_player_telemetry(".*") \
    + event_telemetry.away_team.get_player_telemetry(".*")

    telemetry_map = {
        p: t.telemetry_data for p, t in telemetry_list
    }

    return telemetry_map


def get_event_telemetry(game_root, game_id, predator_git_path):

    sys.path.append(predator_git_path)
    from hudl_predator.data_import.processing.event_telemetry import EventTelemetry
    telemetry_file = os.path.join(game_root, 'full_event_{}.hdf5'.format(game_id))
    event_telemetry = EventTelemetry.from_hdf5(telemetry_file)

    return event_telemetry


def get_video_metadata(video_metadata_path):

    with open(video_metadata_path) as f:
        video_metadata = xmltodict.parse(f.read())

    return video_metadata

def get_projection_matrix(video_metadata):
    projection_line = video_metadata['MediaproPanaMetaData']['match']['videofile']['projection']['@matrix']
    projection_matrix = np.array(map(float, projection_line.split())).reshape(3, 4)

    return projection_matrix


def get_positions(adjusted_frame, telemetry_map):

    if adjusted_frame < 0:
        raise ValueError("Invalid frame number {frame}.".format(frame=adjusted_frame))

    positions = []
    for p_id, p_tel in telemetry_map.iteritems():
        if p_tel['in_play'][adjusted_frame]:
            p_x = p_tel['x_pos'][adjusted_frame]
            p_y = p_tel['y_pos'][adjusted_frame]
            positions.append(np.array([p_x, p_y]))
    return np.array(positions)