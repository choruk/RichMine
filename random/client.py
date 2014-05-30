import time
import json
import urllib
import urllib2
from subprocess import call

# Set up important variables
url = 'http://ec2-54-86-64-225.compute-1.amazonaws.com:8080/'

# Send initial request
empty_file = {'size' : '-1'}
no_data = urllib.urlencode(empty_file)
request = urllib2.Request(url, no_data)
try:
    response = urllib2.urlopen(request)
except urllib2.HTTPError, e:
    checksLogger.error('HTTPError = ' + str(e.code))
except urllib2.URLError, e:
    checksLogger.error('URLError = ' + str(e.reason))
except httplib.HTTPException, e:
    checksLogger.error('HTTPException')
except Exception:
    import traceback
    checksLogger.error('generic exception: ' + traceback.format_exc())
current_best_string = response.read()
current_best_info = json.loads(current_best_string)
system_size = current_best_info['size']
system_count = current_best_info['count']
system_graph = current_best_info['graph']
system_success = current_best_info['success']

# Write system info to local file
f = open('system_best.txt', 'w')
file_content = system_size + '\n' + system_count + '\n' + system_graph
f.write(file_content)
f.closed

# Start c code
#subprocess.call(['./runMiner'])

# Main loop
while True:

    # Wait 30 minutes before checking again
    #time.sleep(60*30)

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
        checksLogger.error('HTTPError = ' + str(e.code))
    except urllib2.URLError, e:
        checksLogger.error('URLError = ' + str(e.reason))
    except httplib.HTTPException, e:
        checksLogger.error('HTTPException')
    except Exception:
        import traceback
        checksLogger.error('generic exception: ' + traceback.format_exc())
    current_best_string = response.read()
    current_best_info = json.loads(current_best_string)
    system_size = current_best_info['size']
    system_success = current_best_info['success']

    # If master graph is better than local, write that to file
    if system_size != -1 and system_success :
        system_count = current_best_info['count']
        system_graph = current_best_info['graph']
        f = open('system_best.txt', 'w')
        file_content = system_size + '\n' + system_count + '\n' + system_graph
        f.write(file_content)
        f.closed

    break
