import re

def get_AC(label):
    ret = re.search('.*app\[0\].*', label)
    if(ret != None):
        #print("Found app[0] (background)")
        return 'background'
    
    ret = re.search('.*app\[1\].*', label)
    if(ret != None):
        #print("Found app[1] (best effort)")
        return 'best effort'
    
    ret = re.search('.*app\[2\].*', label)
    if(ret != None):
        #print("Found app[2] (video)")
        return 'video'
    
    ret = re.search('.*app\[3\].*', label)
    if(ret != None):
        #print("Found app[3] (voice)")
        return 'voice'
    
    return None