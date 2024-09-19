import subprocess
import datetime

# Fonction pour obtenir le hash du commit Git courant
def get_git_commit_hash():
    try:
        return subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).strip().decode('utf-8')
    except Exception:
        return 'unknown'

def get_build_time():
    return datetime.datetime.now().strftime("%y%m%d")

print(get_git_commit_hash(), get_build_time())