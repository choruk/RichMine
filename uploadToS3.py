from boto.s3.connection import S3Connection
from boto.s3.key import Key
import boto

AWS_ACCESS_KEY_ID = 'AKIAIO7IUJCD2P5INQYA'
AWS_SECRET_ACCESS_KEY = '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'
bucket_name = 'richcoin'

# Connect
c = boto.connect_s3(AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY)

# Get the bucket
b = c.get_bucket(bucket_name)

# Upload the file
k = Key(b)
k.key = 'solution_99'
k.set_contents_from_filename("99.txt");
