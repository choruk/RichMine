import sys
import time
import json
import urllib
import urllib2
import logging
import subprocess, shlex
from datetime import datetime

# Need to keep a timer going
start_time = datetime.now()

# Set up important variables
url = 'http://ec2-54-209-130-182.compute-1.amazonaws.com:8080/'
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

while True:
    # Send initial request
    empty_file = {'size' : '-1'}
    no_data = urllib.urlencode(empty_file)
    request = urllib2.Request(url, no_data)
    try:
        response = urllib2.urlopen(request)
    except urllib2.HTTPError, e:
        logger.error('HTTPError = ' + str(e.code))
    except urllib2.URLError, e:
        logger.error('URLError = ' + str(e.reason))
    except httplib.HTTPException, e:
        logger.error('HTTPException')
    except Exception:
        import traceback
        logger.error('generic exception: ' + traceback.format_exc())
    current_best_string = response.read()
    current_best_info = json.loads(current_best_string)
    system_success = current_best_info['success']
    if not system_success:
        # If there was a failure, wait 2 minutes and try again
        time.sleep(120)
        continue
    system_size = current_best_info['size']
    system_count = current_best_info['count']
    system_graph = current_best_info['graph']
    break

# Write system info to local file
f = open('system_best.txt', 'w')
file_content = system_size + '\n' + system_count + '\n' + system_graph
f.write(file_content)
f.close()

# Start c code
p = subprocess.Popen(shlex.split('./ce_constructor'))
# 15 hours in minutes
fifteen_hours = 54000
# 16 hours in minutes
sixteen_hours = 57600
# 30 minute intervals
sleep_time = 60*30
# Shorter times used for debugging purposes
debug_start_range = 60*10
debug_end_range = 60*20
debug_sleep_time = 60*5
# Flag used so we dont tell the master to start a new batch job more than once
new_batch_job_request_sent = False
# Main loop
while True:
    # We need to make sure we aren't close to being done
    time_delta = datetime.now() - start_time
    total_seconds = time_delta.seconds + (time_delta.days * 24 * 3600)
    print "\nPYTHON", total_seconds, "seconds have passed so far...\n"
    sys.stdout.flush()
    # Let's just check if we are in the final hour of our run time.
    # If we aren't, then waiting 30 minutes should still give us enough
    # time to catch it on the next loop iteration. (because total_seconds +
    # 30 minutes should still be less than 15.5 hours)
    # if not new_batch_job_request_sent and total_seconds in range(fifteen_hours, sixteen_hours):
    if not new_batch_job_request_sent and total_seconds in range(debug_start_range, debug_end_range):
        # Tell the server we are close to done so it can submit a new
        # batch job.
        data = urllib.urlencode({"meaningless": -1})
        request = urllib2.Request("{0}batch".format(url), data)
        try:
            response = urllib2.urlopen(request)
        except urllib2.HTTPError, e:
            logger.error('HTTPError = ' + str(e.code))
        except urllib2.URLError, e:
            logger.error('URLError = ' + str(e.reason))
        except httplib.HTTPException, e:
            logger.error('HTTPException')
        except Exception:
            import traceback
            logger.error('generic exception: ' + traceback.format_exc())
        response_string = response.read()
        response_dict = json.loads(response_string)
        if response_dict["success"]:
            new_batch_job_request_sent = True
    # Wait 30 minutes before checking again
    # time.sleep(sleep_time)
    time.sleep(debug_sleep_time)
    # Open local file and send to master
    f = open('local_best.txt', 'r')
    local_size = f.readline().strip()
    local_count = f.readline().strip()
    local_graph = f.readline().strip()
    data = {'size' : local_size,
            'count' : local_count,
            'graph' : local_graph}
    data = urllib.urlencode(data)
    request = urllib2.Request(url, data)
    # Get system information from master
    try:
        response = urllib2.urlopen(request)
    except urllib2.HTTPError, e:
        logger.error('HTTPError = ' + str(e.code))
    except urllib2.URLError, e:
        logger.error('URLError = ' + str(e.reason))
    except httplib.HTTPException, e:
        logger.error('HTTPException')
    except Exception:
        import traceback
        logger.error('generic exception: ' + traceback.format_exc())
    current_best_string = response.read()
    current_best_info = json.loads(current_best_string)
    system_success = current_best_info['success']
    if not system_success:
        # Something failed on the master...lets just continue to the next
        # loop iteration and hope things have been fixed.
        continue
    system_size = int(current_best_info['size'])
    # If master graph is better than local, write that to file
    if system_size != -1:
        system_count = current_best_info['count']
        system_graph = current_best_info['graph']
        f = open('system_best.txt', 'w')
        file_content = "{0}\n{1}\n{2}".format(system_size, system_count, system_graph)
        f.write(file_content)
        f.close()
# Done