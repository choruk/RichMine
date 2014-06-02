import os, subprocess, shlex

path_to_file = os.path.dirname(os.path.abspath(__file__))
path_to_appengine_sdk = os.path.join(
    path_to_file,
    "google_appengine"
)
appserver_string = "python {0}/dev_appserver.py --host=0.0.0.0 master_app".format(
    path_to_appengine_sdk
)
stdout = os.path.join(
    path_to_file,
    "stdout.log"
)
stdout_fh = open(stdout, 'w')
stderr = os.path.join(
    path_to_file,
    "stderr.log"
)
stderr_fh = open(stderr, 'w')
p = subprocess.Popen(
    shlex.split(appserver_string),
    stdout=stdout_fh,
    stderr=stderr_fh
)
print "Server process running with identifier = {0}".format(p.pid)