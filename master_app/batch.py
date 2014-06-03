import webapp2, logging
import subprocess, shlex
import json
#NOTE: This needs to be downloaded/installed if it isn't already
# sudo pip install pexpect
# ^^^^^^^^^^^^^^^^^^^^^^^^ will do the trick
import pexpect
from google.appengine.api import background_thread

class BatchJobHandler(webapp2.RequestHandler):
    '''
    '''
    LOG_FILE_NAME = "ssh-batch.log"
    def post(self):
        '''
        '''
        tid = background_thread.start_new_background_thread(self.__submit_new_batch_job, [])
        self.response.headers['Content-Type'] = 'application/json'
        return self.response.write(json.dumps({
            "success": True
        }))
    
    def __submit_new_batch_job(self):
        ssh_and_submit_batch_string = "ssh -o 'StrictHostKeyChecking no' mhinson@stampede.tacc.xsede.org "
        ssh_and_submit_batch_string += "'cd RichMine/ce_constructor ; sbatch launchStampede.sh'"
        child = pexpect.spawn(ssh_and_submit_batch_string)
        child.expect('password:')
        child.sendline('RichDigger2')
        fh = open(self.LOG_FILE_NAME, 'w')
        child.logfile = fh
        child.expect(pexpect.EOF)
        child.close()
        fh.close()
        with open(self.LOG_FILE_NAME, 'r') as fh:
            logging.info("Just finished SSH command with return code={0} output={1}".format(
                child.exitstatus,
                fh.read()
            ))
