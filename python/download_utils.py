__author__ = 'williamspearman'

import boto
import json
import os
import zipfile

# list all entries on the s3 bucket.
def list_entries():
    conn = boto.connect_s3()
    bucket = conn.get_bucket('hudl-hadoop')
    bucket_list = bucket.list('PredatorDataCatalogue/prod/')
    entries = []
    for key in bucket_list:
        json_data = key.get_contents_as_string()
        entries.append(json.loads(json_data))
    return entries

# download a particular entry to the output location
def download_entry(entry, output_location):
    conn = boto.connect_s3()
    bucket_name = entry['job_metadata']['output_bucket_name'];
    bucket = conn.get_bucket(bucket_name)
    things_to_download = ['streaming_manifest', 'frame_sync', 'event_telemetry', 'breakdown'];
    output_names = []
    for thing_key in things_to_download:
        try:
            key = bucket.get_key(entry['predator_output'][thing_key]['Key'])
            fname = entry['predator_output'][thing_key]['OriginalFileName']
            output_fname = os.path.join(output_location, fname)
            if not os.path.exists(output_fname):
                key.get_contents_to_filename(output_fname)
                output_names.append(output_fname)
        except:
            print('Failed for {}'.format(thing_key))

    key = bucket.get_key(entry['job_metadata']['hudl_opta_key'])
    opta_fname = os.path.join(output_location, 'opta.zip')
    key.get_contents_to_filename(opta_fname)
    output_names.append(opta_fname)
    key = bucket.get_key(entry['job_metadata']['hudl_tracab_key'])
    tracab_fname = os.path.join(output_location, 'tracab.zip')
    key.get_contents_to_filename(tracab_fname)
    output_names.append(tracab_fname)
    return output_names

# download all entries to a specific folder
def download_all_entries(download_folder):
    entries = list_entries()

    if not os.path.isdir(download_folder):
        os.mkdir(download_folder)

    for entry in entries:
        eid = entry['job_metadata']['hudl_event_id']
        output_location = os.path.join(download_folder, eid)
        os.mkdir(output_location)
        download_entry(entry, output_location)

# upzips a downloaded folder
def unzip_entries(download_folder):
    for folder_name in os.listdir(download_folder):
        fpath = os.path.join(download_folder, folder_name)
        if os.path.isdir(fpath):
            for fname in os.listdir(fpath):
                if fname[-4:] == '.zip':
                    zip_fpath = os.path.join(fpath, fname)
                    zfile = zipfile.ZipFile(zip_fpath)
                    zfile.extractall(fpath)
                    zfile.close()

# helper function that gets all the relevant files from a downloaded directory
def get_tracab_opta_files(base_directory):
    files_dict = {}

    for fname in os.listdir(base_directory):
        if '.hdf5' in fname:
            files_dict['event_telemetry'] = os.path.join(base_directory, fname)
        elif '-eventdetails.xml' in fname:
            files_dict['opta_xml'] = os.path.join(base_directory, fname)
        elif '-matchresults.xml' in fname:
            files_dict['matchresults_xml'] = os.path.join(base_directory, fname)
        elif fname == 'opta' or fname == 'OptaUpload' or fname == 'Opta':
            cur_path = os.path.join(base_directory, fname)
            if os.path.isdir(cur_path):
                for fname2 in os.listdir(cur_path):
                    if '-eventdetails.xml' in fname2:
                        files_dict['opta_xml'] = os.path.join(cur_path, fname2)
                    elif '-matchresults.xml' in fname2:
                        files_dict['matchresults_xml'] = os.path.join(cur_path, fname2)
        elif 'metadata.xml' in fname:
            files_dict['tracab_xml'] = os.path.join(base_directory, fname)
        elif 'tracab' in fname:
            cur_path = os.path.join(base_directory, fname)
            if os.path.isdir(cur_path):
                for fname2 in os.listdir(cur_path):
                    if 'metadata.xml' in fname2:
                        files_dict['tracab_xml'] = os.path.join(cur_path, fname2)
    return files_dict

def download_single_entry(tracab_id, download_folder):
    entries = list_entries()

    if not os.path.isdir(download_folder):
        os.mkdir(download_folder)

    for entry in entries:
        eid = entry['job_metadata']['hudl_event_id']
        if eid == tracab_id:
            output_location = os.path.join(download_folder, eid)
            os.mkdir(output_location)
            download_entry(entry, output_location)
            break