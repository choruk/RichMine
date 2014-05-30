"""A simple webapp2 server."""
import os, sys
sys.path.append(os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "boto/"
))
import webapp2, logging
import json, tempfile
from boto.s3.connection import S3Connection

class MainPage(webapp2.RequestHandler):
    '''
    '''
    AWS_CREDS_FILE = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        ".aws-creds"
    )
    AWS_BUCKET_NAME = "group-home-maxwell-hinson"
    AWS_FILE_KEY = "system-best.out"
    def post(self):
        '''
        '''
        logging.info(self.request.POST)
        params = self.request.POST
        graph_size = int(params["size"])
        self.response.headers['Content-Type'] = 'application/json'
        if graph_size == -1:
            logging.info("Processing a request with no input graph data...")
            system_best_data = self.__get_system_best()
            if system_best_data:
                # Just add the success key and return it
                system_best_data["success"] = True
                return self.response.write(json.dumps(system_best_data))
            else:
                logging.info("Failed to get system best data for some reason...returning failure")
                result = {
                    "success": False
                }
                return self.response.write(json.dumps(result))
        else:
            clique_count = int(params["count"])
            graph_string = params["graph"]
            logging.info("Processing a request with input clique_count = {0} and input graph = {1}...".format(
                clique_count,
                graph_string
            ))
    
    def __get_system_best(self):
        credentials = {
            "access_key": '',
            "secret_key": ''
        }
        with open(self.AWS_CREDS_FILE, 'r') as fh:
            for line in fh:
                line_segments = line.strip().split('=')
                if line_segments[0].lower() == "ec2_access_key":
                    credentials["access_key"] = line_segments[1]
                elif line_segments[0].lower() == "ec2_secret_key":
                    credentials["secret_key"] = line_segments[1]
        # Make sure we actually found the creds
        if not credentials["access_key"] or not credentials["secret_key"]:
            logging.info("Unable to find AWS credentials")
            return False
        # Now get the latest from S3
        result = self.__get_from_s3(credentials)
        if result:
            response_data = {}
            # If it exists, result will be the absolute path to the newly downloaded file
            with open(result, "r") as fh:
                for index, line in enumerate(fh.readlines()):
                    if index == 0:
                        # size
                        response_data["size"] = line.strip()
                    elif index == 1:
                        # count
                        response_data["count"] = line.strip()
                    elif index == 2:
                        # graph
                        response_data["graph"] = line.strip()
                    else:
                        logging.info("Found unrecognized line in system best graph: {0}".format(
                            line
                        ))
            # Now just delete the file and return this dict
            os.remove(result)
            return response_data
        else:
            logging.info("Get from S3 failed, unable to get the system best graph")
            return False
    
    def __get_from_s3(self, credentials):
        bucket_name = self.AWS_BUCKET_NAME
        key_name = self.AWS_FILE_KEY
        conn = S3Connection(
            credentials["access_key"],
            credentials["secret_key"]
        )
        # Does it exist?
        if conn.lookup(bucket_name):
            bucket = conn.get_bucket(bucket_name)
            key = bucket.get_key(key_name)
            if key:
                fileint, file_name = tempfile.mkstemp(suffix=".out")
                os.close(fileint)
                key.get_contents_to_filename(file_name)
                return file_name
            else:
                logging.info("Fatal error: Key with name {0} doesn't exist".format(key_name))
                return False
        else:
            logging.info("Fatal error: Bucket with name {0} doesn't exist".format(bucket_name))
            return False


logging.getLogger().setLevel(logging.DEBUG)

application = webapp2.WSGIApplication([
    ('/', MainPage),
], debug=True)